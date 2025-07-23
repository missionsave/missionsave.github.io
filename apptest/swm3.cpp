//not working

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Pack.H> // For arranging buttons easily
#include <FL/fl_ask.H>   // For alerts
#include <FL/x.H>
#include <iostream>
#include <map>
#include <string>

// Xlib headers
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h> // For KeySym
// Required for X_ChangeWindowAttributes constant
#include <X11/Xproto.h> 

// Global Xlib Display pointer (managed by FLTK internally)
extern Display* fl_display;

// Forward declarations
class ClientFrame;
class WindowManager;

// Forward declaration for the Xlib event handler function
int XlibHandler(int event_type); // <--- ADDED THIS FORWARD DECLARATION

// Global pointer to the WindowManager instance
// This is somewhat hacky but common for simple single-instance WMs to allow XlibHandler access
static WindowManager* global_wm_instance = nullptr;

// A map to keep track of client window IDs to their FLTK frame objects
std::map<Window, ClientFrame*> client_to_frame_map;

// ============================================================================
// ClientFrame Class: The FLTK window that acts as a frame for a client app
// ============================================================================
class ClientFrame : public Fl_Window {
private:
    Window client_xid; // The XID of the actual client application window
    Fl_Box* title_box;
    Fl_Button* close_button;
    Fl_Button* maximize_button;

    // Store original client geometry for toggle maximize
    int original_x, original_y, original_w, original_h;
    bool is_maximized = false;

    // Private callback to close the client (from close_button)
    static void close_client_cb(Fl_Widget* w, void* data) {
        ClientFrame* frame = static_cast<ClientFrame*>(data);
        if (frame && frame->client_xid && fl_display) {
            // Send _NET_WM_PING (not implemented here) or _NET_CLOSE_WINDOW to client
            Atom wm_protocols_atom = XInternAtom(fl_display, "WM_PROTOCOLS", False);
            Atom wm_delete_window_atom = XInternAtom(fl_display, "WM_DELETE_WINDOW", False);

            if (wm_protocols_atom != None && wm_delete_window_atom != None) {
                // Check if client supports WM_DELETE_WINDOW
                Atom* protocols = nullptr;
                int count = 0;
                if (XGetWMProtocols(fl_display, frame->client_xid, &protocols, &count)) {
                    bool supports_delete = false;
                    for (int i = 0; i < count; ++i) {
                        if (protocols[i] == wm_delete_window_atom) {
                            supports_delete = true;
                            break;
                        }
                    }
                    if (protocols) XFree(protocols);

                    if (supports_delete) {
                        // Send ClientMessage to request client to close itself gracefully
                        XClientMessageEvent xclient;
                        xclient.type = ClientMessage;
                        xclient.window = frame->client_xid;
                        xclient.message_type = wm_protocols_atom;
                        xclient.format = 32;
                        xclient.data.l[0] = wm_delete_window_atom;
                        xclient.data.l[1] = CurrentTime;
                        xclient.data.l[2] = 0;
                        xclient.data.l[3] = 0;
                        xclient.data.l[4] = 0;

                        // Send the event to the client window
                        XSendEvent(fl_display, frame->client_xid, False, NoEventMask, (XEvent*)&xclient);
                        XFlush(fl_display);
                        std::cout << "Sent WM_DELETE_WINDOW to client: " << frame->client_xid << std::endl;
                        return; // Assume client will close, WM will get DestroyNotify
                    }
                }
            }
            // If graceful close fails or not supported, force kill (last resort, not recommended)
            // XKillClient(fl_display, frame->client_xid);
            // std::cerr << "WARNING: Force killing client " << frame->client_xid << std::endl;
        }
        // If client XID is somehow invalid or already gone, just hide the frame
        frame->hide();
    }

