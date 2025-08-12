//  g++ taskbar2.cpp ../common/general.cpp -I../common -o taskbar2 -l:libfltk.a -lX11 -lXtst -Os -s -flto=auto -std=c++20 -lXext -lXft -lXrender -lXcursor -lXinerama -lXfixes -lfontconfig -lfreetype -lz -lm -ldl -lpthread -lstdc++ -Os -w -Wfatal-errors -DNDEBUG -lasound  -lXss -lXi -I/usr/include/libnl3
//  g++ taskbar2.cpp ../common/general.cpp -I../common -o taskbar2 -l:libfltk.a -lX11 -lXtst -Os -s -flto=auto -std=c++20 -lXext -lXft -lXrender -lXcursor -lXinerama -lXfixes -lfontconfig -lfreetype -lz -lm -ldl -lpthread -lstdc++ -Os -w -Wfatal-errors -DNDEBUG -lasound  -lXss -lXi $(pkg-config --cflags --libs  dbus-1) 
// sudo setcap cap_net_admin,cap_net_raw+ep ./wifi_connect


//  g++ taskbar2.cpp ../common/general.cpp -I../common -o taskbar2 -l:libfltk.a -lX11 -lXtst -Os -s -flto=auto -std=c++20 -lXext -lXft -lXrender -lXcursor -lXinerama -lXfixes -lfontconfig -lfreetype -lz -lm -ldl -lpthread -lstdc++ -Os -w -Wfatal-errors -DNDEBUG -lasound  -lXss -lXi -ludev


// sudo apt install libasound2-dev acpid wmctrl libudev-dev  pmount

#pragma region  Includes
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
// #include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Help_View.H>
#include <FL/Fl_Browser.H>
#include <FL/fl_ask.H>
#include <FL/x.H> // X11-specific functions

#include <string>
#include <iostream>
#include <thread>
#include <mutex>

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
#include <FL/Fl_Text_Display.H>
#include <fstream>
#include <string>
#include <ctime>

#include <unistd.h>

#include <chrono>

#include "general.hpp"


void dbg();
void refresh(void*);
bool isWifiConnected();
using namespace std;
#pragma endregion Includes
 
#pragma region  Globals
Fl_Window* tasbwin;
Fl_Group* fg;
vector<Fl_Button*> vbtn;
struct WindowInfo;
vector<WindowInfo> vwin;
Fl_Button* btest;

#pragma endregion Globals

#pragma region  Dock
void set_dock_properties(Fl_Window* win) {
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
    strut[3] = win->h();       // Reserve "bottom" space (height of the bar)
    strut[10] = 0;            // Start of reserved area (X start)
    strut[11] = win->w() - 1;  // End of reserved area (X end)

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

    // Prevent window from taking focus
    Atom skip_taskbar = XInternAtom(dpy, "_NET_WM_STATE_SKIP_TASKBAR", False);
    Atom skip_pager = XInternAtom(dpy, "_NET_WM_STATE_SKIP_PAGER", False);
    
    // Add these states to the window
    Atom states[2] = {skip_taskbar, skip_pager};
    XChangeProperty(dpy, xid, wm_state, XA_ATOM, 32, PropModeAppend,
                   (unsigned char*)states, 2);

    // Set input hints to prevent focus
    XWMHints* hints = XAllocWMHints();
    hints->flags = InputHint;
    hints->input = False;  // This window should never receive input focus
    XSetWMHints(dpy, xid, hints);
    XFree(hints);

    // Set the window's override_redirect attribute
    // This prevents window manager from managing our window (including focus)
    // Comment this out if you need proper strut behavior with some WMs
    XSetWindowAttributes attrs;
    attrs.override_redirect = True;
    XChangeWindowAttributes(dpy, xid, CWOverrideRedirect, &attrs);

    // Some window managers need this to properly handle dock windows
    Atom wm_desktop = XInternAtom(dpy, "_NET_WM_DESKTOP", False);
    unsigned long desktop = 0xFFFFFFFF; // Show on all desktops
    XChangeProperty(dpy, xid, wm_desktop, XA_CARDINAL, 32, PropModeReplace,
                   (unsigned char*)&desktop, 1);
#ifdef __linux__1
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

#pragma endregion Dock

#pragma region  right_statusbar
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
string batstatus(){
	std::string status  = read_file("/sys/class/power_supply/BAT0/status");
	trim(status);
	return status;
}
bool bat_is_Discharging(){
	return batstatus()=="Discharging";
}
// Create the floating message window
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <X11/Xlib.h>
#include <X11/Xatom.h> 
#include <FL/x.H>

Fl_Window* create_system_modal_message(const char* msg) {
    // Create window with a message
    Fl_Window* popup = new Fl_Window(300, 150, "Alert");
    Fl_Box* box = new Fl_Box(20, 20, 260, 110, msg);
    box->align(FL_ALIGN_WRAP | FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
    
    // Window settings
    popup->set_non_modal();
    popup->border(0);
    popup->clear_border();
    
    // Make it stay on top
    popup->set_override();
    
    // Show before setting X11 properties
    popup->show();
    
    // X11 specific settings
    Window xid = fl_xid(popup);
    Display* dpy = fl_display;
    
    // Set window type to utility/dialog to make it float
    Atom window_type = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
    Atom dialog_type = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DIALOG", False);
    XChangeProperty(dpy, xid, window_type, XA_ATOM, 32, 
                   PropModeReplace, (unsigned char*)&dialog_type, 1);
    
    // Set to stay on top
    Atom above = XInternAtom(dpy, "_NET_WM_STATE_ABOVE", False);
    Atom state = XInternAtom(dpy, "_NET_WM_STATE", False);
    XChangeProperty(dpy, xid, state, XA_ATOM, 32,
                   PropModeReplace, (unsigned char*)&above, 1);
    
    // Position window (optional)
    XMoveWindow(dpy, xid, 100, 100);
    
    // Make sure it's visible
    XMapRaised(dpy, xid);
    XFlush(dpy);
    
    return popup;
}

void update_display(Fl_Help_View* output) {
    std::string battery = read_file("/sys/class/power_supply/BAT0/capacity"); 
	// std::string status  = read_file("/sys/class/power_supply/BAT0/status");

    std::string clock   = get_time();


// 1. Build a single-row, two-cell table that spans 100% width
// std::ostringstream html;
// html << "<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\">"
//      << "  <tr>"
//      // spacer cell to push the second cell flush right
//      << "    <td width=\"99%\"></td>"
//      // our content cell, top-aligned and right-aligned
//      << "    <td align=\"right\" valign=\"top\">"
//      <<      "<font color=\"green\">wifi </font>" 
//                << battery << "% " << clock
//      << "    </td>"
//      << "  </tr>"
//      << "</table>";

// // 2. Set the HTML and force the view back to the very top
// output->value(html.str().c_str());
// // output->topline(0);



// output->wrap(0);
Fl::set_font(FL_SYMBOL, "Noto Color Emoji");
// Fl::set_font(FL_SYMBOL, "Noto Sans Symbols");
std::ostringstream html;
html<<"<center>"
	// <<"<font face=arial  color=green><b>wifi</b></font>"
	<<"<font face=symbol >📡📶</font>"
	<<"<font face=symbol >🔋</font>"
	<<"<font face=helvetica  color=green><b>"<<battery<<"%"<<"</b>"<<" </font>"
	<<"<font face=helvetica  color=black><b>"<<clock<<"</b>"<<" </font>";

output->value(html.str().c_str());



    // output->value(("<center><font face=symbol color=green>wifi 🔋</font> 🔋"+battery + "% " + clock+"").c_str());
    // output->value(("<center><font color=green>wifi </font>"+battery + "% " + clock+"").c_str());
	// output->align(FL_ALIGN_RIGHT);
	// output->leftline(20);

output->deactivate();


	// output->topline(0);
	// output->value(("<table width=\"100%\"><tr><td align=\"right\"><font color=\"green\">wifi </font>" + battery + "% " + clock + "</td></tr></table>").c_str());
	if(atoi(battery.c_str())<=5){ 
		if(bat_is_Discharging()){ 
			// sleep(3);
			Fl_Window* warn=create_system_modal_message("Warning! Low Battery. Connect charger or system will suspend.");
			// Continue program execution while window shows
			
			Fl::add_timeout(1, [](void* w) {
				Fl_Window* win = static_cast<Fl_Window*>(w);
				int ci=0;
				while(ci<100){ // seconds to get charger
					ci++;
					if(!bat_is_Discharging())break;
					sleep(1);
				}
				if (win) {
					win->hide();
					delete win;
					if(bat_is_Discharging()){
						cotm("suspend");
						system("systemctl suspend");
					}					
				}
			}, warn);
		}
	}

    Fl::repeat_timeout(30.0, (Fl_Timeout_Handler)update_display, output);
}




#pragma endregion right_statusbar

#pragma region  Clipboard

vstring vclipboard;
Fl_Browser* browserpaste;

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
// PopupWindow* winpaste; 
Fl_Window* winpaste; 
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
		// Fl::grab(nullptr);
		// trigger_winpop = false;
		delete browserpaste;
		delete winpaste;
		winpaste=0;
    }
} 
void winpop(void*) {
	Fl_Group::current(nullptr); // important
    winpaste = new Fl_Window(0, Fl::h()-300, 400, 300, "Popup");
    // winpaste = new PopupWindow(0, Fl::h()-300, 400, 300, "Popup");
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

 
}

