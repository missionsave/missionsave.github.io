#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Pack.H>
#include <FL/fl_ask.H>
#include <FL/x.H>
#include <iostream>
#include <map>
#include <string>
#include <unistd.h>

// Xlib headers
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/Xproto.h>

extern Display* fl_display;

// Forward declarations
class ClientFrame;
class WindowManager;

// Global pipe for waking up FLTK's event loop
static int wakeup_pipe[2] = {-1, -1};

// Global pointer to the WindowManager instance
static WindowManager* global_wm_instance = nullptr;

// Map to track client window IDs to their FLTK frame objects
std::map<Window, ClientFrame*> client_to_frame_map;

// ============================================================================
// ClientFrame Class (Fixed Implementation)
// ============================================================================
class ClientFrame : public Fl_Window {
private:
    Window client_xid;
    Fl_Box* title_box;
    Fl_Button* close_button;
    Fl_Button* maximize_button;
    int original_x, original_y, original_w, original_h;
    bool is_maximized = false;
    static const int TITLE_BAR_HEIGHT = 25;

    static void close_client_cb(Fl_Widget* w, void* data) {
        ClientFrame* frame = static_cast<ClientFrame*>(data);
        if (frame && frame->client_xid && fl_display) {
            // Send WM_DELETE_WINDOW message
            Atom wm_protocols = XInternAtom(fl_display, "WM_PROTOCOLS", False);
            Atom wm_delete = XInternAtom(fl_display, "WM_DELETE_WINDOW", False);
            
            XEvent xev;
            xev.type = ClientMessage;
            xev.xclient.window = frame->client_xid;
            xev.xclient.message_type = wm_protocols;
            xev.xclient.format = 32;
            xev.xclient.data.l[0] = wm_delete;
            xev.xclient.data.l[1] = CurrentTime;
            XSendEvent(fl_display, frame->client_xid, False, NoEventMask, &xev);
        }
    }

    static void maximize_toggle_cb(Fl_Widget* w, void* data) {
        ClientFrame* frame = static_cast<ClientFrame*>(data);
        if (!frame || !fl_display) return;

        if (!frame->is_maximized) {
            // Store original size
            frame->original_x = frame->x();
            frame->original_y = frame->y();
            frame->original_w = frame->w();
            frame->original_h = frame->h();

            // Maximize
            int scr_w = XDisplayWidth(fl_display, XDefaultScreen(fl_display));
            int scr_h = XDisplayHeight(fl_display, XDefaultScreen(fl_display));
            frame->resize(0, 0, scr_w, scr_h);
            
            // Resize client window
            XMoveResizeWindow(fl_display, frame->client_xid, 
                             0, TITLE_BAR_HEIGHT, 
                             scr_w, scr_h - TITLE_BAR_HEIGHT);
            frame->is_maximized = true;
            frame->maximize_button->label("Restore");
        } else {
            // Restore
            frame->resize(frame->original_x, frame->original_y, 
                         frame->original_w, frame->original_h);
            XMoveResizeWindow(fl_display, frame->client_xid,
                             0, TITLE_BAR_HEIGHT,
                             frame->original_w, frame->original_h - TITLE_BAR_HEIGHT);
            frame->is_maximized = false;
            frame->maximize_button->label("Maximize");
        }
        frame->redraw();
    }

public:
    ClientFrame(Window client_xid, int x, int y, int w, int h)
        : Fl_Window(x, y, w, h + TITLE_BAR_HEIGHT, ""), client_xid(client_xid),
          original_x(x), original_y(y), original_w(w), original_h(h) {
        
        // Configure frame window
        box(FL_FLAT_BOX);
        color(FL_LIGHT2);
        begin();
        
        // Title bar
        Fl_Group* title_bar = new Fl_Group(0, 0, w, TITLE_BAR_HEIGHT);
        title_bar->box(FL_FLAT_BOX);
        title_bar->color(FL_BLUE);
        
        title_box = new Fl_Box(10, 0, w-70, TITLE_BAR_HEIGHT);
        title_box->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
        title_box->labelcolor(FL_WHITE);
        
        // Control buttons
        Fl_Pack* buttons = new Fl_Pack(w-60, 0, 60, TITLE_BAR_HEIGHT);
        buttons->type(Fl_Pack::HORIZONTAL);
        buttons->spacing(2);
        
        maximize_button = new Fl_Button(0, 0, 20, TITLE_BAR_HEIGHT, "@square");
        maximize_button->callback(maximize_toggle_cb, this);
        
        close_button = new Fl_Button(0, 0, 20, TITLE_BAR_HEIGHT, "X");
        close_button->color(FL_RED);
        close_button->callback(close_client_cb, this);
        
        buttons->end();
        title_bar->end();
        
        // Client area
        Fl_Group* client_area = new Fl_Group(0, TITLE_BAR_HEIGHT, w, h);
        client_area->end();
        end();
        
        // Reparent and configure client window
        XReparentWindow(fl_display, client_xid, fl_xid(this), 0, TITLE_BAR_HEIGHT);
        XMoveResizeWindow(fl_display, client_xid, 0, TITLE_BAR_HEIGHT, w, h);
        XMapWindow(fl_display, client_xid);
        XSelectInput(fl_display, client_xid, StructureNotifyMask|PropertyChangeMask);
        
        // Set window title
        update_title();
        
        show();
    }