    static void maximize_toggle_cb(Fl_Widget* w, void* data) {
        ClientFrame* frame = static_cast<ClientFrame*>(data);
        if (!frame || !frame->client_xid || !fl_display) return;

        if (!frame->is_maximized) {
            // Store original geometry before maximizing
            frame->original_x = frame->x();
            frame->original_y = frame->y();
            frame->original_w = frame->w();
            frame->original_h = frame->h();

            // Get screen dimensions
            int screen_num = XDefaultScreen(fl_display);
            int screen_width = XDisplayWidth(fl_display, screen_num);
            int screen_height = XDisplayHeight(fl_display, screen_num);

            // Resize frame to full screen (adjust for panels/taskbars in a real WM)
            frame->resize(0, 0, screen_width, screen_height);
            // No frame->layout() needed here directly, Fl_Window's resize/redraw handles children

            // Resize client window to fill the frame content area
            int client_x = 0; // Relative to frame
            int client_y = frame->title_box->h(); // Below title bar
            int client_w = frame->w();
            int client_h = frame->h() - frame->title_box->h();
            XMoveResizeWindow(fl_display, frame->client_xid, client_x, client_y, client_w, client_h);
            XFlush(fl_display);

            frame->is_maximized = true;
            frame->maximize_button->label("Restore");
        } else {
            // Restore to original geometry
            frame->resize(frame->original_x, frame->original_y, frame->original_w, frame->original_h);
            // No frame->layout() needed here directly

            // Restore client window size based on frame's original size
            int client_x = 0;
            int client_y = frame->title_box->h();
            int client_w = frame->original_w;
            int client_h = frame->original_h - frame->title_box->h();
            XMoveResizeWindow(fl_display, frame->client_xid, client_x, client_y, client_w, client_h);
            XFlush(fl_display);

            frame->is_maximized = false;
            frame->maximize_button->label("Maximize");
        }
    }


public:
    ClientFrame(Window client_xid, int x, int y, int w, int h)
        : Fl_Window(x, y, w, h, ""), client_xid(client_xid),
          original_x(x), original_y(y), original_w(w), original_h(h)
    {
        box(FL_THIN_UP_BOX); // Frame border
        color(FL_LIGHT3); // Background color of the frame

        begin(); // Start adding widgets to the frame

        // Title bar area
        Fl_Group* title_bar_group = new Fl_Group(0, 0, w, 24); // Adjust height as needed
        title_bar_group->box(FL_FLAT_BOX);
        title_bar_group->color(FL_DARK_BLUE);
        title_bar_group->end();

        title_box = new Fl_Box(4, 2, w - 50, 20); // Title text
        title_box->labelfont(FL_BOLD);
        title_box->labelsize(12);
        title_box->labelcolor(FL_WHITE);
        title_box->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

        // Control buttons
        Fl_Pack* button_pack = new Fl_Pack(w - 70, 0, 70, 24); // Right-aligned pack
        button_pack->type(Fl_Pack::HORIZONTAL);
        button_pack->spacing(2);
        button_pack->end();

        maximize_button = new Fl_Button(0, 0, 20, 20, "@square"); // Unicode square for maximize
        maximize_button->callback(maximize_toggle_cb, this);
        maximize_button->tooltip("Maximize/Restore");

        close_button = new Fl_Button(0, 0, 20, 20, "@red_x"); // Unicode red X for close
        close_button->callback(close_client_cb, this);
        close_button->tooltip("Close Window");

        button_pack->add(maximize_button);
        button_pack->add(close_button);

        end(); // End adding widgets to the frame

        resizable(this); // Make the frame resizable
        set_label_from_client(); // Get initial title from client

        // Reparent the actual client window to this frame
        XReparentWindow(fl_display, client_xid, fl_xid(this), 0, title_bar_group->h());
        XAddToSaveSet(fl_display, client_xid); // Ensure client is restored if WM crashes

        // Set initial size of the client window within the frame
        XResizeWindow(fl_display, client_xid, w, h - title_bar_group->h());
        XMapWindow(fl_display, client_xid); // Make the client window visible

        // Select input for events on the client window (e.g., PropertyNotify for title changes)
        XSelectInput(fl_display, client_xid, PropertyChangeMask | StructureNotifyMask);

        show(); // Show the FLTK frame window
    }

    ~ClientFrame() {
        // When the frame is destroyed, if the client still exists, unparent it
        if (fl_display && client_xid) {
            // Check if client window still exists before unparenting
            XWindowAttributes attrs;
            if (XGetWindowAttributes(fl_display, client_xid, &attrs)) {
                // Reparent back to root if client still exists
                XReparentWindow(fl_display, client_xid, XDefaultRootWindow(fl_display), x(), y());
                XRemoveFromSaveSet(fl_display, client_xid);
                XMapWindow(fl_display, client_xid); // Show it again on root
            }
        }
    }

    Window get_client_xid() const { return client_xid; }

