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

int main() {
	// Fl::add_handler(global_handler); // Install global handler
	// getlastfocus();
    int bar_height = 24; // Adjust height as needed
    int screen_w = Fl::w();
    int screen_h = Fl::h();

    win = new Fl_Window(0, screen_h - bar_height, screen_w, bar_height);
    win->color(FL_DARK_BLUE);
    win->begin();
        // Add your widgets/controls here
        Fl_Box* box = new Fl_Box(10, 5, 200, bar_height-10, "My Dock Bar");
        // box->color(FL_TRANSPARENT);
        box->labelsize(16);
        box->labelcolor(FL_WHITE);
        box->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
		
        Fl_Button* btn = new Fl_Button(0, 0, 50, bar_height);
		btn->callback([](Fl_Widget*){
            // This is the text you want to "paste"
			std::string text_to_paste = "This text was pasted from my custom taskbar!";

			winpop();
			return;

#ifdef __linux__
			if(0){
            Display* dpy = fl_display;
            Window xid = fl_xid(win); // Use your window's XID

            // Get the XA_PRIMARY selection atom (for immediate paste)
            // Or XA_CLIPBOARD for the more persistent clipboard
            Atom utf8_string = XInternAtom(dpy, "UTF8_STRING", False);
            Atom primary = XInternAtom(dpy, "PRIMARY", False); // Or XInternAtom(dpy, "CLIPBOARD", False);

            // Set the selection owner
            XSetSelectionOwner(dpy, primary, xid, CurrentTime);

            // Check if we successfully became the selection owner
            if (XGetSelectionOwner(dpy, primary) != xid) {
                fl_alert("Could not become selection owner.");
                return;
            }

            // Create an XEvent to simulate a paste request
            XEvent event;
            memset(&event, 0, sizeof(event));
            event.xselectionrequest.type = SelectionRequest;
            event.xselectionrequest.display = dpy;
            event.xselectionrequest.owner = xid;
            event.xselectionrequest.requestor = XDefaultRootWindow(dpy); // Request to the root window to simulate a paste
            event.xselectionrequest.selection = primary;
            event.xselectionrequest.target = utf8_string;
            event.xselectionrequest.property = None; // Let the client choose the property

            // Send the SelectionRequest event
            XSendEvent(dpy, XDefaultRootWindow(dpy), False, 
                       NoEventMask, (XEvent *)&event);
            XFlush(dpy);

            // The actual paste data needs to be provided when the target requests it.
            // This typically happens in a SelectionNotify event handler.
            // For a simple paste, you might need to directly send the keys.
            // However, a more robust solution involves handling SelectionNotify.

            // Alternative: Simulate key presses for Ctrl+V
            // This is a simpler but less "correct" way to paste.
            // It relies on the active window responding to Ctrl+V.
            XKeyEvent event_press;
            XKeyEvent event_release;

            memset(&event_press, 0, sizeof(event_press));
            memset(&event_release, 0, sizeof(event_release));

            Display* display = fl_display;
            Window root = XDefaultRootWindow(display);

            // Get the currently focused window
            Window focused_window;
            int revert_to;
            XGetInputFocus(display, &focused_window, &revert_to);

            // If no specific window is focused, send to root (might not work as expected)
            if (focused_window == PointerRoot || focused_window == None) {
                focused_window = root;
            }

            event_press.display = display;
            event_press.window = focused_window;
            event_press.root = root;
            event_press.subwindow = None;
            event_press.time = CurrentTime;
            event_press.x = 1; // Dummy values
            event_press.y = 1;
            event_press.x_root = 1;
            event_press.y_root = 1;
            event_press.same_screen = True;
            event_press.state = ControlMask; // Ctrl key pressed
            event_press.keycode = XKeysymToKeycode(display, XK_v); // 'v' key
            event_press.type = KeyPress;

            event_release = event_press;
            event_release.type = KeyRelease;

            // Simulate Ctrl+V key sequence
            XSendEvent(display, focused_window, True, KeyPressMask, (XEvent *)&event_press);
            XSendEvent(display, focused_window, True, KeyReleaseMask, (XEvent *)&event_release);
            XFlush(display);
            
            // Now, you need to set the selection data.
            // This part is crucial and often handled in a dedicated XEvent handler (SelectionRequest).
            // For this simplified example, we'll directly send the string as if it were part of a paste operation.
            // This is NOT the standard X11 selection protocol, but might work for simple cases.
            // A proper implementation would involve responding to SelectionRequest events from the target window.

            // To make the actual text available, you would typically own the selection
            // and then provide the data when another client requests it.
            // Since we're trying to *paste*, we're the *requestor*.
            // The simulation of Ctrl+V is the more direct way to trigger a paste from the existing clipboard.

            // If you want to *set* the clipboard content, you'd use something like this (simplified):
            Atom clipboard = XInternAtom(dpy, "CLIPBOARD", False);
            XSetSelectionOwner(dpy, clipboard, xid, CurrentTime);
            
            // To fulfill a paste request from another application, your application
            // needs to respond to SelectionRequest events. This is a more complex topic
            // involving event loop management.

            // For now, let's focus on triggering the paste of the *current* clipboard content
            // via simulated Ctrl+V, after ensuring our text is in the clipboard.

            // If you want to put `text_to_paste` into the clipboard and *then* paste it:
            // This requires your application to act as the selection owner and respond to requests.
            // For a complete clipboard manager, you'd implement a SelectionRequest handler.
            // For simplicity, we'll try to put the text directly into a selection and then trigger paste.

            // To put your `text_to_paste` into the PRIMARY selection:
            Atom selection = XInternAtom(dpy, "PRIMARY", False); // Or CLIPBOARD
            XSetSelectionOwner(dpy, selection, xid, CurrentTime);

            // The data itself isn't set directly here. It's provided when requested.
            // This means your application needs an event loop that listens for SelectionRequest.
            // FLTK handles its own event loop, so integrating a full X11 selection provider
            // might be more involved.

            // For immediate "paste" of a specific string, simulating key presses is often
            // the quickest, though less elegant, solution.

            // To actually "paste" `text_to_paste` into the active window, we need to
            // provide that text to the selection system. When the other application
            // requests a paste, it will send a SelectionRequest event to our window.
            // We would then respond with SelectionNotify and set the property on their window.

            // Since FLTK abstracts much of X11 event handling, directly implementing
            // a full selection provider can be tricky.

            // Let's refine the Ctrl+V approach to ensure our text is in the clipboard first.
            // This means we need to act as the clipboard owner temporarily.

            // To set the CLIPBOARD content:
            Atom ATOM_CLIPBOARD = XInternAtom(dpy, "CLIPBOARD", False);
            Atom ATOM_UTF8_STRING = XInternAtom(dpy, "UTF8_STRING", False);
            Atom ATOM_TARGETS = XInternAtom(dpy, "TARGETS", False);

            XSetSelectionOwner(dpy, ATOM_CLIPBOARD, xid, CurrentTime);
		}

            // This is where you would process SelectionRequest events.
            // FLTK's event loop might not easily expose this for a separate
            // X11 selection owner.

            // A simpler approach for setting the clipboard data (if you don't need
            // to be a long-term selection owner and just want to set it once):
            // This involves more low-level XSendEvent calls or using a library that
            // wraps selection handling. FLTK provides `Fl::copy()` and `Fl::paste()`,
            // but `Fl::copy()` sets the FLTK clipboard, not necessarily the X11 primary/clipboard.

            // **Revised Strategy: Simulate Ctrl+C of your text, then Ctrl+V**
            // This is still a bit of a hack but might be more reliable if you control
            // a temporary window for the "copy" action.
            // However, a direct programmatic way to set the X11 clipboard is better.

            // **Directly putting text into CLIPBOARD for pasting**
            // This is the most robust way. You need to implement an event handler
            // for SelectionRequest events on your window `xid`.

            // This part of the code would go into an X11 event loop or an FLTK handler
            // that processes raw XEvents. Since this is in a button callback, we need
            // to trigger the paste.

            // Option 1: Simulate Ctrl+V directly (assuming the text is already in the clipboard by some other means)
            // This is what the code above already does effectively. It sends Ctrl+V to the active window.

            // Option 2: Programmatically put text into CLIPBOARD and then simulate Ctrl+V.
            // This is more involved as it requires handling SelectionRequest.
            // For demonstration, let's just use a simplified version for setting the clipboard.

            // For setting the clipboard, you'd typically use `XConvertSelection` to request
            // the target window to convert the selection to a desired format (e.g., UTF8_STRING).
            // But here, *we* want to *be* the source of the paste.

            // Let's try to set the clipboard contents directly. This usually means becoming
            // the selection owner and providing the data on request.

            // This is a common pattern for setting clipboard content in X11:
            // 1. Become selection owner.
            // 2. Wait for SelectionRequest events.
            // 3. When a SelectionRequest for your selection (CLIPBOARD/PRIMARY) comes,
            //    and the target is a type you support (e.g., UTF8_STRING),
            //    set a property on the requestor's window with the data.
            //    Then send a SelectionNotify event.

            // FLTK doesn't expose the raw X11 event loop easily for this.
            // However, if you are running on a modern desktop environment, there
            // might be a simpler way using a utility or a high-level library.

            // **Simplest Approach (and what you're likely looking for):**
            // 1. Put your `text_to_paste` into the X11 CLIPBOARD selection.
            // 2. Send Ctrl+V key presses to the currently active window.

            // To put text into the CLIPBOARD selection without becoming a long-term
            // selection manager, you can use `XSetSelectionOwner` and then let a short-lived
            // dummy window manage the selection.

            // More directly for setting the clipboard in a short-lived context:
            // Create a temporary window that will own the selection.
            // This window won't be shown, just used for selection ownership.

            // The best way to set the X11 clipboard is by becoming the selection owner
            // and responding to SelectionRequest events. This is typically done in an event loop.

            // Given FLTK's abstraction, the most practical approach for a button callback
            // might be to use a system command if available, or simulate Ctrl+V
            // after somehow putting the desired text into the system clipboard.

            // Since you're using X11 directly, you can become the selection owner temporarily
            // and respond to the request. This requires a small event loop to wait for the request.
            // This is more complex for a simple button callback.

            // Let's go with the simulated Ctrl+V, assuming the text_to_paste is already
            // in the system clipboard (e.g., you manually copied it, or another part
            // of your app put it there).

            // To *put* text into the X11 CLIPBOARD for external use, a more complete example:
            //
            // static std::string selection_data;
            // static int selection_owner_window_id; // Store XID of the owner window
            //
            // // Event handler for SelectionRequest
            // int handle_selection_request(XEvent* event) {
            //     if (event->xany.window != selection_owner_window_id) return 0; // Not for our window
            //
            //     XSelectionRequestEvent *req = &(event->xselectionrequest);
            //     Display* dpy = req->display;
            //     Window requestor = req->requestor;
            //     Atom property = req->property;
            //     Atom target = req->target;
            //
            //     Atom ATOM_UTF8_STRING = XInternAtom(dpy, "UTF8_STRING", False);
            //     Atom ATOM_TARGETS = XInternAtom(dpy, "TARGETS", False);
            //     Atom ATOM_TEXT = XInternAtom(dpy, "TEXT", False); // Older text type
            //
            //     XEvent notify;
            //     memset(&notify, 0, sizeof(notify));
            //     notify.xselection.type = SelectionNotify;
            //     notify.xselection.display = dpy;
            //     notify.xselection.requestor = requestor;
            //     notify.xselection.selection = req->selection;
            //     notify.xselection.target = target;
            //     notify.xselection.time = req->time;
            //
            //     if (target == ATOM_TARGETS) {
            //         // Respond with supported targets
            //         Atom supported_targets[] = { ATOM_UTF8_STRING, ATOM_TEXT, ATOM_TARGETS };
            //         XChangeProperty(dpy, requestor, property, XA_ATOM, 32, PropModeReplace,
            //                         (unsigned char*)supported_targets, sizeof(supported_targets)/sizeof(Atom));
            //         notify.xselection.property = property;
            //     } else if (target == ATOM_UTF8_STRING || target == ATOM_TEXT) {
            //         // Provide the actual string data
            //         XChangeProperty(dpy, requestor, property, ATOM_UTF8_STRING, 8, PropModeReplace,
            //                         (unsigned char*)selection_data.c_str(), selection_data.length());
            //         notify.xselection.property = property;
            //     } else {
            //         notify.xselection.property = None; // Unsupported target
            //     }
            //
            //     XSendEvent(dpy, requestor, False, 0, &notify);
            //     return 1; // Handled
            // }
            //
            // // In your main loop, before Fl::run() or in a custom event handler:
            // // XSetWMProtocols(dpy, xid, &WM_DELETE_WINDOW, 1); // If not already set
            // // // Set Fl::handle to route events
            // // Fl::add_handler(handle_selection_request);
            //
            // // Inside the button callback to set the clipboard:
            // selection_data = text_to_paste;
            // selection_owner_window_id = xid;
            // Atom clipboard = XInternAtom(dpy, "CLIPBOARD", False);
            // XSetSelectionOwner(dpy, clipboard, xid, CurrentTime);
            //
            // // After setting the selection owner, trigger paste with Ctrl+V
            // // (This assumes the recipient will request the CLIPBOARD selection)
            // // ... (Ctrl+V simulation as below) ...

            // **For your current setup and simplicity, here's a combined approach:**
            // 1. Try to set the PRIMARY selection (often used for immediate paste with middle-click)
            // 2. Simulate Ctrl+V to trigger paste from the CLIPBOARD selection.
            //    This implies you need a way to get your text into the CLIPBOARD first.

            // Since FLTK has its own clipboard functions, a common approach for users
            // is to use `Fl::copy()` to put text into the FLTK clipboard, and then
            // `Fl::paste()` to get it out. However, `Fl::copy()` interacts with the
            // X11 PRIMARY selection and potentially the CLIPBOARD selection depending
            // on FLTK's internal X11 integration.

            // Let's leverage FLTK's clipboard mechanism as much as possible for `text_to_paste`.
            // Fl::copy() sets the X11 PRIMARY selection and potentially CLIPBOARD.
            Fl::copy(text_to_paste.c_str(), text_to_paste.length());

            // Now that the text is (hopefully) in the system clipboard, simulate Ctrl+V.
            // This is the part that will "paste" it into the active window.
            
            XKeyEvent event_press;
            XKeyEvent event_release;

            memset(&event_press, 0, sizeof(event_press));
            memset(&event_release, 0, sizeof(event_release));

            Display* display = fl_display;
            Window root = XDefaultRootWindow(display);

            // Get the currently focused window
            Window focused_window;
            int revert_to;
            XGetInputFocus(display, &focused_window, &revert_to);

            // If no specific window is focused or it's the root, try to get the active window
            // _NET_ACTIVE_WINDOW property on the root window
            if (focused_window == PointerRoot || focused_window == None) {
                Atom net_active_window = XInternAtom(display, "_NET_ACTIVE_WINDOW", False);
                Atom actual_type;
                int actual_format;
                unsigned long nitems, bytes_after;
                unsigned char *prop_return = NULL;
                Window active_window_id = None;

                if (XGetWindowProperty(display, root, net_active_window, 0, 1, False,
                                       XA_WINDOW, &actual_type, &actual_format,
                                       &nitems, &bytes_after, &prop_return) == Success && nitems > 0) {
                    active_window_id = ((Window*)prop_return)[0];
                    XFree(prop_return);
                    if (active_window_id != None) {
                        focused_window = active_window_id;
                    }
                }
            }
            
            // Fallback to root if still no suitable window
            if (focused_window == PointerRoot || focused_window == None) {
                focused_window = root;
            }

            event_press.display = display;
            event_press.window = focused_window;
            event_press.root = root;
            event_press.subwindow = None;
            event_press.time = CurrentTime;
            event_press.x = 1; 
            event_press.y = 1;
            event_press.x_root = 1;
            event_press.y_root = 1;
            event_press.same_screen = True;
            event_press.state = ControlMask; // Ctrl key pressed
            event_press.keycode = XKeysymToKeycode(display, XK_v); // 'v' key
            event_press.type = KeyPress;

            event_release = event_press;
            event_release.type = KeyRelease;

            // Simulate Ctrl+V key sequence
            XSendEvent(display, focused_window, True, KeyPressMask, (XEvent *)&event_press);
            XSendEvent(display, focused_window, True, KeyReleaseMask, (XEvent *)&event_release);
            XFlush(display);

#endif // __linux__
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