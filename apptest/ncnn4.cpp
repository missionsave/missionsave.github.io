// g++ ncnn4.cpp -o ncnn4 -I/home/super/msv/ncnn/build/install/include -L/home/super/msv/ncnn/build/install/lib -lncnn -lglslang -lMachineIndependent -lGenericCodeGen  -lOSDependent -lSPIRV -lpthread -fopenmp -lfltk -lfltk_images -lX11 -lXext -lXfixes -lXcursor -lXrender -lXinerama -lXft -lfontconfig -lfltk_gl -lGL -lGLU

#include <ncnn/net.h>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/fl_draw.H>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <time.h> 
#include <thread>

using namespace std;
// struct performance{ 
//     std::chrono::steady_clock::time_point pclock;
//     const char* pname; 
//     performance(const char*  name=""); //
//     void p(const char* prefix=""); //
// };

// performance::performance(const char*  name){
//     pname=name;
//     pclock=std::chrono::steady_clock::now();
// }
// void performance::p(const char* prefix){
// 	if(prefix==0){
// 		pclock=std::chrono::steady_clock::now();
// 		printf("benchmark\n");
// 		return;
// 	}
// 	auto end = std::chrono::steady_clock::now();
// 	auto elapsed = end - pclock;
// 	printf("%s %.9f sec\n",prefix,std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count()/1000000000.0);
// 	// printf("%s %s %f sec\n",pname,prefix,((double)clock()-pclock)/CLOCKS_PER_SEC);
// 	fflush(stdout);
// 	pclock=std::chrono::steady_clock::now();
// }
// performance perfinit;
// void perf(string p){
//     if(p==""){perfinit.pclock=std::chrono::steady_clock::now();return;}
//     perfinit.p(p.c_str());
// } 
// performance perfinit1;
// void perf1(string p){
//     if(p==""){perfinit1.pclock=std::chrono::steady_clock::now();return;}
//     perfinit1.p(p.c_str());
// } 
// performance perfinit2;
// void perf2(string p){
//     if(p==""){perfinit2.pclock=std::chrono::steady_clock::now();return;}
//     perfinit2.p(p.c_str());
// } 

struct Box { int x, y, w, h; int id; float score; };

//region nms
#if defined(__ARM_NEON)
#include <arm_neon.h>

// NEON-accelerated IoU calculation
inline float calculate_iou_neon(const Box& a, const Box& b) {
    // Load coordinates into NEON vectors
    int32x4_t a_coords = {a.x, a.y, a.x + a.w, a.y + a.h};
    int32x4_t b_coords = {b.x, b.y, b.x + b.w, b.y + b.h};
    
    // Calculate intersection coordinates
    int32x4_t max_coords = vmaxq_s32(a_coords, b_coords);
    int32x4_t min_coords = vminq_s32(a_coords, b_coords);
    
    // Intersection width and height (max_coords are x1,y1, min_coords are x2,y2)
    int32x2_t inter_wh = vsub_s32(vget_high_s32(min_coords), vget_low_s32(max_coords));
    
    // Get individual values
    int32_t inter_w = vget_lane_s32(inter_wh, 0);
    int32_t inter_h = vget_lane_s32(inter_wh, 1);
    
    if (inter_w <= 0 || inter_h <= 0) return 0.0f;
    
    int inter_area = inter_w * inter_h;
    int area_a = a.w * a.h;
    int area_b = b.w * b.h;
    int union_area = area_a + area_b - inter_area;
    
    return static_cast<float>(inter_area) / static_cast<float>(union_area);
}

// Use the NEON version
#define calculate_iou calculate_iou_neon

