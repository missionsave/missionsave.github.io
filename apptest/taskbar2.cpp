//  g++ taskbar2.cpp ../common/general.cpp -I../common -o taskbar2 -l:libfltk.a -lX11 -lXtst -Os -s -flto=auto -std=c++20 -lXext -lXft -lXrender -lXcursor -lXinerama -lXfixes -lfontconfig -lfreetype -lz -lm -ldl -lpthread -lstdc++ -Os -w -Wfatal-errors -DNDEBUG -lasound  -lXss -lXi


// sudo apt install libasound2-dev acpid wmctrl

#pragma region  Includes
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

#include <chrono>

#include "general.hpp"


void dbg();
void refresh();
using namespace std;
#pragma endregion Includes
 
#pragma region  Globals
Fl_Window* tasbwin;
Fl_Group* fg;
vector<Fl_Button*> vbtn;
struct WindowInfo;
vector<WindowInfo> vwin;

#pragma endregion Globals

#pragma region  Dock
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

void update_display(Fl_Output* output) {
    std::string battery = read_file("/sys/class/power_supply/BAT0/capacity"); 
	// std::string status  = read_file("/sys/class/power_supply/BAT0/status");

    std::string clock   = get_time();

    output->value((""+battery + "% " + clock).c_str());

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


int listenkeyp1() {
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
#include <dirent.h>

// Obtem PID da janela ativa (foco) no X11
pid_t get_active_window_pid() {
    Display* display = XOpenDisplay(nullptr);
    if (!display) return -1;

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
        return -1;
    }

    if (XGetWindowProperty(display, active_window, net_wm_pid, 0, 1, False,
                           XA_CARDINAL, &actual_type, &actual_format,
                           &nitems, &bytes_after, &prop) == Success && prop) {
        pid_t pid = *(pid_t*)prop;
        XFree(prop);
        XCloseDisplay(display);
        return pid;
    }

    XCloseDisplay(display);
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

void initscrensaver() {
    constexpr int IDLE_THRESHOLD = 60*5; // segundos para desligar o ecrã
    int idle_seconds = 0;

    while (true) {
        unsigned long idle_ms = get_idle_time_ms();
        pid_t focused_pid = get_active_window_pid();

        if (idle_ms < 1000) {
            // Input recente do rato/teclado: reseta contador
            idle_seconds = 0;
        } else if (focused_pid == -1 || !is_audio_focused(focused_pid)) {
            // Sem foco ou áudio não relacionado com a janela ativa: incrementa contador
            idle_seconds++;
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

        std::this_thread::sleep_for(std::chrono::seconds(1));
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
};

unordered_map <string,string> uapps;
vector<string> pinnedApps={"Google Chrome","Visual Studio Code","tmux x86_64"};

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
	// Removes placeholders like %F, %U, %f, etc.
	std::string parseExecCommand(const std::string& execLine) {
		// return execLine;
		std::string cleaned = std::regex_replace(execLine, std::regex("%[fFuUdDnNickvm]"), "");
		return cleaned;
	}

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
void activateWindowById(const std::string& winId) {
	std::string cmd = "wmctrl -i -a " + winId;
	system(cmd.c_str());
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



void refresh(void*){ 
	fg->begin();
	int widthbtn=150;

	for (int i = 0; i < vbtn.size(); i++){
		vbtn[i]->hide();
		delete vbtn[i];
	}
	vbtn.clear();
	vwin.clear();

	//pinned
	perf();
	vector<WindowInfo> windows = getOpenWindowsInfo();
	perf("getwindows");
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
			string sexec = parseExecCommand(sexecp); 
			trim(sexec);
			// cotm(pinnedApps[ip] ,sexecp,sexec)
			string sexecf = findKeyByValue(uapps, sexec).value_or("");
			// string sexecf = findKeyByValue(uapps, sexecp).value_or("");

			if (pinnedApps[ip] == sexecf) {
				vwin.back().title = sexecf;
				// vwin.back().title = uapps[sexecf];
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
			string sexec = parseExecCommand(sexecp); 
			trim(sexec);
			string sexecf = findKeyByValue(uapps, sexecp).value_or("");

			if (sexec== sexecf) {
				vwin.push_back(WindowInfo());
				vwin.back().title = sexecf;
				// vwin.back().title = uapps[sexecf];
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


#pragma region  calculator

#pragma endregion calculator

int main() {
	Fl::lock();
	XInitThreads(); 

	initlauncher();


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
	refresh(0);  
	fg->end();


    tasbwin->end();
    
    tasbwin->border(0); // Remove window borders
    tasbwin->show();

	update_display(output);
    
    // Set X11 properties after window is shown
    set_dock_properties(tasbwin);


 


	thread([](){
		listenclipboard();
 	}).detach();

	thread([](){
		eventWindow();
 	}).detach();
	
	thread([](){
		listenkey();
 	}).detach();

	std::thread(listen_mouse_outside_window).detach(); 

	std::thread(listen_fnvolumes).detach();
	std::thread(initscrensaver).detach();
    std::system("xset b off");

    return Fl::run();
}