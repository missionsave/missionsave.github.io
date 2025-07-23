//  g++ taskbar.cpp ../common/general.cpp -I../common -o taskbar -l:libfltk.a -lX11 -lXtst -Os -s -flto=auto -std=c++20 -lXext -lXft -lXrender -lXcursor -lXinerama -lXfixes -lfontconfig -lfreetype -lz -lm -ldl -lpthread -lstdc++ -w 
//  g++ taskbar.cpp ../common/general.cpp -I../common -o taskbar -lfltk -lX11 -lXtst -std=c++20 -lXext -lXft -lXrender -lXcursor -lXinerama -lXfixes -lfontconfig -lfreetype -lz -lm -ldl -lpthread -lstdc++ -w -Wfatal-errors
// g++ taskbar.cpp ../common/general.cpp -I../common -o taskbar -lfltk -lX11 -lXtst -std=c++20 -lXext -lXft -lXrender -lXcursor -lXinerama -lXfixes -lfontconfig -lfreetype -lz -lm -ldl -lpthread -lstdc++ -w -Wfatal-errors


#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Browser.H>
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

#include <unistd.h>
#include <sys/wait.h>
#include <system_error>
#include <string_view>
#include <vector>
#include <iostream>
#include <string>
#include <regex>
#include <unordered_map>
#include <vector>
#include <string>
#include <cstdio>
#include <memory>
#include <sstream>
#include <cstdio>
#include <memory>
#include <string>
#include <array>
#include <cstdlib>
#include <unordered_map>
#include <unordered_map>
#include <string>
#include <iostream>
#include <sstream>
#include <cstdio>
#include <memory>
#include <regex>
#include <fstream>
#include <cctype>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <wordexp.h>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <vector>
#include <string>
#include <pwd.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xfixes.h>
#include <iostream>
#include <cstring>
#include <string>

#include "general.hpp"

void trim(std::string& str) {
    // Left trim
    str.erase(str.begin(), std::find_if(str.begin(), str.end(),
        [](unsigned char ch) { return !std::isspace(ch); }));

    // Right trim
    str.erase(std::find_if(str.rbegin(), str.rend(),
        [](unsigned char ch) { return !std::isspace(ch); }).base(), str.end());
}


#include <FL/fl_draw.H>

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Browser.H>
#include <FL/fl_draw.H>
#include <vector>
#include <sstream>

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Browser.H>
#include <FL/fl_draw.H>
#include <vector>
#include <cstring>
#include <FL/Fl_Output.H>
#include <fstream>
#include <string>
#include <ctime>

#include <unistd.h>

std::string read_file(const std::string& path) {
    std::ifstream file(path);
    std::string value;
    if (file && std::getline(file, value)) return value;
    return "";
}

std::string get_time() {
    time_t now = time(nullptr);
    struct tm* local = localtime(&now);
    char buffer[32];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", local);
    return buffer;
}

void update_display(Fl_Output* output) {
    std::string battery = read_file("/sys/class/power_supply/BAT0/capacity"); 
	// std::string status  = read_file("/sys/class/power_supply/BAT0/status");

    std::string clock   = get_time();

    output->value((""+battery + "% " + clock).c_str());
    Fl::repeat_timeout(30.0, (Fl_Timeout_Handler)update_display, output);
}


Fl_Window* tasbwin;
Fl_Group* fg;
void dbg();
void refresh();
using namespace std;

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

// Add these global atoms at the start of your program
Atom _NET_SUPPORTED;
Atom _NET_CLIENT_LIST;
Atom _NET_WM_STATE;
Atom _NET_WM_STATE_STICKY;
Atom _NET_WM_STATE_ABOVE;
Atom _NET_WM_WINDOW_TYPE;
Atom _NET_WM_WINDOW_TYPE_DOCK;

void init_global_atoms(Display* dpy= fl_display) {
    _NET_SUPPORTED = XInternAtom(dpy, "_NET_SUPPORTED", False);
    _NET_CLIENT_LIST = XInternAtom(dpy, "_NET_CLIENT_LIST", False);
    _NET_WM_STATE = XInternAtom(dpy, "_NET_WM_STATE", False);
    _NET_WM_STATE_STICKY = XInternAtom(dpy, "_NET_WM_STATE_STICKY", False);
    _NET_WM_STATE_ABOVE = XInternAtom(dpy, "_NET_WM_STATE_ABOVE", False);
    _NET_WM_WINDOW_TYPE = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
    _NET_WM_WINDOW_TYPE_DOCK = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DOCK", False);
}