#else
// Fallback scalar implementation for non-NEON platforms
inline float calculate_iou_scalar(const Box& a, const Box& b) {
    const int x1 = std::max(a.x, b.x);
    const int y1 = std::max(a.y, b.y);
    const int x2 = std::min(a.x + a.w, b.x + b.w);
    const int y2 = std::min(a.y + a.h, b.y + b.h);
    
    const int inter_w = x2 - x1;
    const int inter_h = y2 - y1;
    
    if (inter_w <= 0 || inter_h <= 0) return 0.0f;
    
    const int inter_area = inter_w * inter_h;
    const int area_a = a.w * a.h;
    const int area_b = b.w * b.h;
    const int union_area = area_a + area_b - inter_area;
    
    return static_cast<float>(inter_area) / static_cast<float>(union_area);
}

#define calculate_iou calculate_iou_scalar
#endif

// Fast IoU calculation (uses NEON if available)
inline float calculate_iou_fast(const Box& a, const Box& b) {
    return calculate_iou(a, b);
}

// Optimized NMS function (works on all platforms)
void apply_nms_fast(std::vector<Box>& detections, float iou_threshold = 0.45f) {
    const size_t num_dets = detections.size();
    
    if (num_dets <= 1) return;
    
    // Create indices and sort by score
    std::vector<int> indices(num_dets);
    for (size_t i = 0; i < num_dets; i++) indices[i] = i;
    
    std::sort(indices.begin(), indices.end(),
              [&detections](int i, int j) { return detections[i].score > detections[j].score; });
    
    std::vector<bool> suppressed(num_dets, false);
    std::vector<Box> filtered;
    filtered.reserve(num_dets);
    
    // Pre-cache class IDs for faster access
    std::vector<int> class_ids(num_dets);
    for (size_t i = 0; i < num_dets; i++) class_ids[i] = detections[i].id;
    
    // Main NMS loop
    for (size_t i = 0; i < num_dets; i++) {
        const int idx_i = indices[i];
        
        if (suppressed[idx_i]) continue;
        
        filtered.push_back(detections[idx_i]);
        const Box& box_i = detections[idx_i];
        const int class_i = class_ids[idx_i];
        
        for (size_t j = i + 1; j < num_dets; j++) {
            const int idx_j = indices[j];
            
            if (suppressed[idx_j]) continue;
            if (class_ids[idx_j] != class_i) continue;  // Only same class
            
            if (calculate_iou_fast(box_i, detections[idx_j]) > iou_threshold) {
                suppressed[idx_j] = true;
            }
        }
    }
    
    detections.swap(filtered);
}

// Alternative: Class-separated NMS (useful for many detections)
void apply_nms_by_class(std::vector<Box>& detections, float iou_threshold = 0.45f) {
    if (detections.empty()) return;
    
    // Group detections by class (COCO has 80 classes)
    std::vector<std::vector<Box>> class_boxes(80);
    
    for (auto& box : detections) {
        if (box.id >= 0 && box.id < 80) {
            class_boxes[box.id].push_back(box);
        }
    }
    
    detections.clear();
    
    // Process each class independently
    for (auto& boxes : class_boxes) {
        if (boxes.empty()) continue;
        
        // Sort by score descending
        std::sort(boxes.begin(), boxes.end(),
                  [](const Box& a, const Box& b) { return a.score > b.score; });
        
        std::vector<bool> suppressed(boxes.size(), false);
        
        for (size_t i = 0; i < boxes.size(); i++) {
            if (suppressed[i]) continue;
            
            detections.push_back(boxes[i]);
            
            for (size_t j = i + 1; j < boxes.size(); j++) {
                if (suppressed[j]) continue;
                
                if (calculate_iou_fast(boxes[i], boxes[j]) > iou_threshold) {
                    suppressed[j] = true;
                }
            }
        }
    }
}

//region globals