#include <iostream>
#include <X11/Xlib.h>
#include <X11/extensions/XInput2.h>
#include <FL/Fl.H>
#include <FL/x.H>


#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <iostream>

void disable_v(Display* dpy, KeyCode kc_v) {
    KeySym nosym = NoSymbol;
    XChangeKeyboardMapping(dpy, kc_v, 1, &nosym, 1);
    XFlush(dpy);
}

void restore_v(Display* dpy, KeyCode kc_v) {
    KeySym syms[2] = {XK_v, XK_V};
    XChangeKeyboardMapping(dpy, kc_v, 2, syms, 2);
    XFlush(dpy);
}


int listenkey() {
    Display* dpy = XOpenDisplay(nullptr);
    if (!dpy) {
        std::cerr << "Cannot open display\n";
        return 1;
    }

    Window root = DefaultRootWindow(dpy);

    // --- EXISTING: grab Super+V ---
    KeyCode kc_v = XKeysymToKeycode(dpy, XStringToKeysym("v"));
    unsigned int super_mod = Mod4Mask;

    for (int i = 0; i < 8; ++i) {
        XGrabKey(dpy, kc_v, super_mod | i, root, True,
                 GrabModeAsync, GrabModeAsync);
    }

    // --- NEW: grab Print Screen (no modifier) ---
    KeyCode kc_print = XKeysymToKeycode(dpy, XK_Print);
    for (int i = 0; i < 8; ++i) {
        // i covers variants: NumLock, CapsLock, ScrollLock, etc.
        XGrabKey(dpy, kc_print, i, root, True,
                 GrabModeAsync, GrabModeAsync);
    }

    XSelectInput(dpy, root, KeyPressMask);
    XFlush(dpy);

    std::cout << "[INFO] Listening for Super+V and Print Screen...\n";

    while (true) {
        XEvent ev;
        XNextEvent(dpy, &ev);

        if (ev.type == KeyPress) {
            XKeyEvent xkey = ev.xkey;

            // Super+V handling
            if (xkey.keycode == kc_v && (xkey.state & super_mod)) {
                // Your existing action, e.g. paste popup
                if (!winpaste)
                    Fl::awake(winpop);
            }

            // Print Screen handling
            else if (xkey.keycode == kc_print) {
                // Launch Flameshot GUI
				system("maim -s | xclip -selection clipboard -t image/png &");

            }
        }
    }

    // Cleanup (never reached in this infinite loop example)
    XUngrabKey(dpy, kc_v, AnyModifier, root);
    XUngrabKey(dpy, kc_print, AnyModifier, root);
    XCloseDisplay(dpy);
    return 0;
}

 

int listenkeyworkwinkey() {
    Display* dpy = XOpenDisplay(nullptr);
    if (!dpy) {
        std::cerr << "Cannot open display\n";
        return 1;
    }

    Window root = DefaultRootWindow(dpy);

    // Obter keycode para 'v'
    KeyCode kc_v = XKeysymToKeycode(dpy, XStringToKeysym("v"));

    // Máscara de modificação para Super (usualmente Mod4)
    unsigned int mod = Mod4Mask;

    // Gravar Super+V (com todas variantes de Lock, NumLock etc)
    for (int i = 0; i < 8; ++i) {
        XGrabKey(dpy, kc_v, mod | i, root, True,
                 GrabModeAsync, GrabModeAsync);
    }

    XSelectInput(dpy, root, KeyPressMask);
    XFlush(dpy);

    std::cout << "[INFO] A ouvir Super+V...\n";

    while (true) {
        XEvent ev;
        XNextEvent(dpy, &ev);

        if (ev.type == KeyPress) {
            XKeyEvent xkey = ev.xkey;

            if (xkey.keycode == kc_v && (xkey.state & Mod4Mask)) {
                // Impede que o 'v' seja escrito, executa apenas ação
                if (!winpaste)
                    Fl::awake(winpop);
            }
        }
    }

    XUngrabKey(dpy, kc_v, AnyModifier, root);
    XCloseDisplay(dpy);
    return 0;
}



int listenkeynf() {
    Display* dpy = XOpenDisplay(nullptr);
    if (!dpy) return 1;

    // … (inicialização XInput2, seleção de eventos raw)

    KeyCode kc_super = XKeysymToKeycode(dpy, XStringToKeysym("Super_L"));
    KeyCode kc_v     = XKeysymToKeycode(dpy, XStringToKeysym("v"));
    bool super_down = false;

    while (true) {
        XEvent ev;
        XNextEvent(dpy, &ev);
        // … (filtro de GenericEvent + XGetEventData)
        XIRawEvent* raw = reinterpret_cast<XIRawEvent*>(ev.xcookie.data);

        if (ev.xcookie.evtype == XI_RawKeyPress) {
            if (raw->detail == kc_super) {
                super_down = true;
                disable_v(dpy, kc_v);
            }
            else if (raw->detail == kc_v && super_down) {
                // Aqui já não será enviado 'v' à janela.
                if(!winpaste) Fl::awake(winpop);
            }
        }
        else if (ev.xcookie.evtype == XI_RawKeyRelease) {
            if (raw->detail == kc_super) {
                super_down = false;
                restore_v(dpy, kc_v);
            }
        }

        XFreeEventData(dpy, &ev.xcookie);
    }

    XCloseDisplay(dpy);
    return 0;
}


int listenkeypw() {
    Display* dpy = XOpenDisplay(nullptr);
    if (!dpy) {
        std::cerr << "Cannot open display\n";
        return 1;
    }

    // 1) Inicializar XInput2
    int xi_opcode, xev, xerr;
    if (!XQueryExtension(dpy, "XInputExtension", &xi_opcode, &xev, &xerr)) {
        std::cerr << "XInput2 not available\n";
        XCloseDisplay(dpy);
        return 1;
    }

    int major = 2, minor = 2;
    if (XIQueryVersion(dpy, &major, &minor) != Success) {
        std::cerr << "XInput2 v2.2 not available\n";
        XCloseDisplay(dpy);
        return 1;
    }

    // 2) Preparar máscara para RawKeyPress e RawKeyRelease
    XIEventMask evmask;
    unsigned char mask[XIMaskLen(XI_LASTEVENT)] = {0};
    evmask.deviceid = XIAllMasterDevices;
    evmask.mask_len = sizeof(mask);
    evmask.mask = mask;
    XISetMask(mask, XI_RawKeyPress);
    XISetMask(mask, XI_RawKeyRelease);

    // 3) Selecionar eventos no root window
    Window root = DefaultRootWindow(dpy);
    XISelectEvents(dpy, root, &evmask, 1);
    XFlush(dpy);

    std::cout << "[INFO] Key listener iniciado (raw XInput2)\n";

    // 4) Preparar keycodes de Super_L e 'v'
    KeyCode kc_super = XKeysymToKeycode(dpy, XStringToKeysym("Super_L"));
    KeyCode kc_v     = XKeysymToKeycode(dpy, XStringToKeysym("v"));

    bool super_down = false;

    // 5) Loop de eventos
    while (true) {
        XEvent ev;
        XNextEvent(dpy, &ev);

        if (ev.type == GenericEvent && ev.xcookie.extension == xi_opcode) {
            if (XGetEventData(dpy, &ev.xcookie)) {
                XIRawEvent* raw = reinterpret_cast<XIRawEvent*>(ev.xcookie.data);

                if (ev.xcookie.evtype == XI_RawKeyPress) {
                    // Super_L pressionado?
                    if (raw->detail == kc_super) {
                        super_down = true;
                        // std::cout << "[DEBUG] Super_L pressionado\n";
                    }
                    // 'v' enquanto Super está para baixo?
                    else if (raw->detail == kc_v && super_down) {
                        // std::cout << "[DEBUG] Super+V detectado\n";
						if(!winpaste)
                        	Fl::awake(winpop);
                    }
                }
                else if (ev.xcookie.evtype == XI_RawKeyRelease) {
                    // Super_L libertado?
                    if (raw->detail == kc_super) {
                        super_down = false;
                        // std::cout << "[DEBUG] Super_L soltado\n";
                    }
                }

                XFreeEventData(dpy, &ev.xcookie);
            }
        }
    }

    XCloseDisplay(dpy);
    return 0;
}



