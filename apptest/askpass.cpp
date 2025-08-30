// Compile with:
//   g++ askpass.cpp -o askpass -lfltk -lX11 -lXinerama -lXrender -lXfixes -lfontconfig -lfreetype -lXft -lXcursor

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>

struct PassDialog {
    Fl_Window *win;
    Fl_Input  *input;
    Fl_Button *btn;
    std::string password;
    bool done = false;
};

static void ok_cb(Fl_Widget* w, void* data) {
    auto dlg = static_cast<PassDialog*>(data);
    dlg->password = dlg->input->value();
    dlg->done = true;
    dlg->win->hide();
}

static void ask_password_gui(std::string &out) {
    PassDialog dlg;
    dlg.win   = new Fl_Window(300, 100, "sudo password");
    dlg.input = new Fl_Input(80, 20, 200, 30, "Password:");
    dlg.btn   = new Fl_Button(120, 60, 60, 30, "OK");
    dlg.btn->callback(ok_cb, &dlg);
    dlg.win->end();
    dlg.win->show();

    // Wait for user to click OK
    while (!dlg.done) Fl::wait();

    out = dlg.password;
    delete dlg.win;
}

static std::string join_args(const std::vector<std::string>& v) {
    std::ostringstream os;
    for (const auto& s : v) {
        os << "'" << s << "' ";
    }
    return os.str();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: askpass <command> [args...]\n";
        return 1;
    }

    // Collect the user-requested command
    std::vector<std::string> cmd(argv + 1, argv + argc);
    std::string args_str = join_args(cmd);
    std::string password;

    // 1) Check if sudo -n works (no password)
    {
        std::string test = "sudo -n " + args_str + " >/dev/null 2>&1";
        if (std::system(test.c_str()) != 0) {
            ask_password_gui(password);
        }
    }

    // 2) Build and run the final sudo command
    std::string full;
    if (password.empty()) {
        full = "sudo -n " + args_str;
    } else {
        // pipe password into sudo -S
        // the printf approach avoids a manual pipe()
        full = "printf '%s\\n' \"" + password + "\" | sudo -S " + args_str;
    }

    int status = std::system(full.c_str());

    // 3) Scrub password from memory
    std::fill(password.begin(), password.end(), '\0');
    password.clear();

    return WEXITSTATUS(status);
}