void sleep_ms(long ms) {
    struct timespec ts;
    ts.tv_sec  = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

#define CONFIDENCE_THRESHOLD 0.25f
#define TARGET_SIZE 320
#define CAM_W 320
#define CAM_H 320

std::vector<std::string> classes = {"person","bicycle","car","motorcycle","airplane","bus","train","truck","boat","traffic light","fire hydrant","stop sign","parking meter","bench","bird","cat","dog","horse","sheep","cow","elephant","bear","zebra","giraffe","backpack","umbrella","handbag","tie","suitcase","frisbee","skis","snowboard","sports ball","kite","baseball bat","baseball glove","skateboard","surfboard","tennis racket","bottle","wine glass","cup","fork","knife","spoon","bowl","banana","apple","sandwich","orange","broccoli","carrot","hot dog","pizza","donut","cake","chair","couch","potted plant","bed","dining table","toilet","tv","laptop","mouse","remote","keyboard","cell phone","microwave","oven","toaster","sink","refrigerator","book","clock","vase","scissors","teddy bear","hair drier","toothbrush"};

struct Buffer { void* start; size_t length; } cam_buf;
int cam_fd = -1;
int cam_stride = 0;
float fps=0;
uint8_t* rgb_buf;
std::vector<Box> detections;
bool ncnninitialized=0;

bool init_v4l2(const char* dev = "/dev/video0") {
    cam_fd = ::open(dev, O_RDWR);
    if (cam_fd < 0) return false;
    v4l2_format fmt{};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = CAM_W; fmt.fmt.pix.height = CAM_H;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    ioctl(cam_fd, VIDIOC_S_FMT, &fmt);
    v4l2_requestbuffers req{};
    req.count = 1; req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; req.memory = V4L2_MEMORY_MMAP;
    ioctl(cam_fd, VIDIOC_REQBUFS, &req);
    v4l2_buffer buf{};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; buf.memory = V4L2_MEMORY_MMAP; buf.index = 0;
    ioctl(cam_fd, VIDIOC_QUERYBUF, &buf);
    cam_buf.length = buf.length;
    cam_buf.start = mmap(NULL, buf.length, PROT_READ|PROT_WRITE, MAP_SHARED, cam_fd, buf.m.offset);
    ioctl(cam_fd, VIDIOC_QBUF, &buf);
    v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(cam_fd, VIDIOC_STREAMON, &type);
    return true;
}

void yuyv_to_rgb_not_fliped(const uint8_t* yuyv, uint8_t* rgb) {
    auto clamp = [](int v) { return (uint8_t)std::max(0, std::min(255, v)); };
    for (int i = 0, j = 0; i < CAM_W * CAM_H * 2; i += 4, j += 6) {
        int y0 = yuyv[i], u = yuyv[i+1]-128, y1 = yuyv[i+2], v = yuyv[i+3]-128;
        rgb[j] = clamp(y0 + 1.402*v); rgb[j+1] = clamp(y0 - 0.344*u - 0.714*v); rgb[j+2] = clamp(y0 + 1.772*u);
        rgb[j+3] = clamp(y1 + 1.402*v); rgb[j+4] = clamp(y1 - 0.344*u - 0.714*v); rgb[j+5] = clamp(y1 + 1.772*u);
    }
}
void yuyv_to_rgb_flipped(const uint8_t* yuyv, uint8_t* rgb) {
    auto clamp = [](int v) { return (uint8_t)std::max(0, std::min(255, v)); };

    const int width = CAM_W;
    const int height = CAM_H;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x += 2) {

            int i = (y * width + x) * 2;   // índice YUYV

            int y0 = yuyv[i];
            int u  = yuyv[i + 1] - 128;
            int y1 = yuyv[i + 2];
            int v  = yuyv[i + 3] - 128;

            // posições X espelhadas
            int fx0 = width - 1 - x;
            int fx1 = width - 2 - x;

            // índices RGB
            int j0 = (y * width + fx0) * 3;
            int j1 = (y * width + fx1) * 3;

            // pixel 0
            rgb[j0]     = clamp(y0 + 1.402 * v);
            rgb[j0 + 1] = clamp(y0 - 0.344 * u - 0.714 * v);
            rgb[j0 + 2] = clamp(y0 + 1.772 * u);

            // pixel 1
            rgb[j1]     = clamp(y1 + 1.402 * v);
            rgb[j1 + 1] = clamp(y1 - 0.344 * u - 0.714 * v);
            rgb[j1 + 2] = clamp(y1 + 1.772 * u);
        }
    }
}

