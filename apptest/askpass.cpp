// g++ ./askpass.cpp -o ./askpass -lfltk -lX11 -lXinerama -lXrender -lXfixes -lfontconfig -lfreetype -lXft -lXcursor

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <iostream>
#include <thread>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

// returns true if sudo -n fails (i.e. password is required)
bool run_command_check(const std::string &cmd) {
    int ret = std::system((cmd + " >/dev/null 2>&1").c_str());
    return ret != 0;
}

struct PassDialog {
    Fl_Window *win;
    Fl_Input  *input;
    Fl_Button *ok;
    std::string password;
    bool done = false;
};

void ok_cb(Fl_Widget *w, void *data) {
    PassDialog *dlg = static_cast<PassDialog*>(data);
    dlg->password = dlg->input->value();
    dlg->done = true;
    dlg->win->hide();
}

void ask_password_gui(std::string &out_pass) {
    PassDialog dlg;
    dlg.win   = new Fl_Window(300,100,"Password Required");
    dlg.input = new Fl_Input(80,20,200,30,"Password:");
    dlg.ok    = new Fl_Button(120,60,60,30,"OK");
    dlg.ok->callback(ok_cb, &dlg);
    dlg.win->end();
    dlg.win->show();

    // wait until user clicks OK
    while (!dlg.done) {
        Fl::wait();
    }
    out_pass = dlg.password;
    delete dlg.win;
}

// if password is empty, we assume password-less sudo works
void do_mount(const std::string &password) {
    if (password.empty()) {
        // no password needed: use sudo -n which will error out if password IS needed
        int ret = std::system("sudo -n mount /dev/sdb2 ~/pen");
        if (ret != 0) {
            std::cerr << "mount failed without password. Perhaps password is actually required.\n";
        }
        return;
    }

    // password is provided: pipe it to sudo -S
    int pipefd[2];
    if (pipe(pipefd)<0) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    pid_t pid = fork();
    if (pid<0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (pid==0) {
        // child: read password from pipe
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        execl("/bin/sh","sh","-c","sudo -S mount /dev/sdb2 ~/pen",(char*)NULL);
        perror("execl");
        exit(EXIT_FAILURE);
    } else {
        // parent: write password + newline
        close(pipefd[0]);
        std::string pw = password + "\n";
        write(pipefd[1], pw.c_str(), pw.size());
        close(pipefd[1]);
        wait(nullptr);
    }
}

int main() {
    std::string password;

    // first check: is password required?
    if (run_command_check("sudo -n ls /root")) {
        std::cout << "Password is required for sudo.\n";
        // ask GUI on main thread
        ask_password_gui(password);
    }
    else {
        std::cout << "No password needed for sudo.\n";
    }

    // perform the mount, with or without the password
    do_mount(password);

    // scrub the password
    std::fill(password.begin(), password.end(), '\0');
    password.clear();

    return 0;
}