// Call this when initializing your taskbar
void setup_taskbar_properties() {
// void setup_taskbar_properties(Display* dpy, Window win) {
	    Display* dpy = fl_display; // FLTK's global X11 display
    Window win = fl_xid(tasbwin); 
    // 1. Set as dock window
    XChangeProperty(dpy, win, _NET_WM_WINDOW_TYPE, XA_ATOM, 32,
                   PropModeReplace, (unsigned char*)&_NET_WM_WINDOW_TYPE_DOCK, 1);
    
    // 2. Make it sticky and always on top
    Atom states[2] = {_NET_WM_STATE_STICKY, _NET_WM_STATE_ABOVE};
    XChangeProperty(dpy, win, _NET_WM_STATE, XA_ATOM, 32,
                   PropModeReplace, (unsigned char*)states, 2);
    
    // 3. Set strut (space reservation)
    unsigned long strut[12] = {0};
    strut[3] = 24; // Bottom strut
    XChangeProperty(dpy, win, XInternAtom(dpy, "_NET_WM_STRUT_PARTIAL", False),
                   XA_CARDINAL, 32, PropModeReplace, (unsigned char*)strut, 12);
}

vstring vclipboard;

std::string getClipboardText(Display* display, Window window, Atom property) {
    Atom clipboard = XInternAtom(display, "CLIPBOARD", False);
    Atom utf8 = XInternAtom(display, "UTF8_STRING", False);
    
    // Clear previous property value
    XDeleteProperty(display, window, property);
    
    // Request clipboard content
    XConvertSelection(display, clipboard, utf8, property, window, CurrentTime);
    XFlush(display);

    XEvent event;
    while (true) {
        XNextEvent(display, &event);
        
        if (event.type == SelectionNotify && event.xselection.selection == clipboard) {
            if (event.xselection.property == None) {
                return ""; // Clipboard owner didn't respond
            }
            
            Atom actualType;
            int actualFormat;
            unsigned long nItems, bytesAfter;
            unsigned char* data = nullptr;

            XGetWindowProperty(display, window, property, 0, (~0L), False,
                               AnyPropertyType, &actualType, &actualFormat,
                               &nItems, &bytesAfter, &data);
            
            if (!data) return "";
            
            std::string result(reinterpret_cast<char*>(data), nItems);
            XFree(data);
            XDeleteProperty(display, window, property);
            return result;
        }
    }
}

int listenclipboard() {
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        std::cerr << "Cannot open X display\n";
        return 1;
    }

    // Check for XFixes extension
    int event_base, error_base;
    if (!XFixesQueryExtension(display, &event_base, &error_base)) {
        std::cerr << "XFixes extension not available\n";
        return 1;
    }

    // Create a hidden window
    Window window = XCreateSimpleWindow(display, DefaultRootWindow(display),
                                        0, 0, 1, 1, 0, 0, 0);
    Atom property = XInternAtom(display, "CLIPBOARD_PROPERTY", False);
    std::string lastClipboardContent;
    
    // Select clipboard change notifications
    Atom clipboard = XInternAtom(display, "CLIPBOARD", False);
    XFixesSelectSelectionInput(display, window, clipboard, 
                               XFixesSetSelectionOwnerNotifyMask);
    
    // Main event loop
    while (true) {
        XEvent event;
        XNextEvent(display, &event);
        
        // Check if it's a clipboard change event
        if (event.type == event_base + XFixesSelectionNotify) {
            XFixesSelectionNotifyEvent *sev = (XFixesSelectionNotifyEvent *)&event;
            if (sev->subtype == XFixesSetSelectionOwnerNotify) {
                std::string content = getClipboardText(display, window, property);
                if (!content.empty() && content != lastClipboardContent) {
                    // std::cout << "Clipboard contents: " << content << "\n";
					vclipboard.insert(vclipboard.begin(), content);
					// vclipboard.push_back(content);
                    lastClipboardContent = content;
                }
            }
        }
    }

    XDestroyWindow(display, window);
    XCloseDisplay(display);
    return 0;
}






// namespace Math {

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <unistd.h>

#include <X11/extensions/XTest.h>
#include <X11/keysym.h>

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
// }
// using namespace Math;
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

Fl_Browser* browserpaste;

std::atomic<bool> trigger_winpop = false;
class PopupWindow : public Fl_Window {
public:
    PopupWindow(int x,int y,int W, int H, const char* title)
        : Fl_Window(x,y,W, H, title) { 
			Fl::grab(*this);
			// set_override();
		}
	
	// int handle(int event) override {
	// 	cotm(event);
	// 	if(event==4){
	// 		browserpaste->hide();
	// 		trigger_winpop = false;
	// 		this->hide();
	// 		delete browserpaste;
	// 		// delete this;
	// 		return 1;
	// 	}		
    // 	return Fl_Window::handle(event);
	// }
	~PopupWindow() {
		cotm("release")
        Fl::grab(nullptr); // Release grab when window is destroyed
    }

    int handle(int event) override {
        if (event == FL_PUSH) {
            // Check if the click is outside this window
            if (!Fl::event_inside(browserpaste)) {
				cotm("inside");
                Fl::grab(nullptr);
				// browserpaste->hide();
				trigger_winpop = false;
				this->hide();
				delete browserpaste;
				// delete this; 
                return 1; // Event handled
            }
			// return 0;
        }
        return Fl_Window::handle(event);
    }
};
// struct PopupWindow;
// PopupWindow* current_popup = nullptr;
// int global_click_handler(int event);


