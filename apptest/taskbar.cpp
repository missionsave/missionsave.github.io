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





class SmartBrowser : public Fl_Browser {
    std::vector<int> rowHeights;
    int separatorHeight = 1;
    Fl_Color separatorColor = fl_rgb_color(200, 200, 200);
    
    void calculateRowHeights() {
        rowHeights.clear();
        for (int i = 1; i <= size(); i++) {
            const char* itemText = (const char*)data(i);
            int lines = 1;
            if (itemText) {
                for (const char* c = itemText; *c; c++) {
                    if (*c == '\n') lines++;
                }
            }
            rowHeights.push_back((lines * (textsize() + 4)) + 6);
        }
    }
    
public:
    SmartBrowser(int x, int y, int w, int h, const char* label = 0)
        : Fl_Browser(x, y, w, h, label) {
        type(FL_MULTI_BROWSER);
        textsize(14);
        selection_color(fl_rgb_color(230, 245, 255));
    }
    
    void add(const char* text) {
        Fl_Browser::add(text);
        calculateRowHeights();
    }
    
    void insert(int pos, const char* text) {
        Fl_Browser::insert(pos, text);
        calculateRowHeights();
    }
    
    void remove(int pos) {
        Fl_Browser::remove(pos);
        calculateRowHeights();
    }
    
    void clear() {
        Fl_Browser::clear();
        rowHeights.clear();
    }
    
    void draw() {
        // Let FLTK draw the basic browser structure first
        Fl_Browser::draw();
        
        // Now add our custom decorations
        int Y = y() + 1; // Small offset to account for border
        int X = x() + 1;
        int first_line = topline();
        int last_line = std::min(size(), first_line + (h()-2) / (textsize() + 6) + 2);
        
        for (int i = first_line + 1; i <= last_line; i++) {
            if (i > size()) break;
            
            int itemHeight = rowHeights[i-1];
            
            // Draw separator (before the item except for first item)
            if (i > 1) {
                fl_color(separatorColor);
                fl_rectf(X, Y - separatorHeight, w()-2, separatorHeight);
            }
            
            // Draw text (let FLTK handle it)
            Y += itemHeight;
        }
    }
    
    int full_height() const {
        int total = 0;
        for (int h : rowHeights) {
            total += h;
        }
        return total;
    }
    
    int item_height(void* item) const {
        for (int i = 1; i <= size(); i++) {
            if (data(i) == item) {
                return rowHeights[i-1];
            }
        }
        return textsize() + 6;
    }
    
    void setSeparatorHeight(int height) {
        separatorHeight = height;
        redraw();
    }
    
    void setSeparatorColor(Fl_Color color) {
        separatorColor = color;
        redraw();
    }
};

// int main() {
//     Fl_Double_Window win(500, 400, "Multiline Browser");
//     SmartBrowser browser(20, 20, 460, 360);
    
//     // Set a nice font
//     Fl::set_font(FL_HELVETICA, "Arial");
    
//     // Add sample items
//     browser.add("Single line item");
//     browser.add("Two lines\nSecond line");
//     browser.add("Three lines\nMiddle line\nBottom line");
//     browser.add("Programming Languages:\nC++\nPython\nJavaScript\nRust");
//     browser.add("Final item\nWith two lines");
    
//     win.end();
//     win.show();
//     return Fl::run();
// }










Fl_Window* win;
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

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xfixes.h>
#include <iostream>
#include <cstring>
#include <string>

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






#pragma region 

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

std::string format_to_two_lines(const std::string& text, int line_width) {
    std::string clipped;
    int count = 0, lines = 0;
    for (char c : text) {
        clipped += c;
        count++;
        if (count >= line_width) {
            clipped += '\n';
            count = 0;
            lines++;
            if (lines == 2) break;
        }
    }
    if (lines == 1) clipped += "...";
    return clipped;
}



// SmartBrowser* browserpaste;
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

int eventWindow() {
        Display* display = XOpenDisplay(nullptr);
    if (!display) {
        std::cerr << "Cannot open X display\n";
        return 1;
    }

    Window root = DefaultRootWindow(display);

    // Listen without redirecting (non-exclusive)
    XSelectInput(display, root, SubstructureNotifyMask);

    std::cout << "Listening for window open/close events...\n";

    while (true) {
        XEvent ev;
        XNextEvent(display, &ev);
		// continue;
		// if(trigger_winpop){
		// 	cotm("trigger_winpop")
		// 	continue;
		// }
        switch (ev.type) {
			case CreateNotify:
                std::cout << "Window created: " << ev.xcreatewindow.window << '\n';
	// if(trigger_winpop){			    	// fg->begin(); refresh(); fg->end();
 Fl::awake(refresh_idle_cb);                       // wake up the main loop
    // Fl::add_idle(refresh_idle_cb, nullptr);
	// }
                break;
            case MapNotify:
                // std::cout << "Window opened (mapped): " << ev.xmap.window << '\n';
                break;
            case UnmapNotify:
                // std::cout << "Window closed (unmapped): " << ev.xunmap.window << '\n';
                break;
            case DestroyNotify:
                std::cout << "Window destroyed: " << ev.xdestroywindow.window << '\n';
				    	// fg->begin(); refresh(); fg->end();;
	// if(trigger_winpop){			    	// fg->begin(); refresh(); fg->end();
 Fl::awake(refresh_idle_cb);                       // wake up the main loop
    // Fl::add_idle(refresh_idle_cb, nullptr);
	// }
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

	// If need to handle quoted arguments, need a more robust parser
	void launch(const std::string& execCommand) {
		pid_t pid = fork();

		if (pid < 0) {
			std::cerr << "Failed to fork\n";
			return;
		}

		if (pid > 0) {
			return; // Parent returns
		}

		// Child process:
		setsid();  // Detach from terminal

		// Simple split by spaces
		std::istringstream iss(execCommand);
		std::vector<std::string> args;
		std::string token;

		while (iss >> token) {
			args.push_back(token);
		}

		// Build argv[]
		std::vector<char*> argv;
		for (auto& arg : args)
			argv.push_back(arg.data());
		argv.push_back(nullptr);

		freopen("/dev/null", "r", stdin);
		freopen("/dev/null", "w", stdout);
		freopen("/dev/null", "w", stderr);

		execvp(argv[0], argv.data());

		_exit(1); // exec failed
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
#pragma region 
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
	win->begin();
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
				win->redraw();
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
	win->end();
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

    win = new Fl_Window(0, screen_h - bar_height, screen_w, bar_height);
    win->color(FL_DARK_RED);
    win->begin();
	
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


    win->end();
    
    win->border(0); // Remove window borders
    win->show();

	update_display(output);
    
    // Set X11 properties after window is shown
    set_dock_properties(win);

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