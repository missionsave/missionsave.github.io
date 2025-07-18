//  g++ docker.cpp -o docker2 -l:libfltk.a -lX11 -Os -s -flto -std=c++20 -lXext -lGL -lGLU -lXft -lXrender -lXcursor -lXinerama -lXfixes -lfontconfig -lfreetype -lz -lm -ldl -lpthread -lstdc++ -w -lXtst

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/fl_ask.H>
#include <FL/x.H> // X11-specific functions

#include <string>
#include <iostream>
#include <thread>

#ifdef __linux__
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <string.h> // For strlen
#endif

using namespace std;


#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <X11/Xlib.h>
#include <iostream>
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <X11/keysym.h>
#include <unistd.h>

void typeText(Display* display, const std::string& text) {
    for (char c : text) {
        KeySym keysym = XStringToKeysym(std::string(1, c).c_str());
        KeyCode keycode = XKeysymToKeycode(display, keysym);

        if (keycode != 0) {
            XTestFakeKeyEvent(display, keycode, True, CurrentTime);  // Press
            XTestFakeKeyEvent(display, keycode, False, CurrentTime); // Release
            XFlush(display);
            usleep(50000); // Small delay between keys
        }
    }
}

int pastetype() {
    Display* display = XOpenDisplay(nullptr);
    if (!display) return 1;

    typeText(display, "Hello");

    XCloseDisplay(display);
    return 0;
}

void copy_to_clipboard(const char* text) {
    Fl::copy(text, strlen(text), 1); // Try FLTK's method first
    return;
    // Platform-specific fallbacks
#ifdef WIN32
    if (OpenClipboard(NULL)) {
        EmptyClipboard();
        HGLOBAL hglb = GlobalAlloc(GMEM_MOVEABLE, strlen(text) + 1);
        if (hglb) {
            char* buf = (char*)GlobalLock(hglb);
            strcpy(buf, text);
            GlobalUnlock(hglb);
            SetClipboardData(CF_TEXT, hglb);
        }
        CloseClipboard();
    }
#elif defined(__APPLE__)
    // macOS specific code would go here
#else
    // X11 specific code
    Display* dpy = fl_display;
    if (dpy) {
        Atom clipboard = XInternAtom(dpy, "CLIPBOARD", False);
        Atom utf8 = XInternAtom(dpy, "UTF8_STRING", False);
        Atom targets = XInternAtom(dpy, "TARGETS", False);
        Atom sel = XInternAtom(dpy, "PRIMARY", False);
        
        XSetSelectionOwner(dpy, clipboard, fl_xid(Fl::first_window()), CurrentTime);
        XSetSelectionOwner(dpy, sel, fl_xid(Fl::first_window()), CurrentTime);
    }
#endif
}





#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <unistd.h>

void sendCtrlV(Display* dpy) {
    Window focus;
    int revert;
    
    // Get the currently focused window
    XGetInputFocus(dpy, &focus, &revert);
    
    if (focus == None) {
        focus = DefaultRootWindow(dpy);
    }

    KeyCode ctrl = XKeysymToKeycode(dpy, XK_Control_L);
    KeyCode v = XKeysymToKeycode(dpy, XK_V);

    // Press Control
    XTestFakeKeyEvent(dpy, ctrl, True, CurrentTime);
    
    // Press V
    XTestFakeKeyEvent(dpy, v, True, CurrentTime);
    
    // Release V
    XTestFakeKeyEvent(dpy, v, False, CurrentTime);
    
    // Release Control
    XTestFakeKeyEvent(dpy, ctrl, False, CurrentTime);
    
    XFlush(dpy);
}

int paste() {
    Display* dpy = XOpenDisplay(nullptr);
    if (!dpy) return 1;

    // We need XTest extension for reliable key simulation
    int event_base, error_base;
    if (!XTestQueryExtension(dpy, &event_base, &error_base, &event_base, &error_base)) {
        XCloseDisplay(dpy);
        return 1;
    }

    sendCtrlV(dpy);
    XCloseDisplay(dpy);
    return 0;
}