int listenkeyp() {
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

            // trigger_winpop = true;
            Fl::awake(winpop);
            cotm("wpress");
			//  Fl::awake(); 
			//  winpop();                      
    		// Fl::add_idle(check_trigger, nullptr);
        }
    }

    XCloseDisplay(dpy);
    return 0;
}


void closewinpaste(void*){
	browserpaste->hide();
	winpaste->hide();
		// Fl::grab(nullptr);
		// trigger_winpop = false;
	delete browserpaste;
	delete winpaste;
	winpaste=0;
}

#include <X11/Xlib.h>
#include <X11/extensions/XInput2.h>
#include <FL/x.H>  // For fl_xid()
#include <iostream>

#include <X11/Xlib.h>
#include <X11/extensions/XInput2.h>
#include <FL/Fl.H>
#include <FL/x.H>
#include <iostream>
#include <cstring>
#include <iostream>
#include <X11/Xlib.h>
#include <X11/extensions/XInput2.h>

// Se preferires usar macro, podes definir assim:
// #define DEBUG(x) std::cout << x << std::endl;

#include <iostream>
#include <X11/Xlib.h>
#include <X11/extensions/XInput2.h>
#include <iostream>
#include <X11/Xlib.h>
#include <X11/extensions/XInput2.h>

int listen_mouse_outside_window() {
    Display* dpy = XOpenDisplay(nullptr);
    if (!dpy) {
        std::cerr << "Cannot open display\n";
        return 1;
    }

    // Verificar extensão XInput2
    int xi_opcode, event, error;
    if (!XQueryExtension(dpy, "XInputExtension", &xi_opcode, &event, &error)) {
        std::cerr << "XInput2 not available\n";
        XCloseDisplay(dpy);
        return 1;
    }

    // Confirmar versão 2.2 ou superior
    int major = 2, minor = 2;
    if (XIQueryVersion(dpy, &major, &minor) != Success) {
        std::cerr << "XInput2 version 2.2 not available\n";
        XCloseDisplay(dpy);
        return 1;
    }

    // Configurar para escutar XI_RawButtonPress
    XIEventMask evmask;
    unsigned char mask[XIMaskLen(XI_LASTEVENT)] = {0};
    evmask.deviceid = XIAllMasterDevices;
    evmask.mask_len = sizeof(mask);
    evmask.mask = mask;
    XISetMask(mask, XI_RawButtonPress);

    Window root = DefaultRootWindow(dpy);
    XISelectEvents(dpy, root, &evmask, 1);
    XFlush(dpy);

    std::cout << "[INFO] Mouse listener iniciado (XInput2 - raw events)\n";

    while (true) {
        XEvent ev;
        XNextEvent(dpy, &ev);

        if (ev.type == GenericEvent && ev.xcookie.extension == xi_opcode) {
            if (XGetEventData(dpy, &ev.xcookie)) {
                if (ev.xcookie.evtype == XI_RawButtonPress) {
                    // Obter posição do cursor com XQueryPointer
                    int rx, ry, winx, winy;
                    unsigned int mask_return;
                    Window child_return;
                    XQueryPointer(dpy, root, &root, &child_return,
                                  &rx, &ry, &winx, &winy, &mask_return);

                    // std::cout << "[DEBUG] Click em posição (" << rx << ", " << ry << ")\n";

                    if (winpaste) {
                        Window fltk_window = fl_xid(winpaste);

                        // Obter posição da janela
                        int win_x = 0, win_y = 0;
                        Window unused_win;
                        XTranslateCoordinates(dpy, fltk_window, root,
                                              0, 0, &win_x, &win_y, &unused_win);

                        // Obter dimensões da janela
                        Window dummy_win;
                        int dummy_x, dummy_y;
                        unsigned int w, h, border, depth;
                        XGetGeometry(dpy, fltk_window, &dummy_win, &dummy_x, &dummy_y,
                                     &w, &h, &border, &depth);

                        std::cout << "[DEBUG] FLTK window pos: (" << win_x << ", " << win_y
                                  << "), size: (" << w << "x" << h << ")\n";

                        bool outside =
                            rx < win_x || rx > win_x + static_cast<int>(w) ||
                            ry < win_y || ry > win_y + static_cast<int>(h);

                        if (outside) {
                            std::cout << "[INFO] Click fora da janela detectado\n";
                            Fl::awake(closewinpaste);  // Fechar janela com a tua função
                        } else {
                            std::cout << "[INFO] Click dentro da janela\n";
                        }
                    }
                }
                XFreeEventData(dpy, &ev.xcookie);
            }
        }
    }

    XCloseDisplay(dpy);
    return 0;
}


int listen_mouse_outside_windowp() {  
    Display* dpy = XOpenDisplay(nullptr);
    if (!dpy) {
        std::cerr << "Cannot open display\n";
        return 1;
    }

    Window root = DefaultRootWindow(dpy);
// pausa
    // Grab mouse buttons globally (passive grab)
    for (int button = 1; button <= 5; ++button) {
        XGrabButton(dpy, button, AnyModifier, root, True,
                    ButtonPressMask, GrabModeSync, GrabModeAsync,
                    None, None);
    }
    // XSelectInput(dpy, root, ButtonPressMask); //X_ChangeWindowAttributes: BadAccess (attempt to access private resource denied) 0x55a
	// int ci=0;
    while (true) {
        XEvent ev; 
        XNextEvent(dpy, &ev);
		// cotm(ci++)
		// cotmup;

        if (ev.type == ButtonPress) {
            int click_x = ev.xbutton.x_root;
            int click_y = ev.xbutton.y_root;

            bool clicked_outside = false;

            if (winpaste) {
                Window fltk_window_id = fl_xid(winpaste);
                Window root_ret;
                int x, y;
                unsigned w, h, bw, depth;
                XGetGeometry(dpy, fltk_window_id, &root_ret, &x, &y, &w, &h, &bw, &depth);

                if (click_x < x || click_x > x + (int)w ||
                    click_y < y || click_y > y + (int)h) {
                    clicked_outside = true;
                    std::cout << "Clicked outside FLTK window at (" << click_x << ", " << click_y << ")\n";
                    Fl::awake(closewinpaste);
                }
            }

            // 🚨 This is the critical part: let the click proceed normally
            XAllowEvents(dpy, ReplayPointer, CurrentTime);
            XFlush(dpy);
        }
    }

    XCloseDisplay(dpy);
    return 0;
}



#pragma endregion Clipboard

#pragma region  fnbrightnessandvolume

// sudo apt install libasound2-dev acpid
// g++ -std=c++17 -o fnhandle fnhandle.cpp -lasound 
#include <algorithm>
#include <iostream>
#include <cstdio>
#include <string>
#include <memory>
#include <filesystem>
#include <vector>
#include <array>
#include <fstream>
#include <unistd.h>
#include <alsa/asoundlib.h> // For audio control

namespace fs = std::filesystem;

// ==================== Brightness Control ====================
class BrightnessManager {
public:
    BrightnessManager() {
        backlight_path_ = find_backlight_path();
        if (backlight_path_.empty()) {
            throw std::runtime_error("No backlight interface found");
        }
        max_brightness_ = read_int(backlight_path_ + "/max_brightness");
        std::cout << "Using backlight: " << backlight_path_ << "\n";
    }

    void adjust(int delta) {
        int current = read_int(backlight_path_ + "/brightness");
        int new_value = current + delta;
        new_value = std::clamp(new_value, 10, max_brightness_);
        write_int(backlight_path_ + "/brightness", new_value);
    }

private:
    std::string backlight_path_;
    int max_brightness_;

    static std::string find_backlight_path() {
        const std::string base_path = "/sys/class/backlight/";
        try {
            for (const auto& entry : fs::directory_iterator(base_path)) {
                if (entry.is_directory()) {
                    std::string path = entry.path().string();
                    if (fs::exists(path + "/brightness") && 
                        fs::exists(path + "/max_brightness")) {
                        return path;
                    }
                }
            }
        } catch (const fs::filesystem_error&) {}
        return "";
    }