    void set_label_from_client() {
        if (!fl_display || !client_xid) return;

        char* wm_name = nullptr;
        char* net_wm_name = nullptr;

        // Try _NET_WM_NAME (EWMH, UTF-8) first
        Atom net_wm_name_atom = XInternAtom(fl_display, "_NET_WM_NAME", False);
        Atom utf8_string_atom = XInternAtom(fl_display, "UTF8_STRING", False);
        if (net_wm_name_atom != None && utf8_string_atom != None) {
            Atom actual_type;
            int actual_format;
            unsigned long nitems, bytes_after;
            unsigned char* prop_data = nullptr;
            if (XGetWindowProperty(fl_display, client_xid, net_wm_name_atom, 0, 1024, False,
                                   utf8_string_atom, &actual_type, &actual_format,
                                   &nitems, &bytes_after, &prop_data) == Success) {
                if (prop_data) {
                    net_wm_name = (char*)prop_data;
                }
            }
        }

        // Fallback to WM_NAME (ICCCM, potentially non-UTF8)
        if (!net_wm_name && XFetchName(fl_display, client_xid, &wm_name)) {
            title_box->copy_label(wm_name);
            XFree(wm_name);
            std::cout << "Client " << client_xid << " title (WM_NAME): " << title_box->label() << std::endl;
        } else if (net_wm_name) {
            title_box->copy_label(net_wm_name);
            XFree(net_wm_name); // Free property data
            std::cout << "Client " << client_xid << " title (_NET_WM_NAME): " << title_box->label() << std::endl;
        } else {
            title_box->copy_label("Unnamed Client");
            std::cout << "Client " << client_xid << " no title found." << std::endl;
        }
        title_box->redraw(); // Redraw the label
    }

    void handle_configure_request(XEvent* event) {
        XConfigureRequestEvent* cre = &event->xconfigurerequest; // Corrected: xconfigurerequest

        // Create a new XWindowChanges structure based on the request
        XWindowChanges wc;
        wc.x = cre->x;
        wc.y = cre->y;
        wc.width = cre->width;
        wc.height = cre->height;
        wc.border_width = cre->border_width;
        wc.sibling = cre->above;
        wc.stack_mode = cre->detail;

        // Apply changes to the frame window if they affect its position/size
        if (cre->value_mask & CWX) this->x(cre->x);
        if (cre->value_mask & CWY) this->y(cre->y);
        if (cre->value_mask & CWWidth) this->w(cre->width + 2); // Add border width (adjust as needed)
        if (cre->value_mask & CWHeight) this->h(cre->height + title_box->h() + 2); // Add titlebar and border (adjust as needed)

        // Configure the client window *within* its parent frame
        // This is crucial: the client's requested geometry is relative to its parent (the frame)
        // Its Y position needs to be offset by the title bar height
        int client_x_in_frame = 1; // Little padding
        int client_y_in_frame = title_box->h() + 1; // Below title bar + padding
        int client_w_in_frame = cre->width;
        int client_h_in_frame = cre->height;

        XConfigureWindow(fl_display, client_xid, cre->value_mask, &wc); // Apply client's requested changes
        XMoveResizeWindow(fl_display, client_xid, client_x_in_frame, client_y_in_frame, client_w_in_frame, client_h_in_frame);

        // Notify client of its new geometry
        XEvent event_notify;
        event_notify.type = ConfigureNotify;
        event_notify.xconfigure.event = client_xid;
        event_notify.xconfigure.window = client_xid;
        event_notify.xconfigure.x = client_x_in_frame;
        event_notify.xconfigure.y = client_y_in_frame;
        event_notify.xconfigure.width = client_w_in_frame;
        event_notify.xconfigure.height = client_h_in_frame;
        event_notify.xconfigure.border_width = 0; // WM manages border
        event_notify.xconfigure.above = None; // No sibling
        event_notify.xconfigure.override_redirect = False;

        XSendEvent(fl_display, client_xid, False, StructureNotifyMask, &event_notify);
        XFlush(fl_display);

        // Update FLTK frame geometry
        this->resize(x(), y(), w(), h());
        // No this->layout() needed here directly, Fl_Window's resize/redraw handles children
    }

    // Handle FLTK events for the frame itself (e.g., mouse clicks for moving)
    int handle(int event) override {
        switch (event) {
            case FL_PUSH:
                if (Fl::event_button() == 1) { // Left mouse button
                    // Start moving or resizing logic here
                    // For simplicity, this example doesn't implement full drag-to-move/resize
                    // A real WM would grab pointer/keyboard here and enter a loop.
                    std::cout << "ClientFrame clicked (FL_PUSH)" << std::endl;
                    XSetInputFocus(fl_display, client_xid, RevertToPointerRoot, CurrentTime);
                    XRaiseWindow(fl_display, fl_xid(this)); // Raise our frame window
                    return 1; // Event handled
                }
                break;
            case FL_DRAG:
                // Implement drag to move/resize here if FL_PUSH handled.
                break;
            case FL_UNFOCUS:
                std::cout << "ClientFrame lost focus" << std::endl;
                // Dim frame or similar visual feedback
                redraw();
                return 1;
            case FL_FOCUS:
                std::cout << "ClientFrame gained focus" << std::endl;
                // Highlight frame
                redraw();
                return 1;
        }
        return Fl_Window::handle(event);
    }
};