    void update_title() {
        if (!fl_display || !client_xid) return;
        
        char* name = nullptr;
        if (XFetchName(fl_display, client_xid, &name)) {
            title_box->copy_label(name);
            XFree(name);
        } else {
            title_box->label("Untitled");
        }
        redraw();
    }

    void handle_configure_request(XConfigureRequestEvent* cre) {
        XWindowChanges wc;
        wc.x = 0;
        wc.y = TITLE_BAR_HEIGHT;
        wc.width = cre->width;
        wc.height = cre->height;
        wc.border_width = 0;
        wc.sibling = cre->above;
        wc.stack_mode = cre->detail;
        
        // Resize our frame
        resize(x(), y(), cre->width, cre->height + TITLE_BAR_HEIGHT);
        
        // Configure client window
        XConfigureWindow(fl_display, client_xid, cre->value_mask, &wc);
        
        // Send synthetic configure notify
        XEvent ce;
        ce.xconfigure.type = ConfigureNotify;
        ce.xconfigure.event = client_xid;
        ce.xconfigure.window = client_xid;
        ce.xconfigure.x = 0;
        ce.xconfigure.y = TITLE_BAR_HEIGHT;
        ce.xconfigure.width = cre->width;
        ce.xconfigure.height = cre->height;
        ce.xconfigure.border_width = 0;
        ce.xconfigure.above = None;
        ce.xconfigure.override_redirect = False;
        XSendEvent(fl_display, client_xid, False, StructureNotifyMask, &ce);
    }

    Window get_client_xid() const { return client_xid; }

    int handle(int event) override {
        switch (event) {
            case FL_PUSH:
                if (Fl::event_button() == 1) {
                    // Handle window dragging
                    if (Fl::event_y() < TITLE_BAR_HEIGHT) {
                        XRaiseWindow(fl_display, fl_xid(this));
                        XSetInputFocus(fl_display, client_xid, RevertToPointerRoot, CurrentTime);
                        return 1;
                    }
                }
                break;
            case FL_DRAG:
                if (Fl::event_button() == 1 && Fl::event_y() < TITLE_BAR_HEIGHT) {
                    // Simple drag implementation
                    position(x() + Fl::event_dx(), y() + Fl::event_dy());
                    return 1;
                }
                break;
        }
        return Fl_Window::handle(event);
    }
};

// ============================================================================
// WindowManager Class (Fixed Implementation)
// ============================================================================
class WindowManager {
private:
    static bool wm_running_error;
    
public:
    static int wm_error_handler(Display* display, XErrorEvent* error) {
        if (error->error_code == BadAccess &&
            error->request_code == X_ChangeWindowAttributes &&
            error->resourceid == XDefaultRootWindow(display)) {
            wm_running_error = true;
        }
        return 0;
    }