#include <algorithm>
#include <cstdint>

void yuyv_to_rgb_flipped_optimized(const uint8_t* __restrict yuyv, uint8_t* __restrict rgb, int width, int height) {
    for (int y = 0; y < height; y++) {
        // Source pointer at start of the row
        const uint8_t* src = &yuyv[y * width * 2];
        
        // Destination pointer at the END of the current row (for the horizontal flip)
        // Each pixel is 3 bytes, so we start at the last pixel's first byte.
        uint8_t* dst_row_end = &rgb[y * width * 3 + (width - 1) * 3];

        for (int x = 0; x < width; x += 2) {
            // YUYV layout: [Y0, U, Y1, V]
            int y0 = src[0];
            int u  = src[1] - 128;
            int y1 = src[2];
            int v  = src[3] - 128;
            src += 4;

            // Fixed-point coefficients (shifted by 8 bits)
            int r_diff = (359 * v);
            int g_diff = (88 * u + 183 * v);
            int b_diff = (454 * u);

            // Pixel 0 (y0) - Write to the "current" mirrored position
            // Since x increases, we move dst_row_end backwards
            dst_row_end[0] = std::clamp((y0 << 8) + r_diff >> 8, 0, 255);
            dst_row_end[1] = std::clamp((y0 << 8) - g_diff >> 8, 0, 255);
            dst_row_end[2] = std::clamp((y0 << 8) + b_diff >> 8, 0, 255);

            // Pixel 1 (y1) - The pixel to the left of the previous one
            uint8_t* dst_prev = dst_row_end - 3;
            dst_prev[0] = std::clamp((y1 << 8) + r_diff >> 8, 0, 255);
            dst_prev[1] = std::clamp((y1 << 8) - g_diff >> 8, 0, 255);
            dst_prev[2] = std::clamp((y1 << 8) + b_diff >> 8, 0, 255);

            dst_row_end -= 6; // Move back two pixels
        }
    }
}

 

#include <FL/Fl_Gl_Window.H>
#include <FL/gl.h>

class View : public Fl_Gl_Window {
    uint8_t* rgb_data;
    std::vector<Box> boxes;
    float current_fps;
    GLuint tex = 0;

public:
    View(int W, int H)
        : Fl_Gl_Window(W, H, "NCNN + FLTK"),
          rgb_data(nullptr),
          current_fps(0) {}

    void update(uint8_t* d, const std::vector<Box>& b, float fps) {
        rgb_data = d;
        boxes = b;
        current_fps = fps;
        redraw();
    }

private:
    void init_gl() {
        if (!tex)
            glGenTextures(1, &tex);

        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

// #include <FL/gl.h>      // Required for gl_draw and gl_font
// #include <FL/glu.h>

// ... inside your View class ...


void draw() override {
    if (!valid()) {
        valid(1);
        glViewport(0, 0, w(), h());
        init_gl();
    }

    glClear(GL_COLOR_BUFFER_BIT);

    // --- 1. DRAW IMAGE ---
    if (rgb_data) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, tex);
        // Ensure white tint so the texture colors are natural
        glColor3f(1.0f, 1.0f, 1.0f); 
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, CAM_W, CAM_H-80, 0, GL_RGB, GL_UNSIGNED_BYTE, rgb_data);
        