// ============================================================================
// WindowManager Class: The central controller for all managed windows
// ============================================================================
class WindowManager {
public:
    WindowManager() {
        if (!fl_display) {
            fl_alert("Error: X display not available!");
            exit(1);
        }

        // Try to become the window manager by selecting SubstructureRedirectMask
        // This fails if another WM is already running.
        XSetErrorHandler(wm_error_handler); // Set custom error handler for this crucial step
        XSelectInput(fl_display, XDefaultRootWindow(fl_display),
                     SubstructureRedirectMask | SubstructureNotifyMask |
                     PropertyChangeMask | ColormapChangeMask | EnterWindowMask);
        XSync(fl_display, False); // Force the error to happen immediately

        // Restore default error handler
        XSetErrorHandler(nullptr); // Restore FLTK's default or system default

        if (wm_running_error) {
            fl_alert("Another window manager is already running. Cannot start SimpleWM.");
            exit(1);
        }
        std::cout << "Successfully became the window manager!" << std::endl;

        // Scan for existing windows and manage them (important for starting WM after apps)
        scan_existing_windows();
    }

    ~WindowManager() {
        // Unmanage all clients gracefully if possible
        for (auto const& [client_id, frame_ptr] : client_to_frame_map) {
            delete frame_ptr; // This calls ClientFrame destructor which unparents
        }
        client_to_frame_map.clear();
        std::cout << "Window Manager exiting." << std::endl;
    }

    void manage_client(Window client_xid) {
        if (client_xid == XDefaultRootWindow(fl_display)) return; // Don't manage root
        if (client_to_frame_map.count(client_xid)) return; // Already managing

        XWindowAttributes attrs;
        if (!XGetWindowAttributes(fl_display, client_xid, &attrs)) {
            std::cerr << "Failed to get attributes for window " << client_xid << std::endl;
            return;
        }

        if (attrs.override_redirect) { // Don't manage override_redirect windows (e.g., tooltips, menus)
            std::cout << "Ignoring override_redirect window: " << client_xid << std::endl;
            return;
        }

        // Determine initial position and size of the frame
        // For simplicity, just use client's requested size and place it at 100,100
        int frame_x = attrs.x;
        int frame_y = attrs.y;
        int frame_w = attrs.width;
        int frame_h = attrs.height;

        // A minimal frame height is needed for the title bar, even if the client requests 0 height
        if (frame_h < 24) frame_h = 24; // Ensure at least enough space for title bar
        if (frame_w < 50) frame_w = 50; // Ensure enough space for buttons

        ClientFrame* frame = new ClientFrame(client_xid, frame_x, frame_y, frame_w, frame_h);
        client_to_frame_map[client_xid] = frame;
        std::cout << "Managed new client: " << client_xid << " with frame " << fl_xid(frame) << std::endl;
    }

    void unmanage_client(Window client_xid) {
        if (client_to_frame_map.count(client_xid)) {
            ClientFrame* frame = client_to_frame_map[client_xid];
            client_to_frame_map.erase(client_xid);
            delete frame; // This will destroy the FLTK window and unparent the client
            std::cout << "Unmanaged client: " << client_xid << std::endl;
        }
    }

    // Scans for existing windows when the WM starts
    void scan_existing_windows() {
        Window root = XDefaultRootWindow(fl_display);
        Window parent_ret, *children_ret;
        unsigned int num_children;

        if (XQueryTree(fl_display, root, &root, &parent_ret, &children_ret, &num_children)) {
            std::cout << "Scanning " << num_children << " existing windows..." << std::endl;
            for (unsigned int i = 0; i < num_children; ++i) {
                XWindowAttributes attrs;
                if (XGetWindowAttributes(fl_display, children_ret[i], &attrs)) {
                    // Only manage visible windows that are not override_redirect and are top-level
                    if (attrs.map_state == IsViewable && !attrs.override_redirect && attrs.depth > 0) {
                        manage_client(children_ret[i]);
                    }
                }
            }
            if (children_ret) XFree(children_ret);
        }
    }

private:
    // Error handler for when another WM is running
    static bool wm_running_error;
    static int wm_error_handler(Display* display, XErrorEvent* error) {
        // Check for BadAccess error on the root window indicating another WM
        if (error->error_code == BadAccess &&
            error->request_code == X_ChangeWindowAttributes && // Corrected: X_ChangeWindowAttributes
            error->resourceid == XDefaultRootWindow(display))
        {
            wm_running_error = true;
            return 0; // Suppress default error handler
        }
        // For any other X errors, we don't try to re-dispatch them as FLTK events here.
        // Returning 0 indicates the error was handled by this custom handler.
        return 0;
    }