    void manage_client(Window win) {
        if (client_to_frame_map.count(win)) return;
        
        XWindowAttributes attrs;
        if (!XGetWindowAttributes(fl_display, win, &attrs)) return;
        
        if (attrs.override_redirect) return;
        
        // Create frame with 10px offset from top-left
        static int offset = 10;
        ClientFrame* frame = new ClientFrame(win, offset, offset, 
                                           attrs.width, attrs.height);
        client_to_frame_map[win] = frame;
        offset = (offset + 30) % 100;
    }

    void unmanage_client(Window win) {
        auto it = client_to_frame_map.find(win);
        if (it != client_to_frame_map.end()) {
            delete it->second;
            client_to_frame_map.erase(it);
        }
    }

    WindowManager() {
        if (!fl_display) {
            fl_alert("No X display!");
            exit(1);
        }
        
        // Try to become WM
        XSetErrorHandler(wm_error_handler);
        XSelectInput(fl_display, XDefaultRootWindow(fl_display),
                    SubstructureRedirectMask|SubstructureNotifyMask|
                    StructureNotifyMask|PropertyChangeMask);
        XSync(fl_display, False);
        
        if (wm_running_error) {
            fl_alert("Another WM is running!");
            exit(1);
        }
        
        // Manage existing windows
        manage_existing_windows();
    }

    void manage_existing_windows() {
        Window root, parent, *children;
        unsigned int nchildren;
        
        if (XQueryTree(fl_display, XDefaultRootWindow(fl_display),
                      &root, &parent, &children, &nchildren)) {
            for (unsigned i = 0; i < nchildren; i++) {
                XWindowAttributes attrs;
                if (XGetWindowAttributes(fl_display, children[i], &attrs)) {
                    if (!attrs.override_redirect && attrs.map_state == IsViewable) {
                        manage_client(children[i]);
                    }
                }
            }
            if (children) XFree(children);
        }
    }

    bool has_wm_error() const { return wm_running_error; }
};

bool WindowManager::wm_running_error = false;

// ============================================================================
// X11 Event Handling
// ============================================================================
static void x11_event_callback(int fd, void*) {
    XEvent event;
    while (XPending(fl_display)) {
        XNextEvent(fl_display, &event);
        
        switch (event.type) {
            case MapRequest: {
                XMapRequestEvent* mre = &event.xmaprequest;
                if (global_wm_instance) {
                    ((WindowManager*)global_wm_instance)->manage_client(mre->window);
                    XMapWindow(fl_display, mre->window);
                }
                break;
            }
            case ConfigureRequest: {
                XConfigureRequestEvent* cre = &event.xconfigurerequest;
                auto it = client_to_frame_map.find(cre->window);
                if (it != client_to_frame_map.end()) {
                    it->second->handle_configure_request(cre);
                } else {
                    XWindowChanges wc = {
                        .x = cre->x,
                        .y = cre->y,
                        .width = cre->width,
                        .height = cre->height,
                        .border_width = cre->border_width,
                        .sibling = cre->above,
                        .stack_mode = cre->detail
                    };
                    XConfigureWindow(fl_display, cre->window, cre->value_mask, &wc);
                }
                break;
            }
            case DestroyNotify: {
                XDestroyWindowEvent* de = &event.xdestroywindow;
                if (global_wm_instance) {
                    ((WindowManager*)global_wm_instance)->unmanage_client(de->window);
                }
                break;
            }
            case PropertyNotify: {
                XPropertyEvent* pe = &event.xproperty;
                if (pe->atom == XInternAtom(fl_display, "WM_NAME", False)) {
                    auto it = client_to_frame_map.find(pe->window);
                    if (it != client_to_frame_map.end()) {
                        it->second->update_title();
                    }
                }
                break;
            }
        }
    }
}

// ============================================================================
// Main Function
// ============================================================================
int main(int argc, char** argv) {
    Fl::args(argc, argv);
    
	    Fl_Window *win = new Fl_Window(800, 600, "Simple WM");
    win->show();
	
    // Create pipe for event loop
    if (pipe(wakeup_pipe) == -1) {
        perror("pipe");
        return 1;
    }
    
    // Initialize WM
    WindowManager wm;
    global_wm_instance = &wm;
    
    // Set up X11 event handler
    Fl::add_fd(ConnectionNumber(fl_display), x11_event_callback);
    
    return Fl::run();
}