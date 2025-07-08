#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <turbojpeg.h>
#include <libwebsockets.h>
#include <mutex>
#include <thread>
#include <chrono>
#include <vector>
#include <atomic>
#include <cstring>
#include <iostream>
#include <unordered_set>
#include <queue>

// X11 and compression
Window xwin;
Display* xdisplay = nullptr;
tjhandle tj = nullptr;
const int target_fps = 5;
const int frame_delay_ms = 1000 / target_fps;

// WebSocket
static struct lws_context* context = nullptr;
static std::mutex connections_mutex;
static std::unordered_set<struct lws*> connections;
static std::atomic<bool> running{true};

// Frame buffer
struct Frame {
    std::vector<unsigned char> data;
    std::chrono::steady_clock::time_point capture_time;
};
std::mutex frame_mutex;
Frame current_frame;

// WebSocket protocol callbacks
static int websocket_callback(struct lws* wsi, enum lws_callback_reasons reason,
                            void* user, void* in, size_t len) {
    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED: {
            std::lock_guard<std::mutex> lock(connections_mutex);
            connections.insert(wsi);
            std::cout << "New client connected! Total: " << connections.size() << std::endl;
            // Immediately request write
            lws_callback_on_writable(wsi);
            break;
        }
            
        case LWS_CALLBACK_CLOSED: {
            std::lock_guard<std::mutex> lock(connections_mutex);
            connections.erase(wsi);
            std::cout << "Client disconnected. Remaining: " << connections.size() << std::endl;
            break;
        }
            
        case LWS_CALLBACK_SERVER_WRITEABLE: {
            std::lock_guard<std::mutex> lock(frame_mutex);
            if (!current_frame.data.empty()) {
                unsigned char buf[LWS_PRE + current_frame.data.size()];
                memcpy(buf + LWS_PRE, current_frame.data.data(), current_frame.data.size());
                int ret = lws_write(wsi, buf + LWS_PRE, current_frame.data.size(), LWS_WRITE_BINARY);
                
                if (ret < 0) {
                    std::cerr << "Write failed!" << std::endl;
                } 
				// else {
                //     auto now = std::chrono::steady_clock::now();
                //     auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(
                //         now - current_frame.capture_time).count();
                //     std::cout << "Frame sent (" << current_frame.data.size() 
                //               << " bytes, " << latency << "ms latency)" << std::endl;
                // }
            }
            // Request next write immediately
            lws_callback_on_writable(wsi);
            break;
        }
        
        default:
            break;
    }
    return 0;
}

static struct lws_protocols protocols[] = {
    {
        "fltk-stream",
        websocket_callback,
        0,
        0,
    },
    { NULL, NULL, 0, 0 }
};

void capture_loop() {
    XWindowAttributes attrs;
    XGetWindowAttributes(xdisplay, xwin, &attrs);
    
    while (running) {
        auto capture_start = std::chrono::steady_clock::now();
        
        // Capture
        XImage* img = XGetImage(xdisplay, xwin, 0, 0, 
                              attrs.width, attrs.height, 
                              AllPlanes, ZPixmap);
        if (!img) {
            std::cerr << "XGetImage failed!" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        // Compress
        unsigned char* jpegBuf = nullptr;
        unsigned long jpegSize = 0;
        if (tjCompress2(tj, reinterpret_cast<unsigned char*>(img->data),
                       attrs.width, img->bytes_per_line, attrs.height,
                       TJPF_BGRX, &jpegBuf, &jpegSize, 
                       TJSAMP_420, 75, TJFLAG_FASTDCT) != 0) {
            std::cerr << "JPEG compression failed: " << tjGetErrorStr() << std::endl;
            XDestroyImage(img);
            continue;
        }

        // Store frame
        {
            std::lock_guard<std::mutex> lock(frame_mutex);
            current_frame.data.assign(jpegBuf, jpegBuf + jpegSize);
            current_frame.capture_time = capture_start;
        }
        
        // Cleanup
        tjFree(jpegBuf);
        XDestroyImage(img);
        
        // Maintain FPS
        auto elapsed = std::chrono::steady_clock::now() - capture_start;
        int remaining = frame_delay_ms - 
            std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
        if (remaining > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(remaining));
        }
    }
}

int startstream(Window _win) {
    xwin = _win;
    
    // Initialize X11
    xdisplay = XOpenDisplay(nullptr);
    if (!xdisplay) {
        std::cerr << "Failed to open X display!" << std::endl;
        return 1;
    }

    // Initialize TurboJPEG
    tj = tjInitCompress();
    if (!tj) {
        std::cerr << "Failed to initialize TurboJPEG!" << std::endl;
        XCloseDisplay(xdisplay);
        return 1;
    }

    // WebSocket setup
    struct lws_context_creation_info info;
    memset(&info, 0, sizeof(info));
    info.port = 9002;
    info.protocols = protocols;
    info.gid = -1;
    info.uid = -1;
    
    context = lws_create_context(&info);
    if (!context) {
        std::cerr << "Failed to create WebSocket context!" << std::endl;
        tjDestroy(tj);
        XCloseDisplay(xdisplay);
        return 1;
    }

    std::cout << "Server started on port 9002" << std::endl;
    
    // Start capture thread
    std::thread cap_thread(capture_loop);
    
    // Main loop
    while (running) {
        lws_service(context, 0);  // No timeout for maximum responsiveness
        
        // Manually trigger writable callbacks periodically
        static auto last_notify = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_notify).count() > 50) {
            std::lock_guard<std::mutex> lock(connections_mutex);
            for (auto wsi : connections) {
                lws_callback_on_writable(wsi);
            }
            last_notify = now;
        }
    }
    
    // Cleanup
    cap_thread.join();
    lws_context_destroy(context);
    tjDestroy(tj);
    XCloseDisplay(xdisplay);
    
    return 0;
}