// class PopupWindow : public Fl_Window {
// public:
//     PopupWindow(int x, int y, int W, int H, const char* title)
//         : Fl_Window(x, y, W, H, title) 
//     {
//         current_popup = this;
//         static bool hook_installed = false;
//         if (!hook_installed) {
//             Fl::add_event_handler(global_click_handler);
//             hook_installed = true;
//         }
//     }

//     ~PopupWindow() {
//         if (current_popup == this) {
//             current_popup = nullptr;
//         }
//     }
// };
// int global_click_handler(int event) {
//     if (event == FL_PUSH && current_popup && current_popup->visible()) {
//         if (!Fl::event_inside(current_popup)) {
//             current_popup->hide();
//         }
//     }
//     return 0;
// }


PopupWindow* winpaste; 
// Fl_Window* winpaste; 
void browser_callback(Fl_Widget* , void* ) { 
    cotm("br");
    int index = browserpaste->value()-1;  // Browser indices start at 1
    if (index >= 0 && index < vclipboard.size()) {
		// string copy_to_clipboard="testing";
		string copy_to_clipboard=vclipboard[index];
		
		const char* text=copy_to_clipboard.c_str();
		Fl::copy(text, strlen(text), 1);
		paste();

        // fl_message(vclipboard[index].c_str());
		browserpaste->hide();
		winpaste->hide();
		Fl::grab(nullptr);
		trigger_winpop = false;
		delete browserpaste;
		delete winpaste;
    }
}

void delayed_action(void* ) {
		cotm("after winpop close")
        trigger_winpop = false;
    std::cout << "Subwindow was hidden. Now running delayed action!\n";
    // You can cast and use 'data' if needed
}
int winpoptest() {
	Fl_Group::current(nullptr); // important
	Fl_Group* g = Fl_Group::current();
if ( g ) {
    // g is the widget that new children would be added to
    std::cout << "Current group is " 
              << (g->label() ? g->label() : "(no label)") 
              << "\n";
}
else {
    std::cout << "There's no current group\n";
}
	Fl::lock();
    Fl_Window* winpaste = new Fl_Window(400, 100, 300, 200, "Popup");
	// winpaste->begin();
    Fl_Button* btn = new Fl_Button(100, 80, 100, 40, "Click Me");

    // Button callback
    btn->callback([](Fl_Widget*, void* data) {
        Fl_Window* winpaste = (Fl_Window*)data;
        std::cout << "Button clicked\n";

        std::string copy_to_clipboard = "testing";
        Fl::copy(copy_to_clipboard.c_str(), copy_to_clipboard.length(), 1);

        paste();
        winpaste->hide();  // This will trigger hide callback
		delete winpaste;
		// trigger_winpop = false;
		Fl::add_timeout(0.5, delayed_action, 0);

    }, (void*)winpaste);

    // Hide callback to delete window
    // winpaste->callback([](Fl_Widget* w, void*) {
    //     Fl_Window* win = (Fl_Window*)w;
    //     std::cout << "Window hidden, destroying.\n";
    //     win->hide();
    //     delete win;  // Destroy window completely
    // });

    // winpaste->end();
    // Fl::check();
    winpaste->show();
    Fl::flush();

if(0){
    // 3. force-raise via X11
    Display* dpy = fl_display;
    Window   xid = fl_xid(winpaste);
    XMapWindow(dpy, xid);
    XRaiseWindow(dpy, xid);
    XSetInputFocus(dpy, xid, RevertToParent, CurrentTime);
    XFlush(dpy);
	return 1;
}


    // Prevent window from stealing focus
    Display* dpy = fl_display;
    Window xid = fl_xid(winpaste);

    XSetWindowAttributes attrs;
    attrs.override_redirect = True;
    XChangeWindowAttributes(dpy, xid, CWOverrideRedirect, &attrs);
    XMapRaised(dpy, xid);

    // Fl::run();

	cotm("winpasteshow",winpaste->visible());
	Fl::unlock();
    return 1;
}