Fl_Window* winpaste;
int winpop() {
    winpaste = new Fl_Window(100, 100, 300, 200, "Popup");
    Fl_Button* btn = new Fl_Button(100, 80, 100, 40, "Click Me");

    btn->callback([](Fl_Widget*, void*) {
        std::cout << "Button clicked\n";

		copy_to_clipboard("test");
		paste();
		winpaste->hide();
    });

    winpaste->end();
    winpaste->show();

    // Prevent window from stealing focus
    Display* dpy = fl_display;
    Window xid = fl_xid(winpaste);

    XSetWindowAttributes attrs;
    attrs.override_redirect = True;
    XChangeWindowAttributes(dpy, xid, CWOverrideRedirect, &attrs);
    XMapRaised(dpy, xid); // Show without focus

	Fl::wait();
    return 1;
}


#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <iostream>

int listenkey() {
    Display* dpy = XOpenDisplay(nullptr);
    if (!dpy) {
        std::cerr << "Cannot open display\n";
        return 1;
    }

    Window root = DefaultRootWindow(dpy);
    KeyCode v_key = XKeysymToKeycode(dpy, XStringToKeysym("v"));

    // Grab Alt+V globally
    XGrabKey(dpy, v_key, Mod1Mask, root, True,
             GrabModeAsync, GrabModeAsync);
    XSelectInput(dpy, root, KeyPressMask);

    while (true) {
        XEvent ev;
        XNextEvent(dpy, &ev);

        if (ev.type == KeyPress &&
            ev.xkey.keycode == v_key &&
            (ev.xkey.state & Mod1Mask)) {
            std::cout << "Alt+V pressed globally!\n";
			winpop();
            // You can trigger your FLTK logic here
        }
    }

    XCloseDisplay(dpy);
    return 0;
}







// Global event handler
int global_handler(int event) {
    if (event == FL_KEYDOWN) {
        int key = Fl::event_key();
        int state = Fl::event_state();

        // Check for Alt+V
        if ((state & FL_ALT) && key == 'v') {
            std::cout << "🔑 Alt+V pressed!\n";
			winpop();
            // You can trigger any action here
            return 1; // Event handled
        }
    }
    return 0; // Let other handlers process it
}



    Window current_focus;
    Window last_focus;
int getlastfocus() {
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        std::cerr << "Cannot open display\n";
        return 1;
    }

    Window root = DefaultRootWindow(display);
    // Window current_focus;
    // Window last_focus;
    int revert;

    // Get initial focus
    XGetInputFocus(display, &current_focus, &revert);
    last_focus = current_focus;

    // Subscribe to focus events on root
    XSelectInput(display, root, FocusChangeMask);

    while (true) {
        XEvent event;
        XNextEvent(display, &event); // Blocks until an event is received

        if (event.type == FocusOut) {
            last_focus = current_focus;
            XGetInputFocus(display, &current_focus, &revert);
            std::cout << "Focus changed. Previous window: " << last_focus
                      << ", Current window: " << current_focus << std::endl;
        }
    }

    XCloseDisplay(display);
    return 0;
}

void set_dock_properties(Fl_Window* win) {
#ifdef __linux__
    // Get the X11 display and window handle
    Display* dpy = fl_display; // FLTK's global X11 display
    Window xid = fl_xid(win);  // FLTK's X11 window handle

    // Set as dock type
    Atom wtype = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
    Atom dock_type = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DOCK", False);
    XChangeProperty(dpy, xid, wtype, XA_ATOM, 32, PropModeReplace, 
                   (unsigned char*)&dock_type, 1);

    // Reserve screen space (strut)
    unsigned long strut[12] = {0};
    strut[3] = win->h(); // Reserve "bottom" space (height of the bar)
    strut[10] = 0;       // Start of reserved area (X start)
    strut[11] = win->w() - 1; // End of reserved area (X end)

    Atom partial_strut = XInternAtom(dpy, "_NET_WM_STRUT_PARTIAL", False);
    XChangeProperty(dpy, xid, partial_strut, XA_CARDINAL, 32, PropModeReplace,
                   (unsigned char*)strut, 12);

    // Backward compatibility
    Atom simple_strut = XInternAtom(dpy, "_NET_WM_STRUT", False);
    unsigned long simple[4] = {0, 0, 0, win->h()};
    XChangeProperty(dpy, xid, simple_strut, XA_CARDINAL, 32, PropModeReplace,
                   (unsigned char*)simple, 4);

    // Keep window always on top
    Atom wm_state = XInternAtom(dpy, "_NET_WM_STATE", False);
    Atom wm_above = XInternAtom(dpy, "_NET_WM_STATE_ABOVE", False);
    XChangeProperty(dpy, xid, wm_state, XA_ATOM, 32, PropModeAppend,
                   (unsigned char*)&wm_above, 1);
#endif
}

