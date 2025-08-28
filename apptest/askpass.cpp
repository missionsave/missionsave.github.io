// g++ askpass.cpp -o askpass `fltk-config --cxxflags --ldflags`

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Secret_Input.H>
#include <FL/Fl_Button.H>
#include <thread>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <condition_variable>

static std::string g_password;
static Fl_Window* g_win = nullptr;
static std::mutex mtx;
static std::condition_variable cv;
static bool ready = false; // Flag to indicate if input is ready

static void ok_cb(Fl_Widget*, void* v) {
    Fl_Secret_Input* in = static_cast<Fl_Secret_Input*>(v);
    g_password = in->value();
    in->value("");              // limpa input
    if (g_win) g_win->hide();   // fecha janela
    {
        std::lock_guard<std::mutex> lk(mtx);
        ready = true; // Signal that password is ready
    }
    cv.notify_one(); // Notify waiting thread
}

void ask_password_gui() {
    Fl_Window win(300, 100, "Password");
    g_win = &win;
    Fl_Secret_Input input(80, 20, 200, 25, "Senha:");
    Fl_Button ok(110, 60, 80, 25, "OK");
    ok.callback(ok_cb, &input);
    win.end();
    win.show();
    Fl::run();
}

int main() {
    std::thread t([]{ ask_password_gui(); });

    // Wait for password input
    {
        std::unique_lock<std::mutex> lk(mtx);
        cv.wait(lk, []{return ready;});
    }
    t.join(); // Ensure thread is finished before proceeding

    if (!g_password.empty()) {
        FILE* pipe = popen("sudo mount /dev/sdc2 ~/pen", "w");
        if (pipe) {
            std::string pw = g_password + "\n";
            fwrite(pw.c_str(), 1, pw.size(), pipe);
            fflush(pipe);
            pclose(pipe);
        }
        // apaga a password da memória
        std::fill(g_password.begin(), g_password.end(), '\0');
        g_password.clear();
    }
}