int winpop() {
	Fl_Group::current(nullptr); // important
    // winpaste = new Fl_Window(0, Fl::h()-300, 400, 300, "Popup");
    winpaste = new PopupWindow(0, Fl::h()-300, 400, 300, "Popup");
    // Fl_Button* btn = new Fl_Button(100, 80, 100, 40, "Click Me");
	
    // btn->callback([](Fl_Widget*, void*) {
    //     std::cout << "Button clicked\n";

	// 	string copy_to_clipboard="testing";
		
	// 	const char* text=copy_to_clipboard.c_str();
	// 	Fl::copy(text, strlen(text), 1);
	// 	paste();
	// 	winpaste->hide();
    // });

	// browserpaste = new SmartBrowser(0, 0, 400, 300);
	browserpaste = new Fl_Browser(0, 0, 400, 300);
	// browser->hscrollbar(0);
	browserpaste->textsize(16); 
	browserpaste->textfont(FL_COURIER);
	browserpaste->type(FL_HOLD_BROWSER );
	browserpaste->has_scrollbar(Fl_Browser_::VERTICAL);
	browserpaste->callback(browser_callback);

	// std::string long_text = "This is a long paragraph that won't fit in two lines completely.";
	// full_texts.push_back(long_text);
	lop(i,0,vclipboard.size()){
		std::string clipped = "@b@u"+ vclipboard[i];  
		// std::string clipped = "@b"+format_to_two_lines(vclipboard[i], 40);  // Assuming ~40 characters per line
		    // Remove newlines
    clipped.erase(std::remove(clipped.begin(), clipped.end(), '\n'), clipped.end());

    // Limit to 120 characters
    if (clipped.size() > 120) {
        clipped = clipped.substr(0, 120);
    }
		browserpaste->add(clipped.c_str());
	}


    winpaste->end();
    winpaste->show();

    // Prevent window from stealing focus
    Display* dpy = fl_display;
    Window xid = fl_xid(winpaste);

    XSetWindowAttributes attrs;
    attrs.override_redirect = True;
    XChangeWindowAttributes(dpy, xid, CWOverrideRedirect, &attrs);
    XMapRaised(dpy, xid); // Show without focus

    return 1;
}


#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <iostream>


void check_trigger(void*) {
	cotm("tcalled");
    // if (trigger_winpop) {
        winpop();
		// // Fl::wait();
		// cotm("after winpop close")
        // trigger_winpop = false;
    // }
	// Fl::remove_idle(check_trigger);
}

int listenkey() {
    Display* dpy = XOpenDisplay(nullptr);
    if (!dpy) {
        std::cerr << "Cannot open display\n";
        return 1;
    }

    Window root = DefaultRootWindow(dpy);
    KeyCode v_key = XKeysymToKeycode(dpy, XStringToKeysym("v"));

    // Grab Super_L (Win) + V globally using Mod4Mask as modifier
    XGrabKey(dpy, v_key, Mod4Mask, root, True, GrabModeAsync, GrabModeAsync);
    XSelectInput(dpy, root, KeyPressMask);

    while (true) {
        XEvent ev;
        XNextEvent(dpy, &ev);

        if (ev.type == KeyPress &&
            ev.xkey.keycode == v_key &&
            (ev.xkey.state & Mod4Mask)) {

            trigger_winpop = true;
            Fl::awake(check_trigger);
            cotm("wpress");
			//  Fl::awake(); 
			//  winpop();                      
    		// Fl::add_idle(check_trigger, nullptr);
        }
    }

    XCloseDisplay(dpy);
    return 0;
}








#pragma endregion

struct WindowInfo {
    std::string window_id;
    std::string title;
    int pid;
    std::string process_name;
	bool is_open=0;
};

std::string getProcessName(int pid) {
    std::ifstream file("/proc/" + std::to_string(pid) + "/comm");
    std::string name;
    if (file.is_open()) {
        std::getline(file, name);
    }
    return name;
}

std::vector<WindowInfo> getOpenWindowsInfo() {
    std::vector<WindowInfo> windows;
    const char* command = "wmctrl -lp";

    std::array<char, 512> buffer;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command, "r"), pclose);
    if (!pipe) return windows;

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        std::string line(buffer.data());

        std::istringstream iss(line);
        WindowInfo info;

        iss >> info.window_id; // 0xID
        std::string desktop_num;		
        iss >> desktop_num;    // Desktop number (ignored)
		if (desktop_num == "-1") {
			continue; // Skip sticky windows (appear on all desktops)
		}
        iss >> info.pid;       // PID
        std::string hostname;
        iss >> hostname;       // Hostname (ignored)

        std::string title;
        std::getline(iss, title);
        title.erase(0, title.find_first_not_of(" \t\n\r")); // Trim leading spaces
        info.title = title;
		if(info.title=="")continue;

        info.process_name = getProcessName(info.pid);

        windows.push_back(info);
    }

    return windows;
}

std::vector<std::string> getProcessArgs(int pid) {
    std::vector<std::string> args;
    std::ifstream file("/proc/" + std::to_string(pid) + "/cmdline", std::ios::binary);

    if (!file.is_open()) {
        return args; // Process may have exited or permissions issue
    }

    std::string buffer((std::istreambuf_iterator<char>(file)), {});
    size_t start = 0;

    for (size_t i = 0; i < buffer.size(); ++i) {
        if (buffer[i] == '\0') {
            args.emplace_back(buffer.data() + start, i - start);
            start = i + 1;
        }
    }

    return args;
}
string getExecByPid(int pid){
	std::vector<std::string> args=getProcessArgs(pid);
	stringstream strm;
	for (const auto& arg : args) {
		strm << arg << ' ';
	}
	string sexec=strm.str();
	trim(sexec);
	return sexec;
}


