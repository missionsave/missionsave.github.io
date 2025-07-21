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

// Global pipe for waking up FLTK's event loop
static int wakeup_pipe[2] = {-1, -1};

// Global pointer to the WindowManager instance
static class WindowManager* global_wm_instance = nullptr;

// Map to track client window IDs to their FLTK frame objects
std::map<Window, ClientFrame*> client_to_frame_map;

// ============================================================================
// ClientFrame Class
// ============================================================================
class ClientFrame : public Fl_Window {
private:
    Window client_xid;
    Fl_Box* title_box;
    Fl_Button* close_button;
    Fl_Button* maximize_button;
    int original_x, original_y, original_w, original_h;
    bool is_maximized = false;

    static void close_client_cb(Fl_Widget* w, void* data) {
        ClientFrame* frame = static_cast<ClientFrame*>(data);
        if (frame && frame->client_xid && fl_display) {
            Atom wm_protocols_atom = XInternAtom(fl_display, "WM_PROTOCOLS", False);
            Atom wm_delete_window_atom = XInternAtom(fl_display, "WM_DELETE_WINDOW", False);

            if (wm_protocols_atom != None && wm_delete_window_atom != None) {
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

                        XSendEvent(fl_display, frame->client_xid, False, NoEventMask, (XEvent*)&xclient);
                        XFlush(fl_display);
                        return;
                    }
                }
            }
            frame->hide();
        }
    }

    static void maximize_toggle_cb(Fl_Widget* w, void* data) {
        ClientFrame* frame = static_cast<ClientFrame*>(data);
        if (!frame || !frame->client_xid || !fl_display) return;

        if (!frame->is_maximized) {
            frame->original_x = frame->x();
            frame->original_y = frame->y();
            frame->original_w = frame->w();
            frame->original_h = frame->h();

            int screen_num = XDefaultScreen(fl_display);
            int screen_width = XDisplayWidth(fl_display, screen_num);
            int screen_height = XDisplayHeight(fl_display, screen_num);

            frame->resize(0, 0, screen_width, screen_height);
            int client_x = 0;
            int client_y = frame->title_box->h();
            int client_w = frame->w();
            int client_h = frame->h() - frame->title_box->h();
            XMoveResizeWindow(fl_display, frame->client_xid, client_x, client_y, client_w, client_h);
            XFlush(fl_display);

            frame->is_maximized = true;
            frame->maximize_button->label("Restore");
        } else {
            frame->resize(frame->original_x, frame->original_y, frame->original_w, frame->original_h);
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
          original_x(x), original_y(y), original_w(w), original_h(h) {
        box(FL_THIN_UP_BOX);
        color(FL_LIGHT3);

        begin();
        Fl_Group* title_bar_group = new Fl_Group(0, 0, w, 24);
        title_bar_group->box(FL_FLAT_BOX);
        title_bar_group->color(FL_DARK_BLUE);
        title_bar_group->end();

        title_box = new Fl_Box(4, 2, w - 50, 20);
        title_box->labelfont(FL_BOLD);
        title_box->labelsize(12);
        title_box->labelcolor(FL_WHITE);
        title_box->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

        Fl_Pack* button_pack = new Fl_Pack(w - 70, 0, 70, 24);
        button_pack->type(Fl_Pack::HORIZONTAL);
        button_pack->spacing(2);
        button_pack->end();

        maximize_button = new Fl_Button(0, 0, 20, 20, "@square");
        maximize_button->callback(maximize_toggle_cb, this);
        maximize_button->tooltip("Maximize/Restore");

        close_button = new Fl_Button(0, 0, 20, 20, "@red_x");
        close_button->callback(close_client_cb, this);
        close_button->tooltip("Close Window");

        button_pack->add(maximize_button);
        button_pack->add(close_button);
        end();

        resizable(this);
        set_label_from_client();

        XReparentWindow(fl_display, client_xid, fl_xid(this), 0, title_bar_group->h());
        XAddToSaveSet(fl_display, client_xid);
        XResizeWindow(fl_display, client_xid, w, h - title_bar_group->h());
        XMapWindow(fl_display, client_xid);
        XSelectInput(fl_display, client_xid, PropertyChangeMask | StructureNotifyMask);
        show();
    }

    ~ClientFrame() {
        if (fl_display && client_xid) {
            XWindowAttributes attrs;
            if (XGetWindowAttributes(fl_display, client_xid, &attrs)) {
                XReparentWindow(fl_display, client_xid, XDefaultRootWindow(fl_display), x(), y());
                XRemoveFromSaveSet(fl_display, client_xid);
                XMapWindow(fl_display, client_xid);
            }
        }
    }

    Window get_client_xid() const { return client_xid; }

    void set_label_from_client() {
        if (!fl_display || !client_xid) return;

        char* wm_name = nullptr;
        char* net_wm_name = nullptr;

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

        if (!net_wm_name && XFetchName(fl_display, client_xid, &wm_name)) {
            title_box->copy_label(wm_name);
            XFree(wm_name);
        } else if (net_wm_name) {
            title_box->copy_label(net_wm_name);
            XFree(net_wm_name);
        } else {
            title_box->copy_label("Unnamed Client");
        }
        title_box->redraw();
    }

    void handle_configure_request(XEvent* event) {
        XConfigureRequestEvent* cre = &event->xconfigurerequest;
        XWindowChanges wc;
        wc.x = cre->x;
        wc.y = cre->y;
        wc.width = cre->width;
        wc.height = cre->height;
        wc.border_width = cre->border_width;
        wc.sibling = cre->above;
        wc.stack_mode = cre->detail;

        if (cre->value_mask & CWX) this->x(cre->x);
        if (cre->value_mask & CWY) this->y(cre->y);
        if (cre->value_mask & CWWidth) this->w(cre->width + 2);
        if (cre->value_mask & CWHeight) this->h(cre->height + title_box->h() + 2);

        int client_x_in_frame = 1;
        int client_y_in_frame = title_box->h() + 1;
        int client_w_in_frame = cre->width;
        int client_h_in_frame = cre->height;

        XConfigureWindow(fl_display, client_xid, cre->value_mask, &wc);
        XMoveResizeWindow(fl_display, client_xid, client_x_in_frame, client_y_in_frame, client_w_in_frame, client_h_in_frame);

        XEvent event_notify;
        event_notify.type = ConfigureNotify;
        event_notify.xconfigure.event = client_xid;
        event_notify.xconfigure.window = client_xid;
        event_notify.xconfigure.x = client_x_in_frame;
        event_notify.xconfigure.y = client_y_in_frame;
        event_notify.xconfigure.width = client_w_in_frame;
        event_notify.xconfigure.height = client_h_in_frame;
        event_notify.xconfigure.border_width = 0;
        event_notify.xconfigure.above = None;
        event_notify.xconfigure.override_redirect = False;

        XSendEvent(fl_display, client_xid, False, StructureNotifyMask, &event_notify);
        XFlush(fl_display);
        this->resize(x(), y(), w(), h());
    }

    int handle(int event) override {
        switch (event) {
            case FL_PUSH:
                if (Fl::event_button() == 1) {
                    XSetInputFocus(fl_display, client_xid, RevertToPointerRoot, CurrentTime);
                    XRaiseWindow(fl_display, fl_xid(this));
                    return 1;
                }
                break;
            case FL_UNFOCUS:
                redraw();
                return 1;
            case FL_FOCUS:
                redraw();
                return 1;
        }
        return Fl_Window::handle(event);
    }
};

// ============================================================================
// WindowManager Class
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
            return 0;
        }
        return 0;
    }

    bool has_wm_error() const { return wm_running_error; }

    void manage_client(Window client_xid) {
        if (client_xid == XDefaultRootWindow(fl_display)) return;
        if (client_to_frame_map.count(client_xid)) return;

        XWindowAttributes attrs;
        if (!XGetWindowAttributes(fl_display, client_xid, &attrs)) {
            std::cerr << "Failed to get attributes for window " << client_xid << std::endl;
            return;
        }

        if (attrs.override_redirect) {
            std::cout << "Ignoring override_redirect window: " << client_xid << std::endl;
            return;
        }

        int frame_x = attrs.x;
        int frame_y = attrs.y;
        int frame_w = attrs.width;
        int frame_h = attrs.height;

        if (frame_h < 24) frame_h = 24;
        if (frame_w < 50) frame_w = 50;

        ClientFrame* frame = new ClientFrame(client_xid, frame_x, frame_y, frame_w, frame_h);
        client_to_frame_map[client_xid] = frame;
    }

    void unmanage_client(Window client_xid) {
        if (client_to_frame_map.count(client_xid)) {
            ClientFrame* frame = client_to_frame_map[client_xid];
            client_to_frame_map.erase(client_xid);
            delete frame;
        }
    }

    void scan_existing_windows() {
        Window root = XDefaultRootWindow(fl_display);
        Window parent_ret, *children_ret;
        unsigned int num_children;

        if (XQueryTree(fl_display, root, &root, &parent_ret, &children_ret, &num_children)) {
            for (unsigned int i = 0; i < num_children; ++i) {
                XWindowAttributes attrs;
                if (XGetWindowAttributes(fl_display, children_ret[i], &attrs)) {
                    if (attrs.map_state == IsViewable && !attrs.override_redirect && attrs.depth > 0) {
                        manage_client(children_ret[i]);
                    }
                }
            }
            if (children_ret) XFree(children_ret);
        }
    }

    WindowManager() {
        if (!fl_display) {
            fl_alert("Error: X display not available!");
            exit(1);
        }

        XSetErrorHandler(wm_error_handler);
        XSelectInput(fl_display, XDefaultRootWindow(fl_display),
                     SubstructureRedirectMask | SubstructureNotifyMask |
                     PropertyChangeMask | ColormapChangeMask | EnterWindowMask);
        XSync(fl_display, False);

        XSetErrorHandler(nullptr);
        if (wm_running_error) {
            fl_alert("Another window manager is already running.");
            exit(1);
        }

        scan_existing_windows();
    }

    ~WindowManager() {
        for (auto const& [client_id, frame_ptr] : client_to_frame_map) {
            delete frame_ptr;
        }
        client_to_frame_map.clear();
    }
};

