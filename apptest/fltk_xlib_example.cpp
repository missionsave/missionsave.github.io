// g++ fltk_xlib_example.cpp -o fltk_xlib_example -lfltk -lX11

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/fl_ask.H> // For fl_alert
#include <FL/x.H> // X11-specific functions
#include <iostream>
#include <string>

// Xlib headers
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

// Global Xlib Display and Window for easier access (FLTK manages them internally)
Display* x_display = nullptr;
Window x_window_id = 0;

// Custom Fl_Window to handle focus events and provide XID
class MyWindow : public Fl_Window {
public:
    MyWindow(int W, int H, const char* L = 0) : Fl_Window(W, H, L) {
        // Get Xlib Display and Window ID after the window is shown
        // This must be done after show() because the XID is not available before
        callback(static_window_callback, this); // Use a static callback to access members
    }

    // Handle FLTK events, including focus
    int handle(int event) override {
        switch (event) {
            case FL_FOCUS:
                std::cout << "Window '" << label() << "' gained focus!" << std::endl;
                // You could update visual state here
                redraw(); // Request redraw to reflect focus change (e.g., border color)
                return 1; // Event handled
            case FL_UNFOCUS:
                std::cout << "Window '" << label() << "' lost focus!" << std::endl;
                // Update visual state
                redraw();
                return 1; // Event handled
            default:
                return Fl_Window::handle(event); // Let base class handle other events
        }
    }

    // Static callback to get XID and Display pointer once the window is shown
    static void static_window_callback(Fl_Widget* w, void* user_data) {
        MyWindow* self = static_cast<MyWindow*>(user_data);
        if (self->shown() && x_display == nullptr) { // Only do this once
            x_display = fl_display; // FLTK's global X Display pointer
            x_window_id = fl_xid(self); // FLTK's function to get XID for an Fl_Window
            if (x_display && x_window_id) {
                std::cout << "Xlib Display: " << x_display << ", Window ID: " << x_window_id << std::endl;
            }
        }
    }
};

// Function to set window title using Xlib
void set_x_window_title(const std::string& title) {
    if (x_display && x_window_id) {
        XStoreName(x_display, x_window_id, title.c_str());
        // Also set _NET_WM_NAME for EWMH compliance (UTF-8)
        Atom wm_name_atom = XInternAtom(x_display, "_NET_WM_NAME", False);
        Atom utf8_string_atom = XInternAtom(x_display, "UTF8_STRING", False);
        if (wm_name_atom != None && utf8_string_atom != None) {
            XChangeProperty(x_display, x_window_id, wm_name_atom, utf8_string_atom, 8,
                            PropModeReplace, (unsigned char*)title.c_str(), title.length());
        }
        XFlush(x_display); // Ensure the change is sent to the X server
        fl_alert("Window title updated to: \"%s\"", title.c_str());
    } else {
        fl_alert("Xlib Display or Window ID not available to set title.");
    }
}

// Function to send _NET_WM_STATE client message for maximize/minimize
void send_wm_state_message(Window target_window, long action, long property1, long property2 = 0) {
    if (!x_display || !target_window) return;

    XClientMessageEvent xclient;
    xclient.type = ClientMessage;
    xclient.serial = 0; // Not used
    xclient.send_event = True; // Propagate to children
    xclient.display = x_display;
    xclient.window = target_window; // Root window for _NET_WM_STATE
    
    xclient.message_type = XInternAtom(x_display, "_NET_WM_STATE", False);
    xclient.format = 32;
    xclient.data.l[0] = action; // _NET_WM_STATE_ADD, _NET_WM_STATE_REMOVE, _NET_WM_STATE_TOGGLE
    xclient.data.l[1] = property1; // E.g., _NET_WM_STATE_MAXIMIZED_VERT
    xclient.data.l[2] = property2; // E.g., _NET_WM_STATE_MAXIMIZED_HORIZ
    xclient.data.l[3] = 0; // Source indication: 0 for normal application, 1 for Pager, 2 for other
    xclient.data.l[4] = 0; // Unused

    XSendEvent(x_display, XDefaultRootWindow(x_display), False,
               SubstructureNotifyMask | SubstructureRedirectMask, (XEvent*)&xclient);
    XFlush(x_display);
}