Fl_Window* win;
#include <unistd.h>
#include <sys/wait.h>
#include <system_error>
#include <string_view>
#include <vector>
#include <iostream>

pid_t launch_app(std::string_view app, std::initializer_list<std::string_view> args = {}) {
    pid_t pid = fork();

    if (pid == -1) {
        throw std::system_error(errno, std::generic_category(), "fork failed");
    }

    if (pid == 0) { // Child process
        // Detach from parent so app survives launcher exit
        setsid(); // Start a new session

        // Build argument vector
        std::vector<const char*> argv;
        argv.reserve(args.size() + 2); // app + args + null terminator

        argv.push_back(app.data());
        for (const auto& arg : args) {
            argv.push_back(arg.data());
        }
        argv.push_back(nullptr);

        // Execute with path search
        execvp(app.data(), const_cast<char* const*>(argv.data()));

        // If exec fails
        std::cerr << "Failed to execute " << app << "\n";
        _exit(EXIT_FAILURE);
    }

    // Parent returns child's PID
    return pid;
}


// int main() {
//     try {
//         // Example 1: Launch basic xterm
//         launch_app("xterm");
        
//         // Example 2: Launch xterm running tmux
//         launch_app("xterm", {"-e", "tmux"});
        
//         // Example 3: Launch Firefox with URL
//         launch_app("firefox", {"https://www.example.com"});
        
//     } catch (const std::system_error& e) {
//         std::cerr << "Error: " << e.what() << " (" << e.code() << ")\n";
//         return 1;
//     }
    
//     return 0;
// }
#include <filesystem>
#include <string>
#include <vector>
#include <fstream>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <iostream>

namespace fs = std::filesystem;
#include <X11/Xutil.h> // Add this include for WindowAttributes

void focus_window(Display* display, Window window) {
    // First check if the window is viewable
    XWindowAttributes attrs;
    if (XGetWindowAttributes(display, window, &attrs)) {
        if (attrs.map_state != IsViewable) {
            // Window isn't visible, map it first
            XMapWindow(display, window);
            XFlush(display);
            usleep(100000); // Small delay to let window manager handle the map request
        }
    }

    // Get the window manager frame if it exists (for decorated windows)
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    unsigned char* frame_extents_data = nullptr;
    Atom frame_extents = XInternAtom(display, "_NET_FRAME_EXTENTS", False);
    
    if (XGetWindowProperty(display, window, frame_extents, 0, 4, 
                          False, AnyPropertyType, &actual_type, &actual_format,
                          &nitems, &bytes_after, &frame_extents_data) == Success && frame_extents_data) {
        // This window has frame extents (is decorated)
        XFree(frame_extents_data);
        
        // Try to get the frame window
        unsigned char* frame_window_data = nullptr;
        Atom frame_atom = XInternAtom(display, "_NET_WM_FRAME_WINDOW", False);
        if (XGetWindowProperty(display, window, frame_atom, 0, 1, 
                              False, XA_WINDOW, &actual_type, &actual_format,
                              &nitems, &bytes_after, &frame_window_data) == Success && frame_window_data) {
            Window frame = *reinterpret_cast<Window*>(frame_window_data);
            XFree(frame_window_data);
            window = frame; // Focus the frame instead
        }
    }

    // Raise the window first
    XRaiseWindow(display, window);
    XFlush(display);
    
    // Set input focus - only if the window is viewable
    if (XGetWindowAttributes(display, window, &attrs)) {
        if (attrs.map_state == IsViewable) {
            XSetInputFocus(display, window, RevertToParent, CurrentTime);
            XFlush(display);
        }
    }

    // EWMH-compliant activation
    Atom net_active_window = XInternAtom(display, "_NET_ACTIVE_WINDOW", False);
    XEvent event;
    memset(&event, 0, sizeof(event));
    event.xclient.type = ClientMessage;
    event.xclient.window = window;
    event.xclient.message_type = net_active_window;
    event.xclient.format = 32;
    event.xclient.data.l[0] = 1;  // Source indication (1 = application)
    event.xclient.data.l[1] = CurrentTime;
    event.xclient.data.l[2] = 0;  // Should be 0 when from application
    
    XSendEvent(display, DefaultRootWindow(display), False,
              SubstructureRedirectMask | SubstructureNotifyMask,
              &event);
    XFlush(display);
    
    // Small delay to let window manager process the request
    usleep(50000);
}