        glBegin(GL_QUADS);
            glTexCoord2f(0, 1); glVertex2f(-1, -1);
            glTexCoord2f(1, 1); glVertex2f( 1, -1);
            glTexCoord2f(1, 0); glVertex2f( 1,  1);
            glTexCoord2f(0, 0); glVertex2f(-1,  1);
        glEnd();
        
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);
    }

    // --- 2. DRAW OVERLAY ---
    // Push attributes to save the current state (including color/texture)
    glPushAttrib(GL_ALL_ATTRIB_BITS); 
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, w(), h(), 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    draw_overlay_gl();

    // --- 3. RESTORE EVERYTHING ---
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    
    glPopAttrib(); // Restore all GL states to what they were before the overlay
    
    glFlush();
}
void draw_overlay_gl() {
    // 1. Get current window dimensions (FLTK methods)
    int cur_w = this->w();
    int cur_h = this->h();

    // 2. Define the original resolution the box data refers to
    // Replace these with your actual source image/model dimensions
    float orig_w = CAM_W; 
    float orig_h = CAM_H-80;

    // 3. Calculate scaling factors
    float sw = (float)cur_w / orig_w;
    float sh = (float)cur_h / orig_h;

    // --- Draw FPS ---
    char fps_text[32];
    snprintf(fps_text, sizeof(fps_text), "FPS: %.1f", current_fps);
    gl_font(FL_HELVETICA_BOLD, 16);
    glColor3f(1.0f, 1.0f, 0.0f);
    gl_draw(fps_text, 10, 25); 

    // --- Draw Boxes ---
    glColor3f(0.0f, 1.0f, 0.0f);
    glLineWidth(2.0f);

    for (auto& b : boxes) {
        // Apply scaling to coordinates and dimensions
        float rx = b.x * sw;
        float ry = b.y * sh;
        float rw = b.w * sw;
        float rh = b.h * sh;

        glBegin(GL_LINE_LOOP);
            glVertex2f(rx,      ry);
            glVertex2f(rx + rw, ry);
            glVertex2f(rx + rw, ry + rh);
            glVertex2f(rx,      ry + rh);
        glEnd();

        if (b.id < classes.size()) {
            // Draw label at the scaled x and slightly above the scaled y
            gl_draw(classes[b.id].c_str(), (int)rx, (int)(ry - 5));
        }
    }
}
void draw_overlay_gl1() {
    // FPS Text (Using gl_font and gl_draw)
    char fps_text[32];
    snprintf(fps_text, sizeof(fps_text), "FPS: %.1f", current_fps);
    
    gl_font(FL_HELVETICA_BOLD, 16);
    glColor3f(1.0f, 1.0f, 0.0f); // Yellow
    gl_draw(fps_text, 10, 25);   // gl_draw uses current raster position/ortho

    // Boxes (Using GL Primitives)
    glColor3f(0.0f, 1.0f, 0.0f); // Green
    glLineWidth(2.0f);

    for (auto& b : boxes) {
        // Draw Rectangle
        glBegin(GL_LINE_LOOP);
            // glVertex2f(b.x, b.y);
            // glVertex2f(b.x + b.w, b.y);
            // glVertex2f(b.x + b.w, b.y + b.h);
            // glVertex2f(b.x, b.y + b.h);
			float sx = w()  / CAM_W;
float sy = h() / CAM_H;

glVertex2f(b.x * sx,         b.y * sy);
glVertex2f((b.x+b.w) * sx,   b.y * sy);
glVertex2f((b.x+b.w) * sx,  (b.y+b.h) * sy);
glVertex2f(b.x * sx,        (b.y+b.h) * sy);

        glEnd();

        // Draw Label
        if (b.id < classes.size()) {
            gl_draw(classes[b.id].c_str(), b.x, b.y - 5);
        }
    }
}
};