#include <optional>

std::optional<std::string> findKeyByValue(
    const std::unordered_map<std::string, std::string>& uapps,
    const std::string& targetValue
) {
    for (const auto& [key, value] : uapps) {
        if (value == targetValue) {
            return key; // Found
        }
    }
    return std::nullopt; // Not found
}
std::vector<std::string> findKeysByValue(
    const std::unordered_map<std::string, std::string>& uapps,
    const std::string& targetValue
) {
	vector<string> res;
    for (const auto& [key, value] : uapps) {
        if (value == targetValue) {
			res.push_back(key);
            // return key; // Found
        }
    }
    return res;
}

void refresh_idle_cb(void*) { 
// return;
	if(trigger_winpop){
	cotm("NoRefresh")
		return;
	}
	cotm("Refresh")
	fg->begin(); refresh(); fg->end(); 
    // Fl::remove_idle(refresh_idle_cb); // if you only want it once
}
#include <unordered_set>
#include <chrono>
#include <thread>

std::unordered_set<Window> processed_windows;
std::unordered_set<Window> ignore_windows;




pid_t get_pid_from_window(Display* display, Window window) {
    // Try _NET_WM_PID first
    Atom net_wm_pid = XInternAtom(display, "_NET_WM_PID", False);
    Atom type;
    int format;
    unsigned long nitems, after;
    unsigned char* data = nullptr;
    
    if (XGetWindowProperty(display, window, net_wm_pid,
                          0, 1, False, XA_CARDINAL,
                          &type, &format, &nitems, &after, &data) == Success && data) {
        pid_t pid = *((pid_t*)data);
        XFree(data);
        return pid;
    }
    
    // Fallback: Try WM_CLIENT_LEADER -> _NET_WM_PID
    Atom wm_client_leader = XInternAtom(display, "WM_CLIENT_LEADER", False);
    if (XGetWindowProperty(display, window, wm_client_leader,
                          0, 1, False, XA_WINDOW,
                          &type, &format, &nitems, &after, &data) == Success && data) {
        Window leader = *((Window*)data);
        XFree(data);
        return get_pid_from_window(display, leader); // Recursively check leader
    }
    
    return 0; // PID not found
}
// New function to ensure wmctrl visibility

void set_wm_visibility_properties(Display* display, Window win) {
    // Skip if this is our taskbar window
    if (win == fl_xid(tasbwin)) return;

    // Ensure basic WM properties only for application windows
    XSetWindowAttributes attrs;
    attrs.override_redirect = False;
    XChangeWindowAttributes(display, win, CWOverrideRedirect, &attrs);
    
    // Set WM_CLASS
    XClassHint class_hint;
    class_hint.res_name = (char*)"xclient";
    class_hint.res_class = (char*)"XClient";
    XSetClassHint(display, win, &class_hint);
    
    // Set _NET_WM_PID if missing
    Atom net_wm_pid = XInternAtom(display, "_NET_WM_PID", False);
    pid_t pid = 0;
    if (XGetWindowProperty(display, win, net_wm_pid, 0, 1, False,
                          XA_CARDINAL, nullptr, nullptr, nullptr, nullptr, nullptr) != Success) {
        pid = get_pid_from_window(display, win);
        if (pid > 0) {
            XChangeProperty(display, win, net_wm_pid, XA_CARDINAL, 32,
                          PropModeReplace, (unsigned char*)&pid, 1);
        }
    }
    
    // Set window name if empty
    char* win_name = nullptr;
    if (XFetchName(display, win, &win_name) == 0 || !win_name) {
        XStoreName(display, win, "Application Window");
    }
    if (win_name) XFree(win_name);
    
    // Force WM state update
    XMapWindow(display, win);
    XSync(display, False);
}

void init_wm_properties(Display* dpy) {
    Window root = DefaultRootWindow(dpy);
    
    // Set supported EWMH properties
    Atom net_supported = XInternAtom(dpy, "_NET_SUPPORTED", False);
    Atom net_atoms[] = {
        XInternAtom(dpy, "_NET_CLIENT_LIST", False),
        XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False),
        XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_NORMAL", False),
        XInternAtom(dpy, "_NET_WM_STATE", False),
        XInternAtom(dpy, "_NET_WM_PID", False),
        XInternAtom(dpy, "_NET_WM_NAME", False)
    };
    
    XChangeProperty(dpy, root, net_supported, XA_ATOM, 32, PropModeReplace,
                  (unsigned char*)net_atoms, sizeof(net_atoms)/sizeof(Atom));

    // Initialize empty client list
    Atom net_client_list = XInternAtom(dpy, "_NET_CLIENT_LIST", False);
    XChangeProperty(dpy, root, net_client_list, XA_WINDOW, 32, PropModeReplace,
                  (unsigned char*)nullptr, 0);
}