#include <algorithm>  // Add this include for std::find

// ... [previous includes and namespace] ...

bool is_process_running(const std::string& process_name) {
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        std::cerr << "Failed to open X11 display\n";
        return false;
    }

    bool process_found = false;
    std::vector<pid_t> matching_pids;

    // First pass: Find all matching PIDs
    for (const auto& entry : fs::directory_iterator("/proc")) {
        if (entry.is_directory()) {
            try {
                pid_t pid = static_cast<pid_t>(std::stol(entry.path().filename().string()));
                std::ifstream cmdline(entry.path() / "cmdline");
                if (cmdline) {
                    std::string cmd;
                    std::getline(cmdline, cmd);
                    if (!cmd.empty() && cmd.find(process_name) != std::string::npos) {
                        matching_pids.push_back(pid);
                        process_found = true;
                    }
                }
            } catch (...) {
                continue;
            }
        }
    }

    // Second pass: Find and focus windows for these PIDs
    if (process_found) {
        Window root = DefaultRootWindow(display);
        Atom pid_atom = XInternAtom(display, "_NET_WM_PID", False);
        Window* windows = nullptr;
        unsigned int count = 0;
        
        if (XQueryTree(display, root, &root, &root, &windows, &count)) {
            for (unsigned int i = 0; i < count; i++) {
                Atom actual_type;
                int actual_format;
                unsigned long nitems, bytes_after;
                unsigned char* pid_data = nullptr;
                
// In the is_process_running function's window finding section:
// In the window finding section of is_process_running():
if (XGetWindowProperty(display, windows[i], pid_atom, 0, 1, 
                     False, XA_CARDINAL, &actual_type, &actual_format,
                     &nitems, &bytes_after, &pid_data) == Success && pid_data) {
    pid_t window_pid = *reinterpret_cast<pid_t*>(pid_data);
    XFree(pid_data);
    
    if (std::find(matching_pids.begin(), matching_pids.end(), window_pid) != matching_pids.end()) {
        // Additional check to verify this is a top-level window
        Atom window_type = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
        Atom normal_type = XInternAtom(display, "_NET_WM_WINDOW_TYPE_NORMAL", False);
        unsigned char* window_type_data = nullptr;
        
        if (XGetWindowProperty(display, windows[i], window_type, 0, 1, 
                             False, XA_ATOM, &actual_type, &actual_format,
                             &nitems, &bytes_after, &window_type_data) == Success && window_type_data) {
            Atom type = *reinterpret_cast<Atom*>(window_type_data);
            XFree(window_type_data);
            
            if (type == normal_type) {
                focus_window(display, windows[i]);
            }
        } else {
            // Fallback if window type can't be determined
            focus_window(display, windows[i]);
        }
    }
}
            }
            XFree(windows);
        }
    }

    XCloseDisplay(display);
    return process_found;
}
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <unistd.h>
#include <iostream>