    static int read_int(const std::string& path) {
        std::ifstream file(path);
        if (!file) throw std::runtime_error("Can't read " + path);
        int value;
        file >> value;
        return value;
    }

    static void write_int(const std::string& path, int value) {
        std::ofstream file(path);
        if (!file) throw std::runtime_error("Can't write " + path);
        file << value;
    }
};

// ==================== Audio Control ====================
class AudioManager {
public:
    AudioManager() {
        if (snd_mixer_open(&mixer_handle_, 0) != 0) {
            throw std::runtime_error("Couldn't open ALSA mixer");
        }
        if (snd_mixer_attach(mixer_handle_, "default") != 0) {
            snd_mixer_close(mixer_handle_);
            throw std::runtime_error("Couldn't attach to default mixer");
        }
        if (snd_mixer_selem_register(mixer_handle_, nullptr, nullptr) != 0) {
            snd_mixer_close(mixer_handle_);
            throw std::runtime_error("Mixer registration failed");
        }
        if (snd_mixer_load(mixer_handle_) != 0) {
            snd_mixer_close(mixer_handle_);
            throw std::runtime_error("Couldn't load mixer controls");
        }

        // Find master playback volume control
        snd_mixer_selem_id_alloca(&sid_);
        snd_mixer_selem_id_set_index(sid_, 0);
        snd_mixer_selem_id_set_name(sid_, "Master");
        elem_ = snd_mixer_find_selem(mixer_handle_, sid_);
        if (!elem_) {
            snd_mixer_close(mixer_handle_);
            throw std::runtime_error("Couldn't find master control");
        }
    }

    ~AudioManager() {
        if (mixer_handle_) snd_mixer_close(mixer_handle_);
    }

    void adjust_volume(int delta) {
        long min, max, current;
        snd_mixer_selem_get_playback_volume_range(elem_, &min, &max);
        snd_mixer_selem_get_playback_volume(elem_, SND_MIXER_SCHN_FRONT_LEFT, &current);
        
        long new_vol = current + (delta * max / 100); // Delta as percentage of max
        new_vol = std::clamp(new_vol, min, max);
        
        snd_mixer_selem_set_playback_volume_all(elem_, new_vol);
    }

    void toggle_mute() {
        int mute;
        snd_mixer_selem_get_playback_switch(elem_, SND_MIXER_SCHN_FRONT_LEFT, &mute);
        snd_mixer_selem_set_playback_switch_all(elem_, !mute);
    }

private:
    snd_mixer_t* mixer_handle_ = nullptr;
    snd_mixer_selem_id_t* sid_ = nullptr;
    snd_mixer_elem_t* elem_ = nullptr;
};

// ==================== ACPI Event Handler ====================
void handle_acpi_events(BrightnessManager& brightness, AudioManager& audio) {
    constexpr int brightness_step = 50;
    constexpr int volume_step = 5; // 5% volume change per key press
    
    std::array<char, 256> buffer;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("acpi_listen", "r"), pclose);
    if (!pipe) throw std::runtime_error("Failed to start acpi_listen");

    std::cout << "Listening for ACPI events...\n";
    while (fgets(buffer.data(), buffer.size(), pipe.get())) {
        std::string event(buffer.data());
        
        // Brightness control
        if (event.find("video/brightnessup") != std::string::npos) {
            brightness.adjust(brightness_step);
            std::cout << "Brightness increased\n";
        }
        else if (event.find("video/brightnessdown") != std::string::npos) {
            brightness.adjust(-brightness_step);
            std::cout << "Brightness decreased\n";
        }
        // Volume control
        else if (event.find("button/volumeup") != std::string::npos) {
            audio.adjust_volume(volume_step);
            std::cout << "Volume increased\n";
        }
        else if (event.find("button/volumedown") != std::string::npos) {
            audio.adjust_volume(-volume_step);
            std::cout << "Volume decreased\n";
        }
        else if (event.find("button/mute") != std::string::npos) {
            audio.toggle_mute();
            std::cout << "Mute toggled\n";
        }
    }
}

int listen_fnvolumes() {
    try {
        BrightnessManager brightness;
        AudioManager audio;
        handle_acpi_events(brightness, audio);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << "\n";
        
        std::cerr << "\nTroubleshooting:\n";
        std::cerr << "1. Install required packages:\n";
        std::cerr << "   - acpid (for acpi_listen)\n";
        std::cerr << "   - libasound2-dev (for ALSA development files)\n";
        std::cerr << "2. Check your user is in audio/video groups\n";
        std::cerr << "3. Verify acpid service is running\n";
        
        return 1;
    }
    return 0;
}




#pragma endregion fnbrightnessandvolume

#pragma region  screensaver
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
void initscrensaver() {
    constexpr int IDLE_THRESHOLD = 60*5; // segundos para desligar o ecrã
    int idle_seconds = 0;
	int interval=30;
    while (true) {
        unsigned long idle_ms = get_idle_time_ms();
        focused_pid = get_active_window_pid();
        // pid_t focused_pid = get_active_window_pid();

        if (idle_ms < 1000*interval) {
            // Input recente do rato/teclado: reseta contador
            idle_seconds = 0;
        } else if (focused_pid == -1 || !is_audio_focused(focused_pid)) {
            // Sem foco ou áudio não relacionado com a janela ativa: incrementa contador
            idle_seconds+=interval;
        } else {
            // Áudio relacionado com a janela ativa: reseta contador
            idle_seconds = 0;
        }

        std::cout << "Idle: " << idle_seconds << "s\r" << std::flush;

        if (idle_seconds >= IDLE_THRESHOLD) {
            std::cout << "\nDesligando ecrã...\n";
            force_screen_off();
            idle_seconds = 0;
        }

        std::this_thread::sleep_for(std::chrono::seconds(interval));
    }

    // return 0;
}

#pragma endregion screensaver

#pragma region  Launcher
struct WindowInfo {
    std::string window_id;
    std::string title;
    int pid;
    std::string process_name;
	bool is_open=0;
	pid_t active_pid=0;

};

unordered_map <string,string> uapps;
vector<string> pinnedApps={"Google Chrome","Visual Studio Code","tmux x86_64"};

