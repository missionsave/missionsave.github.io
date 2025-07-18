//  g++ taskbar.cpp -o taskbar -l:libfltk.a -lX11 -lXtst -Os -s -flto=auto -std=c++20 -lXext -lXft -lXrender -lXcursor -lXinerama -lXfixes -lfontconfig -lfreetype -lz -lm -ldl -lpthread -lstdc++ -w 
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

Fl_Window* win;
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
                    std::cout << "Clipboard contents: " << content << "\n";
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


Fl_Window* winpaste;
int winpop() {
    winpaste = new Fl_Window(100, 100, 300, 200, "Popup");
    Fl_Button* btn = new Fl_Button(100, 80, 100, 40, "Click Me");

    btn->callback([](Fl_Widget*, void*) {
        std::cout << "Button clicked\n";

		string copy_to_clipboard="testing";
		
		const char* text=copy_to_clipboard.c_str();
		Fl::copy(text, strlen(text), 1);
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







#pragma endregion

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


	// Removes placeholders like %F, %U, %f, etc.
	std::string parseExecCommand(const std::string& execLine) {
		std::string cleaned = std::regex_replace(execLine, std::regex("%[fFuUdDnNickvm]"), "");
		return cleaned;
	}

	// Launch SciTE in the background
	void launch(std::string desktopExecLine) {
		std::string command = parseExecCommand(desktopExecLine) + " &";
		system(command.c_str());
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

		std::regex placeholder_re("%[fFuUdDnNickvm]");

		while (std::getline(stream, line)) {
			size_t name_pos = line.find("Name=");
			size_t exec_pos = line.find("Exec=");
			if (name_pos == std::string::npos || exec_pos == std::string::npos)
				continue;

			std::string name = line.substr(name_pos + 5, exec_pos - name_pos - 6);
			std::string exec = line.substr(exec_pos + 5);

			// Remove placeholders like %F
			exec = std::regex_replace(exec, placeholder_re, "");

			uapps[name] = exec;
		}
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
	vector<string> pinnedApps={"Google Chrome","Visual Studio Code","SciTE Text Editor","super@dell: /home/super/msv/apptest"};
#pragma endregion


void refresh(){
	int widthbtn=150;
	vector<Fl_Button*> vbtn(pinnedApps.size());
	for(int i=0;i<pinnedApps.size();i++){
    	vbtn[i] = new Fl_Button(i*(widthbtn+4), 0, widthbtn, 24, pinnedApps[i].c_str());
		vbtn[i]->callback([](Fl_Widget* widget){

			const std::string winTitle = std::string(((Fl_Button*)widget)->label());
			if (lnc->windowExists(winTitle)) {
				lnc->activateWindow(winTitle);
			} else {
				lnc->launch(lnc->uapps[winTitle]);
			}
		});
	}

}


int main() { 
	lnc=new Launcher;
	lnc->fillApplications(lnc->uapps);

    int bar_height = 24; // Adjust height as needed
    int screen_w = Fl::w();
    int screen_h = Fl::h();

    win = new Fl_Window(0, screen_h - bar_height, screen_w, bar_height);
    win->color(FL_DARK_RED);
    win->begin();	
    
	refresh();


    win->end();
    
    win->border(0); // Remove window borders
    win->show();
    
    // Set X11 properties after window is shown
    set_dock_properties(win);

	thread([](){
		listenkey();
 	}).detach();

	thread([](){
		listenclipboard();
 	}).detach();
    
    return Fl::run();
}