bool getWindowFromPid(Display* display, Window root, unsigned long pid, Window& result) {
    Atom atomPID = XInternAtom(display, "_NET_WM_PID", True);
    if (atomPID == None) return false;

    Window parent;
    Window *children;
    unsigned int nchildren;

    if (XQueryTree(display, root, &root, &parent, &children, &nchildren)) {
        for (unsigned int i = 0; i < nchildren; ++i) {
            Atom actualType;
            int actualFormat;
            unsigned long nItems, bytesAfter;
            unsigned char *propPID = nullptr;

            if (Success == XGetWindowProperty(display, children[i], atomPID, 0, 1, False,
                                              XA_CARDINAL, &actualType, &actualFormat,
                                              &nItems, &bytesAfter, &propPID)) {
                if (propPID) {
                    if (pid == *(unsigned long*)propPID) {
                        result = children[i];
                        XFree(propPID);
                        XFree(children);
                        return true;
                    }
                    XFree(propPID);
                }
            }
        }
        XFree(children);
    }
    return false;
}

void activateWindow(Display* display, Window window) {
    XEvent event = {};
    event.xclient.type = ClientMessage;
    event.xclient.window = window;
    event.xclient.message_type = XInternAtom(display, "_NET_ACTIVE_WINDOW", False);
    event.xclient.format = 32;
    event.xclient.data.l[0] = 2; // Source indication: user action
    event.xclient.data.l[1] = CurrentTime; // Valid timestamp
    event.xclient.data.l[2] = 0;
    event.xclient.data.l[3] = 0;
    event.xclient.data.l[4] = 0;

    // Raise the window first
    XRaiseWindow(display, window);

    // Send the activation event
    XSendEvent(display, DefaultRootWindow(display), False,
               SubstructureRedirectMask | SubstructureNotifyMask, &event);
    XFlush(display);
}


#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <iostream>

bool isMainWindow(Display* display, Window window, unsigned long pid) {
    Atom atomPID = XInternAtom(display, "_NET_WM_PID", True);
    Atom atomType = XInternAtom(display, "_NET_WM_WINDOW_TYPE", True);
    Atom atomNormal = XInternAtom(display, "_NET_WM_WINDOW_TYPE_NORMAL", True);

    // Check PID match
    Atom actualType;
    int actualFormat;
    unsigned long nItems, bytesAfter;
    unsigned char* propPID = nullptr;

    if (Success != XGetWindowProperty(display, window, atomPID, 0, 1, False,
                                      XA_CARDINAL, &actualType, &actualFormat,
                                      &nItems, &bytesAfter, &propPID)) {
        return false;
    }

    if (!propPID || *(unsigned long*)propPID != pid) {
        if (propPID) XFree(propPID);
        return false;
    }
    XFree(propPID);

    // Check window type
    unsigned char* propType = nullptr;
    if (Success != XGetWindowProperty(display, window, atomType, 0, 1, False,
                                      XA_ATOM, &actualType, &actualFormat,
                                      &nItems, &bytesAfter, &propType)) {
        return false;
    }

    bool isNormal = propType && *(Atom*)propType == atomNormal;
    if (propType) XFree(propType);
    if (!isNormal) return false;

    // Check if window is viewable
    XWindowAttributes attr;
    if (!XGetWindowAttributes(display, window, &attr)) return false;
    if (attr.map_state != IsViewable) return false;

    return true;
}

Window findMainWindow(Display* display, Window root, unsigned long pid) {
    Window parent;
    Window* children;
    unsigned int nchildren;
    Window best = 0;

    if (XQueryTree(display, root, &root, &parent, &children, &nchildren)) {
        for (unsigned int i = 0; i < nchildren; ++i) {
            if (isMainWindow(display, children[i], pid)) {
                best = children[i];
                break; // You could add heuristics here to pick the largest or top-most
            }
        }
        XFree(children);
    }
    return best;
}