void enforce_window_geometry(Display* display, Window win) {
    // Skip if this is our taskbar window or ignored window
    if (win == fl_xid(tasbwin) || ignore_windows.count(win)) return;
    
    // Get screen dimensions
    int screen_w = Fl::w();
    int screen_h = Fl::h();
    int available_h = screen_h - tasbwin->h();
    
    // Standard X11 resize
    XMoveResizeWindow(display, win, 0, 0, screen_w, available_h);
    
    // Set size hints
    XSizeHints hints;
    hints.flags = PPosition | PSize | PMinSize | PMaxSize;
    hints.x = 0;
    hints.y = 0;
    hints.width = screen_w;
    hints.height = available_h;
    hints.min_width = screen_w;
    hints.min_height = available_h;
    hints.max_width = screen_w;
    hints.max_height = available_h;
    XSetWMNormalHints(display, win, &hints);
    
    // Mark as processed
    processed_windows.insert(win);
    
    // Schedule a check after 500ms
    std::thread([display, win, screen_w, available_h]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        XWindowAttributes attrs;
        if (XGetWindowAttributes(display, win, &attrs)) {
            if (attrs.width != screen_w || attrs.height != available_h) {
                // Try again more forcefully
                XResizeWindow(display, win, screen_w, available_h);
                XMoveWindow(display, win, 0, 0);
                
                // If still not cooperating after 3 tries, give up
                static std::unordered_map<Window, int> retry_counts;
                if (++retry_counts[win] > 3) {
                    ignore_windows.insert(win);
                }
            }
        }
    }).detach();
}
int eventWindow() {
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        std::cerr << "Cannot open X display\n";
        return 1;
    }
	init_wm_properties(display);
    Window root = DefaultRootWindow(display);
    XSelectInput(display, root, SubstructureNotifyMask);

    while (true) {
        XEvent ev;
        XNextEvent(display, &ev);
        
        switch (ev.type) {
            case CreateNotify:
                std::cout << "Window created: " << ev.xcreatewindow.window << '\n';
                Fl::awake(refresh_idle_cb);
                break;
                
			case MapNotify: {
    Window mapped_win = ev.xmap.window;
    
    // Skip if we've already processed this window
    if (processed_windows.count(mapped_win)) break;
    
    // Declare attrs here before using it
    XWindowAttributes attrs;
    if (XGetWindowAttributes(display, mapped_win, &attrs)) {
        if (!attrs.override_redirect) {
            // Wait briefly before acting to let window initialize
            std::thread([display, mapped_win, tasbwin]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                
                // First enforce window geometry (your existing functionality)
                enforce_window_geometry(display, mapped_win);
                
                // Then set WM properties for wmctrl visibility
                set_wm_visibility_properties(display, mapped_win); //removing this the taskbar appear normal
                
                // Mark as processed
                processed_windows.insert(mapped_win);
            }).detach();
        }
    }
    break;
}
            
            case DestroyNotify:
                std::cout << "Window destroyed: " << ev.xdestroywindow.window << '\n';
                Fl::awake(refresh_idle_cb);
                break;
        }
    }

    XCloseDisplay(display);
    return 0;
}

struct Launcher{
	// Check if a window with the given title substring exists
	bool windowExists(const std::string& title) {
		std::array<char, 256> buffer;
		std::string output;
		std::unique_ptr<FILE, decltype(&pclose)> pipe(
			popen("wmctrl -l", "r"), pclose);
		if (!pipe) return false;
		while (fgets(buffer.data(), buffer.size(), pipe.get())) {
			output += buffer.data();
		}
		return output.find(title) != std::string::npos;
	}

	// Activate the window whose title matches
	void activateWindow(const std::string& title) {
		std::string cmd = "wmctrl -a \"" + title + "\"";
		system(cmd.c_str());
	}
	
	void activateWindowById(const std::string& winId) {
		std::string cmd = "wmctrl -i -a " + winId;
		system(cmd.c_str());
	}

	// Removes placeholders like %F, %U, %f, etc.
	std::string parseExecCommand(const std::string& execLine) {
		// return execLine;
		std::string cleaned = std::regex_replace(execLine, std::regex("%[fFuUdDnNickvm]"), "");
		return cleaned;
	}


void launch(const std::string& execCommand, bool dropPrivileges=true) {
    pid_t pid = fork();

    if (pid < 0) {
        std::cerr << "Failed to fork\n";
        return;
    }

    if (pid > 0) {
        return;  // Parent returns
    }

    // Optional: setsid() if you really want (but usually unnecessary for GUI apps)
    // setsid();

    if (dropPrivileges) {
        uid_t ruid = getuid();
        gid_t rgid = getgid();
        setgid(rgid);
        setuid(ruid);
    }

    // Optional: Silence stdio
    // fclose(stdin);
    // fclose(stdout);
    // fclose(stderr);
    // stdin  = fopen("/dev/null", "r");
    // stdout = fopen("/dev/null", "w");
    // stderr = fopen("/dev/null", "w");

    const char* display = getenv("DISPLAY");
	cotm(display)
    if (!display) display = ":0";

    std::string fullCommand = "DISPLAY=" + std::string(display) + " " + execCommand;
cotm(fullCommand)
    std::vector<const char*> argv = { "/bin/sh", "-c", fullCommand.c_str(), nullptr };

    execv("/bin/sh", const_cast<char* const*>(argv.data()));

    _exit(1);
}