Window stringToWindow(const std::string& winStr) {
    Window win;
    std::stringstream ss;

    // If the string starts with "0x", interpret as hexadecimal
    if (winStr.rfind("0x", 0) == 0) {
        ss << std::hex << winStr;
    } else {
        ss << winStr; // assume decimal
    }

    ss >> win;
    return win;
}

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
		// if (desktop_num == "-1") {
		// 	continue; // Skip sticky windows (appear on all desktops)
		// }
        iss >> info.pid;       // PID
		cotm(info.pid);
        std::string hostname;
        iss >> hostname;       // Hostname (ignored)

        std::string title;
        std::getline(iss, title);
        title.erase(0, title.find_first_not_of(" \t\n\r")); // Trim leading spaces
        info.title = title;
		// if(info.title=="")continue;

		info.active_pid=get_active_window_pid();

        info.process_name = getProcessName(info.pid);

        windows.push_back(info);
    }

    return windows;
}
	// Removes placeholders like %F, %U, %f, etc.
	std::string parseExecCommand(const std::string& execLine) {
		// return execLine;
		std::string cleaned = std::regex_replace(execLine, std::regex("%[fFuUdDnNickvm]"), "");
		return cleaned;
	}

	void fillApplications(std::unordered_map<std::string, std::string>& uapps) {
	const char* command =
		"find /usr/share/applications ~/.local/share/applications -name '*.desktop' 2>/dev/null | "
		"xargs stat --format='%Y %n' 2>/dev/null | "
		"sort -nr | cut -d' ' -f2- | "
		"xargs grep -l 'Type=Application' 2>/dev/null | "
		"xargs grep -L 'NoDisplay=true' 2>/dev/null | "
		"xargs grep -L 'Hidden=true' 2>/dev/null | "
		"xargs -d '\\n' awk 'BEGIN{ORS=\"\"} /^Name=/{name=$0; next} /^Exec=/{exec=$0; gsub(/ *%[fFuUdDnNickvm]/, \"\", exec); print name \"\\t\" exec \"\\n\"}'";

	std::array<char, 512> buffer;
	std::string output;
	std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command, "r"), pclose);
	if (!pipe) return;

	while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
		output += buffer.data();
	}

	std::istringstream stream(output);
	std::string line;
	std::unordered_set<std::string> seen_execs;

	while (std::getline(stream, line)) {
		size_t tab_pos = line.find('\t');
		if (tab_pos == std::string::npos)
			continue;

		std::string name = line.substr(5, tab_pos - 5); // remove "Name="
		std::string exec = line.substr(tab_pos + 1);
		if (exec.rfind("Exec=", 0) == 0)
			exec = exec.substr(5); // remove "Exec="

		trim(name);
		trim(exec);
		exec = parseExecCommand(exec);

		if (name.empty() || exec.empty())
			continue;

		// Skip duplicates
		if (seen_execs.count(exec))
			continue;
		seen_execs.insert(exec);

		cotm(name, exec);
		uapps[name] = exec;
	}
}

	void fillApplications_v2(std::unordered_map<std::string, std::string>& uapps) {
	const char* command =
		"find /usr/share/applications ~/.local/share/applications -name '*.desktop' 2>/dev/null | "
		"xargs stat --format='%Y %n' 2>/dev/null | "
		"sort -nr | cut -d' ' -f2- | "
		"xargs grep -l 'Type=Application' 2>/dev/null | "
		// "xargs grep -L 'NoDisplay=true' 2>/dev/null | "
		"xargs grep -L 'Hidden=true' 2>/dev/null | "
		"xargs -d '\\n' awk 'BEGIN{ORS=\"\"} /^(Name=|Exec=)/{print $0\"\\n\"}' | "
		"paste - -";
	// const char* command =
	// 	"find /usr/share/applications ~/.local/share/applications -name '*.desktop' 2>/dev/null | "
	// 	"xargs stat --format='%Y %n' 2>/dev/null | "
	// 	"sort -nr | cut -d' ' -f2- | "
	// 	"xargs grep -l 'Type=Application' 2>/dev/null | "
	// 	"xargs grep -L 'NoDisplay=true' 2>/dev/null | "
	// 	"xargs grep -L 'Hidden=true' 2>/dev/null | "
	// 	"xargs -d '\\n' awk 'BEGIN{ORS=\"\"} /^(Name=|Exec=)/{print $0\"\\n\"}' | "
	// 	"paste - -";

	std::array<char, 512> buffer;
	std::string output;
	std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command, "r"), pclose);
	if (!pipe) return;

	while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
		output += buffer.data();
	}

	std::istringstream stream(output);
	std::string line;

	while (std::getline(stream, line)) {
		size_t name_pos = line.find("Name=");
		size_t exec_pos = line.find("Exec=");
		if (name_pos == std::string::npos || exec_pos == std::string::npos)
			continue;

		std::string name = line.substr(name_pos + 5, exec_pos - name_pos - 6);
		std::string exec = line.substr(exec_pos + 5);

		cotm(name, exec);

		exec = parseExecCommand(exec);
		trim(name);
		trim(exec);
		uapps[name] = exec;
	}
}

	void fillApplications_v1(std::unordered_map<std::string, std::string>& uapps) {
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

			cotm(name,exec);

			// Remove placeholders like %F
			// exec = std::regex_replace(exec, placeholder_re, "");
			string res=parseExecCommand(exec);
			trim(name);
			trim(res);
			uapps[name] = res; 

		}
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
	cotm("getProcessArgs",args);
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

// implement value find duplicates
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
void activateWindowById(const std::string& winId) {
	std::string cmd = "wmctrl -i -a " + winId;
	system(cmd.c_str());
}


void launch(const std::string & execCommandr, bool dropPrivileges=true) {
	string execCommand=parseExecCommand(execCommandr);
    pid_t pid = fork();

    if (pid < 0) {
        std::cerr << "Failed to fork\n";
        return;
    }

    if (pid > 0) {
        return;  // Parent returns
    }

    // Optional: setsid() if you really want (but usually unnecessary for GUI apps)
    setsid();

    if (dropPrivileges) {
        uid_t ruid = getuid();
        gid_t rgid = getgid();
        setgid(rgid);
        setuid(ruid);
    }

    // Optional: Silence stdio
    fclose(stdin);
    fclose(stdout);
    fclose(stderr);
    stdin  = fopen("/dev/null", "r");
    stdout = fopen("/dev/null", "w");
    stderr = fopen("/dev/null", "w");

    const char* display = getenv("DISPLAY");
	cotm(display)
    if (!display) display = ":0";

    std::string fullCommand = "DISPLAY=" + std::string(display) + " " + execCommand;
cotm(fullCommand)
    std::vector<const char*> argv = { "/bin/sh", "-c", fullCommand.c_str(), nullptr };

    execv("/bin/sh", const_cast<char* const*>(argv.data()));

    _exit(1);
}

int teste(bool t=0){if(t)return 9999;else return 7777;}
void minimizeWindow(Window win) {
	Display* display = XOpenDisplay(nullptr);
    if (!display) {
        std::cerr << "Cannot open X display\n";
        return;
    }
    XIconifyWindow(display, win, DefaultScreen(display));
    XFlush(display);
    XCloseDisplay(display);
}


void refresh(void*){ 
	fg->begin();
	int widthbtn=150;

	for (int i = 0; i < vbtn.size(); i++){
		vbtn[i]->hide();
		delete vbtn[i];
	}
	vbtn.clear();
	vwin.clear();

	pid_t pid_=get_active_window_pid();
	cotm(pid_);
	//pinned
	perf();
	vector<WindowInfo> windows = getOpenWindowsInfo();
	perf("getwindows");
	cotm(windows.size())
	for(int ip=0;ip<pinnedApps.size();ip++){
		vwin.push_back(WindowInfo());
		vwin.back().title=pinnedApps[ip];
		vwin.back().is_open = 0;
		vwin.back().active_pid =pid_;
		// cout<<"is "<<vwin.back().is_open<<"\n";
		for (auto it = windows.begin(); it != windows.end(); /* no ++ here */) {
			// cotm(it->title,it->pid,it->window_id)
			// if()
			if(it->title==""){++it; continue;}
			string sexecp = getExecByPid(it->pid);
			string sexec =  (sexecp); 
			// string sexec = parseExecCommand(sexecp); 
			trim(sexec);
			// cotm(pinnedApps[ip] ,sexecp,sexec)
			string sexecf = findKeyByValue(uapps, sexec).value_or("");
			// string sexecf = findKeyByValue(uapps, sexecp).value_or("");
			cotm(pinnedApps[ip],sexecf,sexec);
			if (pinnedApps[ip] == sexecf) {
				vwin.back().title = sexecf;
				// vwin.back().title = uapps[sexecf];
				vwin.back().window_id = it->window_id;
				vwin.back().is_open = 1;
				vwin.back().active_pid =pid_;
				vwin.back().pid = it->pid;
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
			string sexec = parseExecCommand(sexecp); 
			trim(sexec);
			string sexecf = findKeyByValue(uapps, sexecp).value_or("");

			if (sexec== sexecf) {
				vwin.push_back(WindowInfo());
				vwin.back().title = sexecf;
				// vwin.back().title = uapps[sexecf];
				vwin.back().window_id = it->window_id;
				vwin.back().is_open = 1;
				vwin.back().active_pid =pid_;
				vwin.back().pid = it->pid;
			} else {
				vwin.push_back(WindowInfo());
				vwin.back().title = it->title;
				vwin.back().window_id = it->window_id;
				vwin.back().is_open = 1;
				vwin.back().active_pid =pid_;
				vwin.back().pid = it->pid;
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
			// pid_t pid=wi.active_pid;
			pid_t pid=get_active_window_pid();

			cotm(wi.pid,pid,focused_pid);
			if(wi.pid==pid){
				minimizeWindow(stringToWindow( wi.window_id));
				return;
			}
// cotm(wi.is_open )
			if(wi.is_open){
				activateWindowById(wi.window_id);
			}
			if(!wi.is_open){
// cotm(wi.title)
				launch(uapps[wi.title]);
				// fg->begin(); refresh(); fg->end(); 
				tasbwin->redraw();
				btn->redraw();
				btn->show();
				// launch(uapps[wi.title]);
				// activateWindow(wi.title);
			}
			// const std::string winTitle = std::string(((Fl_Button*)widget)->label());
			// if (windowExists(winTitle)) {
			// 	activateWindow(winTitle);
			// 	btn->redraw();
			// 	btn->show();
				return;
			// }
		},&vwin[i]);
	}
	tasbwin->end();
fg->end();
}
 
int eventWindow() {
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        std::cerr << "Cannot open X display\n";
        return 1;
    }

    Window root = DefaultRootWindow(display);
    XSelectInput(display, root, SubstructureNotifyMask);

    while (true) {
        XEvent ev;
        XNextEvent(display, &ev);
        
        switch (ev.type) {
            case CreateNotify:
                std::cout << "Window created: " << ev.xcreatewindow.window << '\n';
                Fl::awake(refresh);
                break;
                
            
            case DestroyNotify:
                std::cout << "Window destroyed: " << ev.xdestroywindow.window << '\n';
                Fl::awake(refresh);
                break;
        }
    }

    XCloseDisplay(display);
    return 0;
}