    friend int XlibHandler(int event_type); // Allow XlibHandler to access WM methods
};

bool WindowManager::wm_running_error = false;


// ============================================================================
// Xlib Event Handler (registered with Fl::add_handler)
// ============================================================================
// This function will receive all X events that FLTK doesn't handle itself
// This is where the window manager logic processes raw X events.
int XlibHandler(int event_type) {
    // We are no longer checking 'event_type' directly against FL_EVENT_READ
    // because it seems to be undefined in your environment.
    // Fl::add_handler guarantees that this function is called for raw X events.
    // The actual event type is determined from the XEvent* itself.

    XEvent* event = (XEvent*)Fl::event_original(); // <--- Reverted to Fl::event_original()

    if (!event) return 0; // Ensure we have a valid event pointer

    switch (event->type) {
        case MapRequest: {
            XMapRequestEvent* mre = &event->xmaprequest;
            std::cout << "MapRequest for window: " << mre->window << std::endl;
            if (global_wm_instance) {
                global_wm_instance->manage_client(mre->window);
            }
            return 1; // Event handled
        }
        case DestroyNotify: {
            XDestroyWindowEvent* de = &event->xdestroywindow;
            std::cout << "DestroyNotify for window: " << de->window << std::endl;
            if (global_wm_instance) {
                global_wm_instance->unmanage_client(de->window);
            }
            return 1; // Event handled
        }
        case UnmapNotify: {
            XUnmapEvent* ue = &event->xunmap;
            // UnmapNotify can happen when a window is hidden, not necessarily destroyed.
            // A more robust WM might just hide its frame, not unmanage.
            // For this example, we'll let DestroyNotify handle actual cleanup.
            std::cout << "UnmapNotify for window: " << ue->window << std::endl;
            return 1; // Event handled
        }
        case ReparentNotify: {
            XReparentEvent* re = &event->xreparent;
            std::cout << "ReparentNotify: window " << re->window << " parent " << re->event << " -> " << re->parent << std::endl;
            // This event occurs when we reparent. Don't process it further here.
            return 1;
        }
        case ConfigureRequest: {
            XConfigureRequestEvent* cre = &event->xconfigurerequest; // Corrected: xconfigurerequest
            std::cout << "ConfigureRequest for window: " << cre->window << std::endl;
            auto it = client_to_frame_map.find(cre->window);
            if (it != client_to_frame_map.end()) {
                it->second->handle_configure_request(event);
            } else {
                // If it's a window we don't manage (e.g., override_redirect),
                // just pass the configure request to the window unchanged.
                XWindowChanges wc;
                wc.x = cre->x;
                wc.y = cre->y;
                wc.width = cre->width;
                wc.height = cre->height;
                wc.border_width = cre->border_width;
                wc.sibling = cre->above;
                wc.stack_mode = cre->detail;
                XConfigureWindow(fl_display, cre->window, cre->value_mask, &wc);
            }
            return 1; // Event handled
        }
        case PropertyNotify: {
            XPropertyEvent* pe = &event->xproperty;
            if (pe->atom == XInternAtom(fl_display, "WM_NAME", False) ||
                pe->atom == XInternAtom(fl_display, "_NET_WM_NAME", False))
            {
                auto it = client_to_frame_map.find(pe->window);
                if (it != client_to_frame_map.end()) {
                    it->second->set_label_from_client(); // Update title
                }
            }
            return 1; // Event handled
        }
        // You would handle many more X events here for a full WM:
        // EnterNotify, LeaveNotify, FocusIn, FocusOut, ButtonPress, ButtonRelease, MotionNotify, KeyPress, KeyRelease etc.
    }
    return 0; // Event not handled by our WM logic, let FLTK try.
}


// ============================================================================
// Main Application
// ============================================================================
int main(int argc, char** argv) {
    Fl::args(argc, argv);

    // Register our Xlib event handler BEFORE showing any FLTK windows
    // because some events (like MapRequest) need to be caught early.
    Fl::add_handler(XlibHandler);

    // Create the Window Manager instance
    WindowManager wm;
    global_wm_instance = &wm; // Set the global instance for XlibHandler

    // FLTK main loop
    std::cout << "FLTK Window Manager is running. Open some apps like xterm, firefox, etc." << std::endl;
    return Fl::run();
}