	unordered_map <string,string> uapps;

	void fillApplications(std::unordered_map<std::string, std::string>& uapps) {
		const char* command =
			"grep -rl 'Type=Application' /usr/share/applications ~/.local/share/applications 2>/dev/null | "
			"xargs grep -L 'NoDisplay=true' 2>/dev/null | "
			"xargs grep -L 'Hidden=true' 2>/dev/null | "
			"xargs grep -hE '^Name=|^Exec=' 2>/dev/null | "
			"paste - -";

		std::array<char, 512> buffer;
		std::string output;
		std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command, "r"), pclose);
		if (!pipe) return;

		while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
			output += buffer.data();
		}

		std::istringstream stream(output);
		std::string line;

		// std::regex placeholder_re("%[fFuUdDnNickvm]");

		while (std::getline(stream, line)) {
			size_t name_pos = line.find("Name=");
			size_t exec_pos = line.find("Exec=");
			if (name_pos == std::string::npos || exec_pos == std::string::npos)
				continue;

			std::string name = line.substr(name_pos + 5, exec_pos - name_pos - 6);
			std::string exec = line.substr(exec_pos + 5);

			// Remove placeholders like %F
			// exec = std::regex_replace(exec, placeholder_re, "");
			string res=parseExecCommand(exec);
			trim(res);
			uapps[name] = res; 

		}
	}

	int getPidFromWindowTitle(const std::string& titleSubstring) {
		const char* command = "wmctrl -lp";
		std::array<char, 512> buffer;
		std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command, "r"), pclose);

		if (!pipe) return -1;

		while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
			std::string line(buffer.data());

			std::istringstream iss(line);
			std::string window_id;
			std::string desktop_num;
			int pid;
			std::string hostname;
			std::string title;

			iss >> window_id;
			iss >> desktop_num;

			if (desktop_num == "-1") {
				continue; // Skip sticky windows
			}

			iss >> pid;
			iss >> hostname;
			std::getline(iss, title);

			// Clean leading spaces
			title.erase(0, title.find_first_not_of(" \t\n\r"));

			// Check if the titleSubstring is found in the window title
			if (title.find(titleSubstring) != std::string::npos) {
				return pid; // Found matching window
			}
		}

		return -1; // Not found
	}



	// FLTK button callback
	void on_click(Fl_Widget*, void*) {
		const std::string winTitle = "Panel";
		if (windowExists(winTitle)) {
			activateWindow(winTitle);
		} else {
			launch(uapps[winTitle]);
		}
	} 
};
#pragma region init
	Launcher* lnc;
	vector<string> pinnedApps={"Google Chrome","Visual Studio Code","tmux x86_64"};
	
#pragma endregion 

vector<Fl_Button*> vbtn;
vector<WindowInfo> vwin;

