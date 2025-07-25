#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/scrnsaver.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <chrono>
#include <thread>
#include <sstream>
#include <map>

// Global display connection to reuse
Display* display = nullptr;

// Initialize display connection
bool init_display() {
    if (!display) {
        display = XOpenDisplay(nullptr);
    }
    return display != nullptr;
}

// Cleanup display connection
void cleanup_display() {
    if (display) {
        XCloseDisplay(display);
        display = nullptr;
    }
}

// Get PID of active window
pid_t get_active_window_pid() {
    if (!init_display()) return -1;

    static Atom net_active_window = None;
    static Atom net_wm_pid = None;

    if (net_active_window == None) {
        net_active_window = XInternAtom(display, "_NET_ACTIVE_WINDOW", True);
        net_wm_pid = XInternAtom(display, "_NET_WM_PID", True);
    }

    Window root = DefaultRootWindow(display);
    Window active_window = 0;
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    unsigned char* prop = nullptr;

    if (XGetWindowProperty(display, root, net_active_window, 0, 1, False,
                           XA_WINDOW, &actual_type, &actual_format,
                           &nitems, &bytes_after, &prop) == Success && prop) {
        active_window = *(Window*)prop;
        XFree(prop);
    } else {
        return -1;
    }

    if (XGetWindowProperty(display, active_window, net_wm_pid, 0, 1, False,
                           XA_CARDINAL, &actual_type, &actual_format,
                           &nitems, &bytes_after, &prop) == Success && prop) {
        pid_t pid = *(pid_t*)prop;
        XFree(prop);
        return pid;
    }

    return -1;
}

// Get audio PIDs (cached)
std::vector<pid_t> get_audio_pids() {
    static std::vector<pid_t> cached_pids;
    static auto last_update = std::chrono::steady_clock::now();
    constexpr int UPDATE_INTERVAL = 5; // seconds

    auto now = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::seconds>(now - last_update).count() < UPDATE_INTERVAL) {
        return cached_pids;
    }

    cached_pids.clear();
    FILE* fp = popen("pactl list sink-inputs | grep -Po 'application.process.id = \"\\K\\d+(?=\")'", "r");
    if (!fp) return cached_pids;

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        char* end;
        pid_t pid = strtol(line, &end, 10);
        if (end != line) {
            cached_pids.push_back(pid);
        }
    }
    pclose(fp);
    last_update = now;

    return cached_pids;
}

// Cache for process parent relationships
std::map<pid_t, pid_t> parent_cache;

// Check if process is descendant (with caching)
bool is_descendant(pid_t child_pid, pid_t parent_pid) {
    if (child_pid <= 1 || child_pid == parent_pid) 
        return child_pid == parent_pid;

    pid_t current = child_pid;
    std::vector<pid_t> path;

    while (current > 1 && current != parent_pid) {
        // Check cache first
        auto it = parent_cache.find(current);
        if (it != parent_cache.end()) {
            current = it->second;
            path.push_back(current);
            continue;
        }

        // Read from proc filesystem
        std::string path_str = "/proc/" + std::to_string(current) + "/status";
        std::ifstream file(path_str);
        if (!file) break;

        std::string line;
        pid_t ppid = -1;
        while (std::getline(file, line)) {
            if (line.compare(0, 5, "PPid:") == 0) {
                ppid = strtol(line.c_str() + 5, nullptr, 10);
                break;
            }
        }

        if (ppid < 0) break;
        parent_cache[current] = ppid;
        current = ppid;
        path.push_back(current);
    }

    // Cache the entire path
    for (size_t i = 0; i < path.size() - 1; ++i) {
        parent_cache[path[i]] = path[i+1];
    }

    return current == parent_pid;
}

// Get idle time using existing display connection
unsigned long get_idle_time_ms() {
    if (!init_display()) return 0;

    static XScreenSaverInfo* info = nullptr;
    if (!info) {
        info = XScreenSaverAllocInfo();
    }
    if (!info) return 0;

    XScreenSaverQueryInfo(display, DefaultRootWindow(display), info);
    return info->idle;
}

// Turn off screen
void force_screen_off() {
    std::system("xset dpms force off");
}

int main() {
    constexpr int IDLE_THRESHOLD = 60*1; // 5 minutes
    constexpr int SLEEP_INTERVAL = 5;    // seconds
    int idle_seconds = 0;
    pid_t last_focused_pid = -2;         // Invalid initial value
    bool last_audio_state = false;

    while (true) {
        unsigned long idle_ms = get_idle_time_ms();

        if (idle_ms < 5000) {
            // User activity
            idle_seconds = 0;
            std::this_thread::sleep_for(std::chrono::seconds(5));
            continue;
        }

        pid_t focused_pid = get_active_window_pid();
        bool audio_focused = false;

        // Only recheck audio focus when PID changes
        if (focused_pid != last_focused_pid || focused_pid == -1) {
            last_focused_pid = focused_pid;
            last_audio_state = false;

            if (focused_pid != -1) {
                auto audio_pids = get_audio_pids();
                for (pid_t audio_pid : audio_pids) {
                    if (is_descendant(audio_pid, focused_pid)) {
                        audio_focused = true;
                        last_audio_state = true;
                        std::cout << "✅ PID " << audio_pid 
                                  << " is receiving focus (via parent PID " 
                                  << focused_pid << ")\n";
                        break;
                    }
                }
            }
        } else {
            audio_focused = last_audio_state;
        }

        if (audio_focused) {
            idle_seconds = 0;
        } else {
            idle_seconds += SLEEP_INTERVAL;
        }

        std::cout << "Idle: " << idle_seconds << "s\r" << std::flush;

        if (idle_seconds >= IDLE_THRESHOLD) {
            std::cout << "\nTurning off screen...\n";
            force_screen_off();
            idle_seconds = 0;
            // Longer sleep after turning off screen
            std::this_thread::sleep_for(std::chrono::seconds(10));
            continue;
        }

        std::this_thread::sleep_for(std::chrono::seconds(SLEEP_INTERVAL));
    }

    cleanup_display();
    return 0;
}