void initlauncher(){ 
	fillApplications(uapps);
}
#pragma endregion Launcher

#pragma region usb

#include <libudev.h>
#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <cstdlib>
#include <pwd.h>

static std::string basenameDev(const std::string& devnode) {
    // devnode like /dev/sdb1 -> sdb1
    if (devnode.rfind("/dev/", 0) == 0) return devnode.substr(5);
    return devnode;
}

static bool pmountDevice(const std::string& devnode, const std::string& name = "") {
    // If name is provided, pmount will use /media/<name>
    std::string cmd = "pmount \"" + devnode + "\"";
    if (!name.empty()) cmd += " \"" + name + "\"";
    return system(cmd.c_str()) == 0;
}

static bool pumountDevice(const std::string& what) {
    // what can be /dev/sdXN or a /media/* mountpoint
    std::string cmd = "pumount \"" + what + "\"";
    return system(cmd.c_str()) == 0;
}

int listenusb() {
    struct udev* udev = udev_new();
    if (!udev) {
        std::cerr << "Can't create udev\n";
        return 1;
    }

    struct udev_monitor* mon = udev_monitor_new_from_netlink(udev, "udev");
    if (!mon) {
        std::cerr << "Can't create udev monitor\n";
        udev_unref(udev);
        return 1;
    }
    udev_monitor_filter_add_match_subsystem_devtype(mon, "block", nullptr);
    udev_monitor_enable_receiving(mon);

    int fd = udev_monitor_get_fd(mon);
    std::cout << "Waiting for USB storage events...\n";

    while (true) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);

        if (select(fd + 1, &fds, nullptr, nullptr, nullptr) > 0 && FD_ISSET(fd, &fds)) {
            struct udev_device* dev = udev_monitor_receive_device(mon);
            if (!dev) continue;

            const char* action_c = udev_device_get_action(dev);
            std::string action = action_c ? action_c : "";

            // Only handle add/remove
            if (action != "add" && action != "remove") {
                udev_device_unref(dev);
                continue;
            }

            // Ensure it's a USB block device
            struct udev_device* parent_usb =
                udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device");
            if (!parent_usb) {
                udev_device_unref(dev);
                continue;
            }

            // Only partitions, never whole disks
            const char* devtype = udev_device_get_devtype(dev);
            if (!devtype || std::string(devtype) != "partition") {
                udev_device_unref(dev);
                continue;
            }

            const char* devnode_c = udev_device_get_devnode(dev);
            if (!devnode_c) {
                udev_device_unref(dev);
                continue;
            }
            std::string devnode = devnode_c; // e.g., /dev/sdb1

            // Optional: only filesystem partitions
            const char* fs_usage = udev_device_get_property_value(dev, "ID_FS_USAGE");
            if (!fs_usage || std::string(fs_usage) != "filesystem") {
                udev_device_unref(dev);
                continue;
            }

            // Prefer label for clearer mountpoint under /media
            const char* label_enc = udev_device_get_property_value(dev, "ID_FS_LABEL_ENC");
            std::string mountName = label_enc && *label_enc ? label_enc : basenameDev(devnode);
            std::string mediaMountPoint = "/media/" + mountName;

            if (action == "add") {
                // Mount the partition device (not the disk)
                if (pmountDevice(devnode /*, mountName*/)) {
                    std::cout << "Mounted " << devnode << " at " << mediaMountPoint << "\n";
                } else {
                    std::cerr << "Failed to mount " << devnode << "\n";
                }
            } else if (action == "remove") {
                // Try to unmount by device; if that fails, try the media mountpoint
                if (pumountDevice(devnode)) {
                    std::cout << "Unmounted " << devnode << "\n";
                } else if (pumountDevice(mediaMountPoint)) {
                    std::cout << "Unmounted " << mediaMountPoint << "\n";
                } else {
                    std::cerr << "Failed to unmount " << devnode << " (or " << mediaMountPoint << ")\n";
                }
            }

            udev_device_unref(dev);
        }
    }

    // Unreachable in this loop, but keep for completeness
    udev_unref(udev);
    return 0;
}

#pragma endregion usb


#pragma region net


bool isWifiConnected() {
    const std::string interface = "wlp2s0"; // Change this to match your system
    const std::string cmd = "wpa_cli status";
    // const std::string cmd = "wpa_cli -i " + interface + " status";

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return false;

    char buffer[256];
    std::string result;

    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }

    pclose(pipe);

    return result.find("wpa_state=COMPLETED") != std::string::npos;
}


#if 0
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <dbus/dbus.h>
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <sstream>
#include "general.hpp"

#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <net/if.h>       // IFF_RUNNING
#include <linux/if_link.h> // ifinfomsg (no conflito com net/if.h)

#ifndef IFF_LOWER_UP
#define IFF_LOWER_UP 0x10000
#endif

#include <unistd.h>
#include <iostream>
#include <thread>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/wait.h>
#include <vector>

using namespace std;

void start_dhclient_with_dns(const std::string& dns) {
    const std::string conf_path = "/tmp/dhclient_custom.conf";

    // 1. Criar ficheiro de config temporário
    std::ofstream conf(conf_path);
    if (!conf) {
        std::cerr << "Erro ao criar config temporário\n";
        return;
    }
    conf << "supersede domain-name-servers " << dns << ";\n";
    conf.close();

    // 2. Criar filho
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        // Código do filho — executa o comando
        const char* argv[] = {
            "sudo",                    // precisa do path absoluto se "sudo" não estiver em /usr/bin
            "dhclient",
            "-1",
            "-nw",
            "-cf",
            conf_path.c_str(),
            nullptr
        };

        execvp("sudo", const_cast<char* const*>(argv));
        perror("execvp falhou");
        _exit(127);  // se exec falhar
    } else {
        // Código do pai — espera o filho terminar
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            std::cout << "dhclient saiu com código " << WEXITSTATUS(status) << "\n";
        } else {
            std::cerr << "dhclient terminou de forma inesperada\n";
        }
    }
}


void listen_netlink() {
    int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (sock < 0) {
        perror("socket");
        return;
    }

    sockaddr_nl addr{};
    addr.nl_family = AF_NETLINK;
    addr.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR;

    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(sock);
        return;
    }

    std::cout << "Listening for link changes...\n";

    char buf[4096];
    while (true) {
        int len = recv(sock, buf, sizeof(buf), 0);
        if (len < 0) continue;

        nlmsghdr *nh;
        for (nh = (nlmsghdr*)buf; NLMSG_OK(nh, len); nh = NLMSG_NEXT(nh, len)) {
            // if (nh->nlmsg_type == RTM_NEWLINK){
			// 	cotm("RTM_NEWLINK")

			// 	//implement only if dhclient not in execution
			// 	start_dhclient_with_dns("8.8.8.8");
			// }
			if (nh->nlmsg_type == RTM_NEWLINK) {
				struct ifinfomsg *ifi = (struct ifinfomsg*)NLMSG_DATA(nh);

				bool link_up = ifi->ifi_flags & IFF_RUNNING;
				bool physical_up = ifi->ifi_flags & IFF_LOWER_UP;

				cout<<"RTM_NEWLINK: ifindex = " << ifi->ifi_index
					<< " running=" << link_up
					<< " lower_up=" << physical_up<<endl;

				if (!link_up || !physical_up) {
					cotm("Link down → maybe disconnected");
					// Aqui podes matar dhclient se quiser, ou reagir
				} else {
					cotm("Link up → talvez reconectado");
					// Aqui podes iniciar dhclient, se não estiver já ativo
				}
			}
			if(nh->nlmsg_type == RTM_DELLINK) { 
				cotm("RTM_DELLINK")
            }
        }
    }

    close(sock);
}







// Globals
static DBusConnection* conn = nullptr;
static std::string iface_path;
static std::vector<std::string> bss_paths;
static std::vector<std::string> ssids;
static Fl_Browser* browser;


// Checa erro D-Bus e encerra se necessário
void check_error(DBusError &err) {
    if (dbus_error_is_set(&err)) {
        std::cerr << "D-Bus Error: " << err.name << " - " << err.message << std::endl;
        dbus_error_free(&err);
        exit(1);
    }
}