int pidActivate(pid_t targetPid) {
Display* display = XOpenDisplay(nullptr);
Window root = DefaultRootWindow(display);
// pid_t targetPid = 14775;

Window mainWin = findMainWindow(display, root, targetPid);
if (mainWin) {
    std::cout << "Main window: " << mainWin << "\n";
    activateWindow(display, mainWin); // Your existing function
} else {
    std::cerr << "No main window found for PID " << targetPid << "\n";
}
XCloseDisplay(display);

    return 0;
}

////////////////////////////////////////////////////////
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <cstring>
#include <iostream>

bool matchWindowClass(Display* display, Window window, const std::string& name) {
    Atom atomClass = XInternAtom(display, "WM_CLASS", True);
    if (atomClass == None) return false;

    Atom actualType;
    int actualFormat;
    unsigned long nItems, bytesAfter;
    unsigned char* prop = nullptr;

    if (Success == XGetWindowProperty(display, window, atomClass, 0, (~0L), False,
                                      XA_STRING, &actualType, &actualFormat,
                                      &nItems, &bytesAfter, &prop)) {
        if (prop) {
            std::string classStr(reinterpret_cast<char*>(prop), nItems);
            XFree(prop);
            return classStr.find(name) != std::string::npos;
        }
    }
    return false;
}

Window findWindowByClass(Display* display, Window root, const std::string& name) {
    Window parent;
    Window* children;
    unsigned int nchildren;
    Window result = 0;

    if (XQueryTree(display, root, &root, &parent, &children, &nchildren)) {
        for (unsigned int i = 0; i < nchildren; ++i) {
            if (matchWindowClass(display, children[i], name)) {
                result = children[i];
                break;
            }
        }
        XFree(children);
    }
    return result;
}




#include <iostream>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

bool windowExists(const std::string& windowName) {
    std::array<char, 512> buffer;
    std::string result;

    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("wmctrl -l", "r"), pclose);
    if (!pipe) return false;

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    return result.find(windowName) != std::string::npos;
}

void activateWindow(const std::string& windowName) {
    std::string command = "wmctrl -a \"" + windowName + "\"";
    system(command.c_str());
}


pid_t pid_=0;
int main() { 
    int bar_height = 24; // Adjust height as needed
    int screen_w = Fl::w();
    int screen_h = Fl::h();

    win = new Fl_Window(0, screen_h - bar_height, screen_w, bar_height);
    win->color(FL_DARK_BLUE);
    win->begin();
        // Add your widgets/controls here
        // Fl_Box* box = new Fl_Box(10, 5, 200, bar_height-10, "My Dock Bar");
        // // box->color(FL_TRANSPARENT);
        // box->labelsize(16);
        // box->labelcolor(FL_WHITE);
        // box->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
		
        Fl_Button* btn = new Fl_Button(0, 0, 50, bar_height);
		btn->callback([](Fl_Widget*){
            // This is the text you want to "paste"
			std::string text_to_paste = "This text was pasted from my custom taskbar!";
			string launcherstr="scite";
			// string launcherstr="code";
			// string launcherstr="google-chrome-stable";
			// if(!is_process_running(launcherstr))
				pid_= launch_app(launcherstr,{});
				cout<<"pid "<<pid_<<"\n";

		});
        Fl_Button* btn2 = new Fl_Button(50, 0, 50, bar_height);
		btn2->callback([](Fl_Widget*){

				    std::string windowName = "scite";               // Window title
    std::string launchCommand = "myapp &";          // Command to launch

    if (windowExists(windowName)) {
        activateWindow(windowName);
	}
		// pidActivate(pid_) ;
// 		Display* display = XOpenDisplay(nullptr);
// Window root = DefaultRootWindow(display);

// Window win = findWindowByClass(display, root, "scite"); // or "Firefox"
// if (win) {
//     std::cout << "Found window: " << win << "\n";
//     activateWindow(display, win); // Your existing function
// } else {
//     std::cerr << "No window found for class 'firefox'\n";
// }
// XCloseDisplay(display);

		});



    win->end();
    
    win->border(0); // Remove window borders
    win->show();
    
    // Set X11 properties after window is shown
    set_dock_properties(win);

	thread([](){
		listenkey();
 	}).detach();
    
    return Fl::run();
}