bool WindowManager::wm_running_error = false;

// ============================================================================
// X11 Event Handling
// ============================================================================
static void x11_event_callback(int fd, void*) {
    if (!fl_display) {
        std::cerr << "Error: No X display!" << std::endl;
        return;
    }

    XEvent event;
    while (XPending(fl_display)) {
        XNextEvent(fl_display, &event);
        std::cout << "Processing XEvent type: " << event.type << std::endl;
        
        switch (event.type) {
            case MapRequest: {
                XMapRequestEvent* mre = &event.xmaprequest;
                if (global_wm_instance) {
                    ((WindowManager*)global_wm_instance)->manage_client(mre->window);
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
            case ConfigureRequest: {
                XConfigureRequestEvent* cre = &event.xconfigurerequest;
                auto it = client_to_frame_map.find(cre->window);
                if (it != client_to_frame_map.end()) {
                    it->second->handle_configure_request(&event);
                } else {
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
                break;
            }
            case PropertyNotify: {
                XPropertyEvent* pe = &event.xproperty;
                if (pe->atom == XInternAtom(fl_display, "WM_NAME", False) ||
                    pe->atom == XInternAtom(fl_display, "_NET_WM_NAME", False)) {
                    auto it = client_to_frame_map.find(pe->window);
                    if (it != client_to_frame_map.end()) {
                        it->second->set_label_from_client();
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
    
    if (!fl_display) {
        std::cerr << "Failed to connect to X server!" << std::endl;
        return 1;
    }

    // Create pipe for event loop wakeup
    if (pipe(wakeup_pipe) == -1) {
        perror("pipe");
        return 1;
    }

    // First claim WM ownership
    Window root = XDefaultRootWindow(fl_display);
    XSetErrorHandler(WindowManager::wm_error_handler);
    XSelectInput(fl_display, root,
                SubstructureRedirectMask | SubstructureNotifyMask |
                PropertyChangeMask | ColormapChangeMask);
    XSync(fl_display, False);
    
    // Create WM instance and check for errors
    WindowManager wm;
    global_wm_instance = &wm;
    
    if (wm.has_wm_error()) {
        std::cerr << "Another WM is running!" << std::endl;
        return 1;
    }

    // Then setup FLTK
    Fl::add_fd(ConnectionNumber(fl_display), x11_event_callback);

    std::cout << "FLTK Window Manager running. Open applications to manage them." << std::endl;
    return Fl::run();
}