// Mata possíveis gerenciadores de Wi-Fi conflitantes
void kill_managers() {
    const char* services[] = {
        "systemctl stop wpa_supplicant",
        "killall wpa_supplicant",
		"systemctl stop NetworkManager", 
        "systemctl stop iwd",
        "killall wpa_supplicant",
        "killall iwd",
        "killall NetworkManager",
		"killall dhclient"
    };

    for (const auto& cmd : services) {
        std::system(cmd);
    }

    std::cout << "[INFO] WiFi services disabled (temporarily).\n";
    system("pkill -9 NetworkManager wpa_supplicant connman wicd 2>/dev/null");
    sleep(1);
}

// Inicia wpa_supplicant em modo D-Bus
void start_wpa_supplicant() {
    system("wpa_supplicant -u -s -O DIR=/run/wpa_supplicant GROUP=netdev &");
    sleep(1);
}

// Conecta ao barramento system D-Bus
void connect_dbus() {
    DBusError err;
    dbus_error_init(&err);
    conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
    check_error(err);
}

// Cria interface wlp2s0 via D-Bus
void create_interface(const char* ifname) {
    DBusError err;
    dbus_error_init(&err);

    DBusMessage* msg = dbus_message_new_method_call(
        "fi.w1.wpa_supplicant1",
        "/fi/w1/wpa_supplicant1",
        "fi.w1.wpa_supplicant1",
        "CreateInterface");
    if (!msg) {
        std::cerr << "Failed to create D-Bus message\n";
        exit(1);
    }

    DBusMessageIter args, dict, entry, variant;
    dbus_message_iter_init_append(msg, &args);
    dbus_message_iter_open_container(&args, DBUS_TYPE_ARRAY, "{sv}", &dict);

    // Ifname
    const char* key_if = "Ifname";
    dbus_message_iter_open_container(&dict, DBUS_TYPE_DICT_ENTRY, nullptr, &entry);
    dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key_if);
    dbus_message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "s", &variant);
    dbus_message_iter_append_basic(&variant, DBUS_TYPE_STRING, &ifname);
    dbus_message_iter_close_container(&entry, &variant);
    dbus_message_iter_close_container(&dict, &entry);

    // Driver
    const char* key_drv = "Driver";
    const char* driver = "nl80211";
    dbus_message_iter_open_container(&dict, DBUS_TYPE_DICT_ENTRY, nullptr, &entry);
    dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key_drv);
    dbus_message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "s", &variant);
    dbus_message_iter_append_basic(&variant, DBUS_TYPE_STRING, &driver);
    dbus_message_iter_close_container(&entry, &variant);
    dbus_message_iter_close_container(&dict, &entry);

    dbus_message_iter_close_container(&args, &dict);

    DBusMessage* reply = dbus_connection_send_with_reply_and_block(conn, msg, -1, &err);
    dbus_message_unref(msg);
    check_error(err);

    const char* path = nullptr;
    DBusMessageIter riter;
    if (dbus_message_iter_init(reply, &riter) &&
        dbus_message_iter_get_arg_type(&riter) == DBUS_TYPE_OBJECT_PATH) {
        dbus_message_iter_get_basic(&riter, &path);
    }
    dbus_message_unref(reply);

    if (!path) {
        std::cerr << "CreateInterface returned null path\n";
        exit(1);
    }
    iface_path = path;
    std::cout << "Interface created at: " << iface_path << std::endl;
}

// Dispara Scan via D-Bus
void trigger_scan() {
    DBusError err;
    dbus_error_init(&err);

    DBusMessage* msg = dbus_message_new_method_call(
        "fi.w1.wpa_supplicant1",
        iface_path.c_str(),
        "fi.w1.wpa_supplicant1.Interface",
        "Scan");

    if (!msg) {
        std::cerr << "Cannot create Scan message\n";
        exit(1);
    }

    DBusMessageIter args, dict, entry, variant;
    dbus_message_iter_init_append(msg, &args);
    dbus_message_iter_open_container(&args, DBUS_TYPE_ARRAY, "{sv}", &dict);

    // Type=active
    const char* key = "Type";
    const char* val = "active";
    dbus_message_iter_open_container(&dict, DBUS_TYPE_DICT_ENTRY, nullptr, &entry);
    dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key);
    dbus_message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "s", &variant);
    dbus_message_iter_append_basic(&variant, DBUS_TYPE_STRING, &val);
    dbus_message_iter_close_container(&entry, &variant);
    dbus_message_iter_close_container(&dict, &entry);

    dbus_message_iter_close_container(&args, &dict);

    DBusMessage* reply = dbus_connection_send_with_reply_and_block(conn, msg, -1, &err);
    dbus_message_unref(msg);
    check_error(err);
    if (reply) dbus_message_unref(reply);

    std::cout << "[INFO] Scan started...\n";
    sleep(2);
}

// Lê propriedade array of object paths "BSSs"
std::vector<std::string> get_bss_list() {
    std::vector<std::string> list;
    DBusError err;
    dbus_error_init(&err);

    DBusMessage* msg = dbus_message_new_method_call(
        "fi.w1.wpa_supplicant1",
        iface_path.c_str(),
        "org.freedesktop.DBus.Properties",
        "Get");
    if (!msg) exit(1);

    const char* iface = "fi.w1.wpa_supplicant1.Interface";
    const char* prop = "BSSs";
    DBusMessageIter args;
    dbus_message_iter_init_append(msg, &args);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &iface);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &prop);

    DBusMessage* reply = dbus_connection_send_with_reply_and_block(conn, msg, -1, &err);
    dbus_message_unref(msg);
    check_error(err);

    DBusMessageIter rit, var, arr;
    dbus_message_iter_init(reply, &rit);
    dbus_message_iter_recurse(&rit, &var);
    dbus_message_iter_recurse(&var, &arr);

    while (dbus_message_iter_get_arg_type(&arr) == DBUS_TYPE_OBJECT_PATH) {
        const char* path;
        dbus_message_iter_get_basic(&arr, &path);
        list.emplace_back(path);
        dbus_message_iter_next(&arr);
    }
    dbus_message_unref(reply);
    return list;
}

// Lê propriedade "SSID" de um BSS
std::string get_ssid(const std::string& bss_path) {
    DBusError err;
    dbus_error_init(&err);

    DBusMessage* msg = dbus_message_new_method_call(
        "fi.w1.wpa_supplicant1",
        bss_path.c_str(),
        "org.freedesktop.DBus.Properties",
        "Get");
    const char* iface = "fi.w1.wpa_supplicant1.BSS";
    const char* prop = "SSID";
    DBusMessageIter args;
    dbus_message_iter_init_append(msg, &args);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &iface);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &prop);

    DBusMessage* reply = dbus_connection_send_with_reply_and_block(conn, msg, -1, &err);
    dbus_message_unref(msg);
    check_error(err);

    DBusMessageIter rit, var, arr;
    dbus_message_iter_init(reply, &rit);
    dbus_message_iter_recurse(&rit, &var);
    dbus_message_iter_recurse(&var, &arr);

    std::string ssid;
    while (dbus_message_iter_get_arg_type(&arr) == DBUS_TYPE_BYTE) {
        uint8_t b;
        dbus_message_iter_get_basic(&arr, &b);
        ssid.push_back(static_cast<char>(b));
        dbus_message_iter_next(&arr);
    }
    dbus_message_unref(reply);
    return ssid;
}