View* ncnnwin;
void ui_update_cb(void*) {
    if (ncnnwin)
        ncnnwin->update(rgb_buf, detections, fps);
    ncnninitialized=1;
}
//region ncnnrun
void ncnnrun( ) {
    // Fl::screen_scale(0, 1.0f);
    // Fl::rgb_scaling(FL_RGB_SCALING_NEAREST);
    if (!init_v4l2()) return ;
    ncnn::Net net;
    // net.opt.use_fp16_storage = true; 
    // net.opt.lightmode = true;
    // net.opt.num_threads = 4;   // or 8 depending on your CPU
    net.load_param("../apptest/yolo26_no_end_to_end/model.ncnn.param");
    net.load_model("../apptest/yolo26_no_end_to_end/model.ncnn.bin");
    // net.load_param("yolo26_end_to_end/model.ncnn.param");
    // net.load_model("yolo26_end_to_end/model.ncnn.bin");

    // ncnnwin=new View(CAM_W, CAM_H-80);
    // View win(CAM_W, CAM_H-80);
        printf("view %d %d\n",CAM_W,CAM_H);
        // printf("view %d %d\n",win.w(),win.h());

//     win.size_range(CAM_W, CAM_H, CAM_W, CAM_H);
// win.resizable(nullptr);   // ← important
    // ncnnwin->show();////
    // ncnnwin->end();////
    // win.show();

    rgb_buf = new uint8_t[CAM_W * CAM_H * 3];
    // uint8_t* rgb_buf = new uint8_t[CAM_W * CAM_H * 3];
    auto last_time = std::chrono::high_resolution_clock::now();

    while (1) {
    // while (Fl::check()) {
        // perf1("");
        v4l2_buffer vbuf{}; vbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; vbuf.memory = V4L2_MEMORY_MMAP;
        if(ioctl(cam_fd, VIDIOC_DQBUF, &vbuf) < 0) continue;
        
        // perf1("speed");
        // perf1("");
        // yuyv_to_rgb_not_fliped((uint8_t*)cam_buf.start, rgb_buf);
        // yuyv_to_rgb_flipped_avx2((uint8_t*)cam_buf.start, rgb_buf,CAM_W, CAM_H-80);
        yuyv_to_rgb_flipped_optimized((uint8_t*)cam_buf.start, rgb_buf,CAM_W, CAM_H-80);
        // yuyv_to_rgb_flipped((uint8_t*)cam_buf.start, rgb_buf);
// perf1("speed");
        ioctl(cam_fd, VIDIOC_QBUF, &vbuf);
// perf1("");
        ncnn::Mat in = ncnn::Mat::from_pixels_resize(rgb_buf, ncnn::Mat::PIXEL_RGB, CAM_W, CAM_H, TARGET_SIZE, TARGET_SIZE);
        const float norm[3] = {1/255.f, 1/255.f, 1/255.f};
        in.substract_mean_normalize(0, norm);
        ncnn::Extractor ex = net.create_extractor();
        ex.input("in0", in);
        ncnn::Mat out;
        ex.extract("out0", out);

        detections.clear();
        // std::vector<Box> detections;
        for (int i = 0; i < out.w; i++) {
            float score = 0; int class_id = -1;
            for (int j = 4; j < 84; j++) {
                if (out.row(j)[i] > score) { score = out.row(j)[i]; class_id = j-4; }
            }
            if (score > CONFIDENCE_THRESHOLD) {
                float cx = out.row(0)[i] * CAM_W / TARGET_SIZE;
                float cy = out.row(1)[i] * CAM_H / TARGET_SIZE;
                float w = out.row(2)[i] * CAM_W / TARGET_SIZE;
                float h = out.row(3)[i] * CAM_H / TARGET_SIZE;
                detections.push_back({(int)(cx-w/2), (int)(cy-h/2), (int)w, (int)h, class_id, score});
            }
        }
        // perf1("inference speed");
        // perf1("");
        apply_nms_fast(detections, CONFIDENCE_THRESHOLD); //future no need nms 
        // perf1("nms speed"); //nms speed 0.000001401 sec

        // Calculate FPS
        auto now = std::chrono::high_resolution_clock::now();
        double diff = std::chrono::duration<double, std::milli>(now - last_time).count();
        fps = 1000.0f / (float)diff;
        last_time = now;
        // printf("view %d %d\n",win.w(),win.h());

        Fl::awake(ui_update_cb); 
        // ncnnwin->update(rgb_buf, detections, fps);
        // sleep_ms(1000/8);
    }
    return ;
}
void initncnn(){
    ncnnwin=new View(CAM_W, CAM_H-80);
    ncnnwin->resizable(ncnnwin);
    thread( ncnnrun).detach();
}
// int main() {
// 	Fl::lock();
//     initncnn();
//     while (!ncnninitialized) Fl::wait(0.01);
//     ncnnwin->show();
// 	return Fl::run(); 
// }