// Function to send _NET_CLOSE_WINDOW client message
void send_close_message(Window target_window) {
    if (!x_display || !target_window) return;

    XClientMessageEvent xclient;
    xclient.type = ClientMessage;
    xclient.serial = 0;
    xclient.send_event = True;
    xclient.display = x_display;
    xclient.window = target_window;

    xclient.message_type = XInternAtom(x_display, "_NET_CLOSE_WINDOW", False);
    xclient.format = 32;
    xclient.data.l[0] = CurrentTime; // Timestamp of the user interaction
    xclient.data.l[1] = 0; // Unused
    xclient.data.l[2] = 0; // Unused
    xclient.data.l[3] = 0; // Unused
    xclient.data.l[4] = 0; // Unused

    XSendEvent(x_display, XDefaultRootWindow(x_display), False,
               SubstructureNotifyMask | SubstructureRedirectMask, (XEvent*)&xclient);
    XFlush(x_display);
}


// Callbacks for buttons
void set_title_cb(Fl_Widget*, void*) {
    set_x_window_title("My FLTK Xlib Window - New Title");
}

void toggle_maximize_cb(Fl_Widget*, void* user_data) {
    // You'd typically query the window's current state to know whether to add or remove
    // For simplicity, this example just toggles. A more robust app would get _NET_WM_STATE
    // property of the window and determine if it's currently maximized.
    
    // For simplicity, let's assume we toggle both horizontal and vertical maximize.
    // A better approach would be to check current state.
    // We send a _NET_WM_STATE_TOGGLE action.
    Atom max_vert = XInternAtom(x_display, "_NET_WM_STATE_MAXIMIZED_VERT", False);
    Atom max_horz = XInternAtom(x_display, "_NET_WM_STATE_MAXIMIZED_HORIZ", False);

    if (max_vert != None && max_horz != None) {
        send_wm_state_message(x_window_id, 2 /*_NET_WM_STATE_TOGGLE*/, max_vert, max_horz);
    } else {
        fl_alert("Could not get Maximize atoms.");
    }
}

void minimize_cb(Fl_Widget*, void* user_data) {
    Atom hidden_state = XInternAtom(x_display, "_NET_WM_STATE_HIDDEN", False);
    if (hidden_state != None) {
        send_wm_state_message(x_window_id, 1 /*_NET_WM_STATE_ADD*/, hidden_state);
    } else {
        fl_alert("Could not get Hidden state atom.");
    }
}

void close_cb(Fl_Widget*, void* user_data) {
    if (fl_choice("Are you sure you want to close this window?", "No", "Yes", 0) == 1) {
        send_close_message(x_window_id);
    }
}

int main(int argc, char** argv) {
    // Initialize FLTK
    Fl::args(argc, argv); // Process FLTK command line arguments if any

    MyWindow* window = new MyWindow(600, 400, "My FLTK Xlib Interaction Example");
    window->begin();

    Fl_Box* box = new Fl_Box(20, 20, 560, 50, "FLTK Window");
    box->box(FL_UP_BOX);
    box->labelfont(FL_BOLD + FL_ITALIC);
    box->labelsize(24);
    box->labelcolor(FL_DARK_BLUE);

    Fl_Button* title_btn = new Fl_Button(50, 100, 150, 30, "Set Xlib Title");
    title_btn->callback(set_title_cb);

    Fl_Button* maximize_btn = new Fl_Button(50, 150, 150, 30, "Toggle Maximize");
    maximize_btn->callback(toggle_maximize_cb);

    Fl_Button* minimize_btn = new Fl_Button(50, 200, 150, 30, "Minimize");
    minimize_btn->callback(minimize_cb);

    Fl_Button* close_btn = new Fl_Button(50, 250, 150, 30, "Close Window");
    close_btn->callback(close_cb);

    window->end();
    window->show(argc, argv);

    // After window->show(), the XID and Display are available through fl_xid and fl_display.
    // The static_window_callback will capture them.

    return Fl::run();
}