void refresh(){
	// return;
	int widthbtn=150;

	for (int i = 0; i < vbtn.size(); i++){
		vbtn[i]->hide();
		delete vbtn[i];
	}
	vbtn.clear();
	vwin.clear();

	//pinned
	vector<WindowInfo> windows = getOpenWindowsInfo();
	cotm(windows.size())
	for(int ip=0;ip<pinnedApps.size();ip++){
		vwin.push_back(WindowInfo());
		vwin.back().title=pinnedApps[ip];
		vwin.back().is_open = 0;
		// cout<<"is "<<vwin.back().is_open<<"\n";
		for (auto it = windows.begin(); it != windows.end(); /* no ++ here */) {
			// cotm(it->title,it->pid,it->window_id)
			// if()
			if(it->title==""){++it; continue;}
			string sexecp = getExecByPid(it->pid);
			string sexec = lnc->parseExecCommand(sexecp); 
			trim(sexec);
			// cotm(pinnedApps[ip] ,sexecp,sexec)
			string sexecf = findKeyByValue(lnc->uapps, sexec).value_or("");
			// string sexecf = findKeyByValue(lnc->uapps, sexecp).value_or("");

			if (pinnedApps[ip] == sexecf) {
				vwin.back().title = sexecf;
				// vwin.back().title = lnc->uapps[sexecf];
				vwin.back().window_id = it->window_id;
				vwin.back().is_open = 1;
// cout<<"equal "<<sexecf<<" "<<sexecp<<"\n";
				it = windows.erase(it);  // removes and advances the iterator
			} else {
				++it;  // only advance if not erasing
			}
		}

	} 
// cotm("parou ",windows.size(),windows[0].title)
	//remains opened
		for (auto it = windows.begin(); it != windows.end();it++){// /* no ++ here */) {
			cotm(it->title,it->pid,it->window_id)
			if(it->title=="")continue;
			cotm(it->title,it->pid,it->window_id)
			// if(it->title=="")continue;
			string sexecp = getExecByPid(it->pid);
			string sexec = lnc->parseExecCommand(sexecp); 
			trim(sexec);
			string sexecf = findKeyByValue(lnc->uapps, sexecp).value_or("");

			if (sexec== sexecf) {
				vwin.push_back(WindowInfo());
				vwin.back().title = sexecf;
				// vwin.back().title = lnc->uapps[sexecf];
				vwin.back().window_id = it->window_id;
				vwin.back().is_open = 1;
			} else {
				vwin.push_back(WindowInfo());
				vwin.back().title = it->title;
				vwin.back().window_id = it->window_id;
				vwin.back().is_open = 1;
			}
		}

	 
// cotm("parou2 ",windows.size())
//  cotm(vwin.size())

	vbtn=vector<Fl_Button *>(vwin.size());
	// vbtn.resize(vwin.size(), nullptr);
// return;
	tasbwin->begin();
	for(int i=0;i<vwin.size();i++){
		std::string escaped = std::regex_replace(vwin[i].title, std::regex("@"), "@@"); 
    	vbtn[i] = new Fl_Button(i*(widthbtn+4), 0, widthbtn, 24);
		vbtn[i]->copy_label(escaped.c_str());
		vbtn[i]->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
		vbtn[i]->callback([](Fl_Widget* widget,void* data){
			
			Fl_Button* btn=(Fl_Button*)widget;
			WindowInfo wi = *((WindowInfo*)data);

// cotm(wi.is_open )
			if(wi.is_open){
				lnc->activateWindowById(wi.window_id);
			}
			if(!wi.is_open){
// cotm(wi.title)
				lnc->launch(lnc->uapps[wi.title]);
				// fg->begin(); refresh(); fg->end(); 
				tasbwin->redraw();
				btn->redraw();
				btn->show();
				// lnc->launch(lnc->uapps[wi.title]);
				// lnc->activateWindow(wi.title);
			}
			// const std::string winTitle = std::string(((Fl_Button*)widget)->label());
			// if (lnc->windowExists(winTitle)) {
			// 	lnc->activateWindow(winTitle);
			// 	btn->redraw();
			// 	btn->show();
				return;
			// }
		},&vwin[i]);
	}
	tasbwin->end();
}
 
void dbg(){
	cout<<"uapps "<<lnc->uapps.size()<<endl;
	cout<<endl;

	for(int i=0;i<pinnedApps.size();i++){
		cout<<pinnedApps[i]<<" => []"<<lnc->getPidFromWindowTitle(pinnedApps[i])<<endl;
	} 
	cout<<endl;
	auto windows = getOpenWindowsInfo();
    for (const auto& win : windows) {
        std::cout << win.process_name << " [" << win.pid << "] "
                  << win.window_id << " => "
                  << win.title << '\n';

		std::cout << "Process args for PID " << win.pid << ":\n";
		string sexec=getExecByPid(win.pid);
		string sexecf=findKeyByValue(lnc->uapps,sexec).value_or("");
		cout<<"sexec"<<(sexec)<<"\n";
		cout<<"sexecf"<<(sexecf)<<"\n";
		cout<<endl;

    }
	cout<<"windows "<<windows.size()<<"\n";
	cout<<"dbg "<<lnc->uapps["tmux x86_64"]<<"\n";
}
#include <chrono>
int main() { 
	Fl::lock();
	XInitThreads();
	lnc=new Launcher;
	lnc->fillApplications(lnc->uapps);

    int bar_height = 24; // Adjust height as needed
    int screen_w = Fl::w();
    int screen_h = Fl::h();

    tasbwin = new Fl_Window(0, screen_h - bar_height, screen_w, bar_height);
    tasbwin->color(FL_DARK_RED);
    tasbwin->begin();
	
	// Fl_Button* btest=new Fl_Button(screen_w-100,0,100,24,"refresh");
	// btest->callback([](Fl_Widget*){
	// 	fg->begin();
	// 	refresh();
	// 	fg->end();
	// 	// win->redraw();
	// });

    Fl_Output* output = new Fl_Output(screen_w-180,0,180,24);
    output->textfont(FL_COURIER);
    output->textsize(14);
    output->readonly(1);




    fg=new Fl_Group(0,0,screen_w*.7,bar_height);
	fg->begin();
	refresh();  
	fg->end();


    tasbwin->end();
    
    tasbwin->border(0); // Remove window borders
    tasbwin->show();

	update_display(output);
    
    // Set X11 properties after window is shown
    set_dock_properties(tasbwin);
	init_global_atoms();
	setup_taskbar_properties();

	thread([](){
		listenclipboard();
 	}).detach();

	thread([](){
		eventWindow();
 	}).detach();
	
	thread([](){
		listenkey();
 	}).detach();


    // dbg();
    return Fl::run();
}