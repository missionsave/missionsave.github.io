
// g++ -std=c++20 -O2 msvscreensaver.cpp -o msvscreensaver -lX11 -lXss && pkill msvscreensaver && sudo cp ./msvscreensaver /usr/local/bin && msvscreensaver &
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
#include <unordered_set>
#include <dirent.h>

using namespace std;

// Comando para suspender
void suspend_system() {
    std::system("systemctl suspend");
}

mutex mtxgawid;
// Obtem PID da janela ativa (foco) no X11
pid_t get_active_window_pid() {
	mtxgawid.lock();

    Display* display = XOpenDisplay(nullptr);
    if (!display){
		mtxgawid.unlock();
		return -1;
	} 

    Atom net_active_window = XInternAtom(display, "_NET_ACTIVE_WINDOW", True);
    Atom net_wm_pid = XInternAtom(display, "_NET_WM_PID", True);
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    unsigned char* prop = nullptr;

    Window root = DefaultRootWindow(display);
    Window active_window = 0;

    if (XGetWindowProperty(display, root, net_active_window, 0, (~0L), False,
                           AnyPropertyType, &actual_type, &actual_format,
                           &nitems, &bytes_after, &prop) == Success && prop) {
        active_window = *(Window*)prop;
        XFree(prop);
    } else {
        XCloseDisplay(display);

		mtxgawid.unlock();
        return -1;
    }

    if (XGetWindowProperty(display, active_window, net_wm_pid, 0, 1, False,
                           XA_CARDINAL, &actual_type, &actual_format,
                           &nitems, &bytes_after, &prop) == Success && prop) {
        pid_t pid = *(pid_t*)prop;
        XFree(prop);
        XCloseDisplay(display);
		mtxgawid.unlock();
        return pid;
    }

    XCloseDisplay(display);
	mtxgawid.unlock();
    return -1;
}



// Extrai PIDs dos processos de áudio do output do pactl
std::vector<pid_t> get_audio_pids() {
    std::vector<pid_t> audio_pids;
    FILE* fp = popen("pactl list sink-inputs | grep -Po 'application.process.id = \"\\K\\d+(?=\")'", "r");
    if (!fp) return audio_pids;

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        try {
            pid_t pid = std::stoi(line);
            audio_pids.push_back(pid);
        } catch (...) {
            continue;
        }
    }
    pclose(fp);

    return audio_pids;
}

// Verifica se child_pid é descendente de parent_pid
bool is_descendant(pid_t child_pid, pid_t parent_pid) {
    if (child_pid <= 1) return false;
    if (child_pid == parent_pid) return true;

    std::string path = "/proc/" + std::to_string(child_pid) + "/status";
    std::ifstream file(path);
    if (!file.is_open()) return false;

    std::string line;
    pid_t ppid = -1;

    while (std::getline(file, line)) {
        if (line.rfind("PPid:", 0) == 0) {
            try {
                ppid = std::stoi(line.substr(5));
            } catch (...) {
                return false;
            }
            break;
        }
    }

    return is_descendant(ppid, parent_pid);
}

// Verifica se algum PID de áudio é descendente do PID da janela ativa
bool is_audio_focused(pid_t focused_pid) {
    auto audio_pids = get_audio_pids();
    
    for (pid_t audio_pid : audio_pids) {
        if (is_descendant(audio_pid, focused_pid)) {
            std::cout << "✅ PID " << audio_pid << " is receiving focus (via parent PID " << focused_pid << ")\n";
            return true;
        }
    }
    
    return false;
}

// Obtém o tempo de inatividade do teclado e rato em ms (XScreenSaver)
unsigned long get_idle_time_ms() {
    Display* dpy = XOpenDisplay(nullptr);
    if (!dpy) return 0;

    XScreenSaverInfo* info = XScreenSaverAllocInfo();
    if (!info) {
        XCloseDisplay(dpy);
        return 0;
    }

    XScreenSaverQueryInfo(dpy, DefaultRootWindow(dpy), info);
    unsigned long idle = info->idle;

    XFree(info);
    XCloseDisplay(dpy);
    return idle;
}

// Comando para desligar o ecrã
void force_screen_off() {
    std::system("xset dpms force off");
}
pid_t focused_pid=0;
int main() {
// void initscrensaver() {
    constexpr int IDLE_SCREEN_OFF = 60 * 5;   // 5 min → desligar ecrã
    constexpr int IDLE_SUSPEND    = 60 * 10;  // 10 min → suspender

    int idle_seconds = 0;
    int interval = 30;

    while (true) {
        unsigned long idle_ms = get_idle_time_ms();
        focused_pid = get_active_window_pid();

        unsigned long idle_sec_real = idle_ms / 1000;

        // --- lógica existente ---
        if (idle_ms < 1000 * interval) {
            idle_seconds = 0;
        } else if (focused_pid == -1 || !is_audio_focused(focused_pid)) {
            idle_seconds += interval;
        } else {
            idle_seconds = 0;
        }

        std::cout << "Idle logic: " << idle_seconds 
                  << "s | Real idle: " << idle_sec_real << "s\r" << std::flush;

        // --- desligar ecrã ---
        if (idle_seconds >= IDLE_SCREEN_OFF) {
            std::cout << "\nDesligando ecrã...\n";
            force_screen_off();
            idle_seconds = 0;
        }

        // --- NOVO: suspender sistema ---
        if (idle_sec_real >= IDLE_SUSPEND &&  (focused_pid == -1 || !is_audio_focused(focused_pid))){
        // if (idle_sec_real >= IDLE_SUSPEND) {
            std::cout << "\nSuspender sistema...\n";
            suspend_system();
            std::this_thread::sleep_for(std::chrono::seconds(5)); // evita loop imediato
        }

        std::this_thread::sleep_for(std::chrono::seconds(interval));
    }
    return 0;
}
 