// Callback para conectar à rede
void connect_to_network(const std::string& ssid, const std::string& password) {
    DBusError err;
    dbus_error_init(&err);

    // 1. Adiciona nova rede com propriedades
    DBusMessage* add_msg = dbus_message_new_method_call(
        "fi.w1.wpa_supplicant1",
        iface_path.c_str(),
        "fi.w1.wpa_supplicant1.Interface",
        "AddNetwork");

    DBusMessageIter args, dict, entry, variant, array_iter;
    dbus_message_iter_init_append(add_msg, &args);
    dbus_message_iter_open_container(&args, DBUS_TYPE_ARRAY, "{sv}", &dict);

    // Adiciona "ssid"
    const char* key_ssid = "ssid";
    dbus_message_iter_open_container(&dict, DBUS_TYPE_DICT_ENTRY, nullptr, &entry);
    dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key_ssid);
    dbus_message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "ay", &variant);
    dbus_message_iter_open_container(&variant, DBUS_TYPE_ARRAY, "y", &array_iter);
    for (char c : ssid) {
        uint8_t byte = c;
        dbus_message_iter_append_basic(&array_iter, DBUS_TYPE_BYTE, &byte);
    }
    dbus_message_iter_close_container(&variant, &array_iter);
    dbus_message_iter_close_container(&entry, &variant);
    dbus_message_iter_close_container(&dict, &entry);

    // Adiciona "psk"
    const char* key_psk = "psk";
    const char* psk_value = password.c_str();
    dbus_message_iter_open_container(&dict, DBUS_TYPE_DICT_ENTRY, nullptr, &entry);
    dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key_psk);
    dbus_message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "s", &variant);
    dbus_message_iter_append_basic(&variant, DBUS_TYPE_STRING, &psk_value);
    dbus_message_iter_close_container(&entry, &variant);
    dbus_message_iter_close_container(&dict, &entry);

    dbus_message_iter_close_container(&args, &dict);

    DBusMessage* add_reply = dbus_connection_send_with_reply_and_block(conn, add_msg, -1, &err);
    dbus_message_unref(add_msg);
    check_error(err);

    const char* net_path = nullptr;
    if (dbus_message_iter_init(add_reply, &args) &&
        dbus_message_iter_get_arg_type(&args) == DBUS_TYPE_OBJECT_PATH) {
        dbus_message_iter_get_basic(&args, &net_path);
    }
    std::string network_path = net_path ? net_path : "";
    dbus_message_unref(add_reply);

    if (network_path.empty()) {
        std::cerr << "Failed to create network path\n";
        return;
    }

    // 2. Seleciona a rede
    DBusMessage* sel_msg = dbus_message_new_method_call(
        "fi.w1.wpa_supplicant1",
        iface_path.c_str(),
        "fi.w1.wpa_supplicant1.Interface",
        "SelectNetwork");

    dbus_message_iter_init_append(sel_msg, &args);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_OBJECT_PATH, &network_path);

    DBusMessage* sel_reply = dbus_connection_send_with_reply_and_block(conn, sel_msg, -1, &err);
    dbus_message_unref(sel_msg);
    check_error(err);
    if (sel_reply) dbus_message_unref(sel_reply);

    std::cout << "Connecting to network: " << ssid << std::endl;
}

// Callback—ao clicar rede, pede senha e conecta
void browser_cb(Fl_Widget* w, void*) {
    int idx = browser->value();  // 1-based
    if (idx < 1 || idx > (int)ssids.size()) return;

    // Janela de senha
    Fl_Window* pwd_win = new Fl_Window(300,100,"Password");
    Fl_Input* input = new Fl_Input(10,10,280,25,"Password:");
    Fl_Button* btn = new Fl_Button(110,50,80,30,"Connect");
    pwd_win->end();
    pwd_win->show();

    btn->callback([](Fl_Widget* w, void* v) {
        Fl_Input* in = (Fl_Input*)v;
        std::string password = in->value();
        int idx = browser->value() - 1;
        std::string ssid = ssids[idx];

        // Fecha a janela de senha
        Fl_Window* win = (Fl_Window*)in->parent();
        win->hide();
        delete win;

        // Conecta à rede
        connect_to_network(ssid, password);

        // Mostra mensagem de confirmação
        Fl_Window* ok = new Fl_Window(200,80,"Connecting");
        Fl_Box* fb = new Fl_Box(20,20,160,20,"Connecting to network...");
        Fl_Button* bn = new Fl_Button(60,50,80,25,"OK");
        bn->callback([](Fl_Widget* w, void*) {

// sudo dhclient -1 -nw -cf <(echo 'supersede domain-name-servers 8.8.8.8;')
            w->parent()->hide();
            delete w->parent();
        });
        ok->end();
        ok->show();
    }, input);
}

// Função para executar comandos shell e capturar output
std::string exec_command(const char* cmd) {
    char buffer[256];
    std::string result = "";
    FILE* pipe = popen(cmd, "r");
    if (!pipe) return "error";
    while (fgets(buffer, sizeof(buffer), pipe)) {
        result += buffer;
    }
    pclose(pipe);
    return result;
}
std::string find_wifi_interface() {
    // Primeiro tenta com iw
    std::string iw_result = exec_command("iw dev | awk '/Interface/ {print $2}'");
    if (!iw_result.empty() && iw_result.find("error") == std::string::npos) {
        std::istringstream iss(iw_result);
        std::string iface;
        while (std::getline(iss, iface)) {
            if (!iface.empty() && iface.back() == '\n')
                iface.pop_back();
            if (!iface.empty()) return iface;
        }
    }
	return "wlan0";
}
int lnet() {
    // 1. Para outros gerenciadores e inicia o wpa_supplicant
    kill_managers();
    start_wpa_supplicant();

    // 2. Conecta D-Bus e cria interface
    connect_dbus(); 
    create_interface(find_wifi_interface().c_str());
    // create_interface("wlp2s0");

    // 3. Escaneia e obtém BSSs
    trigger_scan();
    sleep(3);
    bss_paths = get_bss_list();

    std::cout << "Found " << bss_paths.size() << " networks\n";
    
    // 4. Extrai SSIDs
    ssids.clear();
    for (auto &p : bss_paths) {
        ssids.push_back(get_ssid(p));
    }
	
	Fl_Group::current(nullptr); // important
    // 5. Monta UI FLTK
    Fl_Window* win = new Fl_Window(300, 400, "Wi-Fi Networks");
    browser = new Fl_Browser(10, 10, 280, 380);
    for (auto &s : ssids) browser->add(s.c_str());
    browser->type(FL_HOLD_BROWSER);
    browser->callback(browser_cb);
    win->end();
    win->show();

	std::thread listener(listen_netlink);
    listener.detach();

    // return Fl::run();
}
#endif

#pragma endregion net
#pragma region bottomright





#pragma endregion bottomright

int main() {
	std::cout << "FLTK Version: " << FL_VERSION << std::endl;
	std::cout << "FLTK Release: " << FL_RELEASE << std::endl;
	Fl::lock();
	XInitThreads(); 

	initlauncher();


    int bar_height = 24; // Adjust height as needed
    int screen_w = Fl::w();
    int screen_h = Fl::h();

    tasbwin = new Fl_Window(0, screen_h - bar_height, screen_w, bar_height);
    tasbwin->color(FL_DARK_RED);
    tasbwin->begin();
	
	btest=new Fl_Button(screen_w-320,0,100,24,"refresh ");
	btest->callback([](Fl_Widget* wid){
		Fl_Button* btn=(Fl_Button*)wid;
		// pid_t pid_=get_active_window_pid();
		// stringstream strm;
		// strm<<pid_;
		// btn->copy_label(strm.str().c_str());
		// lnet();


		// fg->begin();
		// refresh();
		// fg->end();
		// win->redraw();
	});
 
    Fl_Help_View* output = new Fl_Help_View(screen_w-220,0,220,24);
	output->scrollbar_size(-1); 
	// output->textfont(FL_HELVETICA_BOLD);
	output->textsize(14);
	// output->scrollbar_width(0);
	// output->wrap_mode(Fl_Text_Display::WRAP_NONE, 0);
    // output->textfont(FL_COURIER);
    // output->textsize(14);
    // output->readonly(1);





// MyOverlay* overlay=new MyOverlay(output->x(), output->y(),output->w(),output->h());

// Agora põe o botão por cima:
Fl_Button* btn = new Fl_Button (output->x(), output->y(),output->w(),output->h());
btn->box(FL_NO_BOX);  // Sem borda/fundo
	btn->callback([](Fl_Widget* wid){
		Fl_Button* btn=(Fl_Button*)wid;
		cotm("config") 
	});


// MixedFontWidget *widget = new MixedFontWidget(output->x()-400, output->y(),output->w(),output->h());







    fg=new Fl_Group(0,0,screen_w*.7,bar_height);
	fg->begin();
	refresh(0);  
	fg->end();


    tasbwin->end();
    
    tasbwin->border(0); // Remove window borders
    tasbwin->show();

	update_display(output);
    
    // Set X11 properties after window is shown
    set_dock_properties(tasbwin);

	// Prevent window from stealing focus
    // Display* dpy = fl_display;
    // Window xid = fl_xid(tasbwin);

    // XSetWindowAttributes attrs;
    // attrs.override_redirect = True;
    // XChangeWindowAttributes(dpy, xid, CWOverrideRedirect, &attrs);
    // XMapRaised(dpy, xid); // Show without focus
 


	thread([](){
		listenclipboard();
 	}).detach();

	thread([](){
		eventWindow();
 	}).detach();
	
	//windowskey+v and printscreen
	thread([](){
		listenkey();
 	}).detach();

	std::thread(listen_mouse_outside_window).detach(); 

	std::thread(listenusb).detach(); 

	std::thread(listen_fnvolumes).detach();
	std::thread(initscrensaver).detach();
    std::system("xset b off");


	cotm(uapps.size());

    return Fl::run();
}