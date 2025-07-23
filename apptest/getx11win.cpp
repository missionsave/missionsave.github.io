#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <iostream>
#include <X11/Xlib.h>
#include <vector>
#include "general.hpp"
 
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <vector>  
#include <string>
#include <cstring>
using namespace std;

struct Windows {
    Window window_id;
    std::string title;
    int pid;
    std::string appname;
    bool isactive = false;
}; 
vector<Windows> vws;

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <vector>

bool is_top_level_window(Display* dpy, Window win) {
    // 1. Check if the window is visible
    XWindowAttributes attr;
    if (!XGetWindowAttributes(dpy, win, &attr) || attr.map_state != IsViewable) {
        return false;
    }

    // 2. Check WM_STATE (ICCCM-compliant windows)
    Atom wm_state = XInternAtom(dpy, "WM_STATE", False);
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    unsigned char* prop = nullptr;

    if (XGetWindowProperty(dpy, win, wm_state, 0, 0, False, AnyPropertyType,
                          &actual_type, &actual_format, &nitems, &bytes_after, &prop) != Success) {
        return false;
    }
    if (prop) XFree(prop);

    // 3. Check _NET_WM_WINDOW_TYPE to exclude docks, toolbars, etc.
    Atom net_wm_window_type = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
    Atom net_wm_window_type_normal = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_NORMAL", False);

    if (XGetWindowProperty(dpy, win, net_wm_window_type, 0, (~0L), False, XA_ATOM,
                          &actual_type, &actual_format, &nitems, &bytes_after, &prop) == Success && prop) {
        bool is_normal_window = false;
        Atom* types = reinterpret_cast<Atom*>(prop);
        for (unsigned long i = 0; i < nitems; i++) {
            if (types[i] == net_wm_window_type_normal) {
                is_normal_window = true;
                break;
            }
        }
        XFree(prop);
        if (!is_normal_window) {
            return false; // Not a "normal" window (might be a dock, toolbar, etc.)
        }
    }

    // 4. Check if the window is in _NET_CLIENT_LIST (optional, but ensures WM recognition)
    Atom net_client_list = XInternAtom(dpy, "_NET_CLIENT_LIST", False);
    Window* client_list = nullptr;
    unsigned long client_count = 0;

    if (XGetWindowProperty(dpy, DefaultRootWindow(dpy), net_client_list, 0, (~0L), False, XA_WINDOW,
                          &actual_type, &actual_format, &client_count, &bytes_after,
                          reinterpret_cast<unsigned char**>(&client_list)) == Success && client_list) {
        bool found = false;
        for (unsigned long i = 0; i < client_count; i++) {
            if (client_list[i] == win) {
                found = true;
                break;
            }
        }
        XFree(client_list);
        if (!found) {
            return false; // Window not in the WM's client list
        }
    }

    return true;
}

 

void get_top_level_windows(Display* dpy, std::vector<Window>& windows) {
    Window root = DefaultRootWindow(dpy);
    windows.clear();

    // 1. Try _NET_CLIENT_LIST first (modern WMs like GNOME/KDE)
    Atom net_client_list = XInternAtom(dpy, "_NET_CLIENT_LIST", False);
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    unsigned char* prop = nullptr;

    if (XGetWindowProperty(dpy, root, net_client_list, 0, (~0L), False, XA_WINDOW,
                         &actual_type, &actual_format, &nitems, &bytes_after, &prop) == Success && prop) {
        Window* client_list = reinterpret_cast<Window*>(prop);
        for (unsigned long i = 0; i < nitems; i++) {
            if (is_top_level_window(dpy, client_list[i])) {
                windows.push_back(client_list[i]);
            }
        }
        XFree(prop);
        return; // Successfully got windows from _NET_CLIENT_LIST
    }

    // 2. Fallback: XQueryTree (older WMs or if _NET_CLIENT_LIST fails)
    Window dummy_root, dummy_parent, *children = nullptr;
    unsigned int nchildren;

    if (XQueryTree(dpy, root, &dummy_root, &dummy_parent, &children, &nchildren)) {
        for (unsigned int i = 0; i < nchildren; ++i) {
            if (is_top_level_window(dpy, children[i])) {
                windows.push_back(children[i]);
            }
        }
        if (children) XFree(children);
    }
}

void getAllAttr(Display* dpy, Window win) {
    XWindowAttributes attr;
    if (XGetWindowAttributes(dpy, win, &attr)) {
        // std::cout << "Width: " << attr.width << "\n";
        // std::cout << "Height: " << attr.height << "\n";
        // std::cout << "X: " << attr.x << ", Y: " << attr.y << "\n";
        // std::cout << "Border width: " << attr.border_width << "\n";
        // std::cout << "Depth: " << attr.depth << "\n";
        // std::cout << "Map state: " << attr.map_state << "\n";
        // std::cout << "Override redirect: " << attr.override_redirect << "\n";
        // std::cout << "Visual ID: " << (attr.visual ? attr.visual->visualid : 0) << "\n";
    }

    Atom* props;
    int n;
    if (XListProperties(dpy, win, &n) && n > 0) {
        props = XListProperties(dpy, win, &n);
        std::cout << "Properties:\n";
        for (int i = 0; i < n; ++i) {
            char* name = XGetAtomName(dpy, props[i]);
            if (name) {
                std::cout << "  - " << name << "\n";
                XFree(name);
            }
        }
        XFree(props);
    }
}




std::string get_string_property(Display* dpy, Window win, const char* name) {
    Atom prop = XInternAtom(dpy, name, True);
    if (prop == None) return "";

    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    unsigned char* prop_ret = nullptr;

    if (XGetWindowProperty(dpy, win, prop, 0, (~0L), False, AnyPropertyType,
                           &actual_type, &actual_format,
                           &nitems, &bytes_after, &prop_ret) != Success || !prop_ret)
        return "";

    std::string result(reinterpret_cast<char*>(prop_ret));
    XFree(prop_ret);
    return result;
}

int get_pid(Display* dpy, Window win) {
    Atom pid_atom = XInternAtom(dpy, "_NET_WM_PID", True);
    if (pid_atom == None) return -1;

    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    unsigned char* prop_ret = nullptr;

    if (XGetWindowProperty(dpy, win, pid_atom, 0, 1, False, XA_CARDINAL,
                           &actual_type, &actual_format,
                           &nitems, &bytes_after, &prop_ret) != Success || !prop_ret)
        return -1;

    int pid = *(int*)prop_ret;
    XFree(prop_ret);
    return pid;
}

Window get_active_window(Display* dpy) {
    Atom prop = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", True);
    if (prop == None) return 0;

    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    unsigned char* prop_ret = nullptr;

    if (XGetWindowProperty(dpy, DefaultRootWindow(dpy), prop, 0, 1, False, AnyPropertyType,
                           &actual_type, &actual_format,
                           &nitems, &bytes_after, &prop_ret) != Success || !prop_ret)
        return 0;

    Window active = *(Window*)prop_ret;
    XFree(prop_ret);
    return active;
}

void get_visible_windowsp(Display* dpy, std::vector<Windows>& vws) {
    Window root = DefaultRootWindow(dpy);
    Window dummy_root, dummy_parent, *children = nullptr;
    unsigned int nchildren;

    Window active = get_active_window(dpy);

    if (XQueryTree(dpy, root, &dummy_root, &dummy_parent, &children, &nchildren)) {
        for (unsigned int i = 0; i < nchildren; ++i) {
            Window w = children[i];
            XWindowAttributes attr;
            if (!XGetWindowAttributes(dpy, w, &attr) || attr.map_state != IsViewable)
                continue;

            Atom wm_state = XInternAtom(dpy, "WM_STATE", False);
            Atom actual_type;
            int actual_format;
            unsigned long nitems, bytes_after;
            unsigned char* prop = nullptr;
            if (XGetWindowProperty(dpy, w, wm_state, 0, 0, False, AnyPropertyType,
                                   &actual_type, &actual_format, &nitems, &bytes_after, &prop) != Success)
                continue;
            if (prop) XFree(prop);

            Windows win;
            win.window_id = w;
            win.title = get_string_property(dpy, w, "_NET_WM_NAME");
            if (win.title.empty())
                win.title = get_string_property(dpy, w, "WM_NAME");

            win.pid = get_pid(dpy, w);
            win.appname = get_string_property(dpy, w, "WM_CLASS");
            win.isactive = (w == active);

            vws.push_back(win);
        }
        if (children) XFree(children);
    }
}


void set_active_window(Display* dpy, Window win) {
    Atom net_active_window = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False);
    Atom net_wm_window_type = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);

    XEvent e = {};
    e.xclient.type = ClientMessage;
    e.xclient.serial = 0;
    e.xclient.send_event = True;
    e.xclient.display = dpy;
    e.xclient.window = win;
    e.xclient.message_type = net_active_window;
    e.xclient.format = 32;
    e.xclient.data.l[0] = 1; // source indication: 1 = application
    e.xclient.data.l[1] = CurrentTime;
    e.xclient.data.l[2] = 0;
    e.xclient.data.l[3] = 0;
    e.xclient.data.l[4] = 0;

    XSendEvent(dpy, DefaultRootWindow(dpy), False,
               SubstructureNotifyMask | SubstructureRedirectMask, &e);
    XFlush(dpy);
}

#include <X11/Xutil.h>


// struct Windows {
//     Window   window_id;
//     std::string title;
//     int      pid;
//     std::string appname;
//     bool     isactive = false;
// };

// ——— Helpers ———

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

// UTF‑8 window title / WM_NAME
// std::string fetch_wm_name(Display* dpy, Window w) {
//     XTextProperty tp;
//     if (XGetWMName(dpy, w, &tp) && tp.value) {
//         std::string s((char*)tp.value, tp.nitems);
//         XFree(tp.value);
//         return s;
//     }
//     return "";
// }
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <string>
#include <cstring>
#include <vector>

std::string get_window_title(Display* dpy, Window win) {
    Atom net_wm_name = XInternAtom(dpy, "_NET_WM_NAME", False);
    Atom utf8_string = XInternAtom(dpy, "UTF8_STRING", False);
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    unsigned char* prop = nullptr;
    std::string title;

    // First try _NET_WM_NAME (UTF-8)
    if (net_wm_name != None &&
        XGetWindowProperty(dpy, win, net_wm_name, 0, (~0L), False, utf8_string,
                          &actual_type, &actual_format, &nitems, &bytes_after, &prop) == Success &&
        prop) {
        title = std::string(reinterpret_cast<char*>(prop), nitems);
        XFree(prop);
        return title;
    }

    // Try _NET_WM_NAME with any type
    if (net_wm_name != None &&
        XGetWindowProperty(dpy, win, net_wm_name, 0, (~0L), False, AnyPropertyType,
                          &actual_type, &actual_format, &nitems, &bytes_after, &prop) == Success &&
        prop) {
        if (actual_type == XA_STRING || actual_type == utf8_string) {
            title = std::string(reinterpret_cast<char*>(prop), nitems);
        }
        XFree(prop);
        if (!title.empty()) return title;
    }

    // Fallback to WM_NAME (legacy)
    prop = nullptr;
    if (XFetchName(dpy, win, reinterpret_cast<char**>(&prop)) && prop) {
        title = reinterpret_cast<char*>(prop);
        XFree(prop);
        return title;
    }

    // Final fallback: XGetWindowProperty for WM_NAME
    if (XGetWindowProperty(dpy, win, XA_WM_NAME, 0, (~0L), False, AnyPropertyType,
                          &actual_type, &actual_format, &nitems, &bytes_after, &prop) == Success &&
        prop) {
        if (actual_type == XA_STRING || actual_type == utf8_string) {
            title = std::string(reinterpret_cast<char*>(prop), nitems);
        }
        XFree(prop);
    }

    return title;
}
// WM_CLASS via XClassHint
std::string fetch_wm_class(Display* dpy, Window w) {
    XClassHint ch;
    if (XGetClassHint(dpy, w, &ch)) {
        std::string res = ch.res_class ? ch.res_class : "";
        if (ch.res_name) XFree(ch.res_name);
        if (ch.res_class) XFree(ch.res_class);
        return res;
    }
    return "";
}

// _NET_WM_PID (if set)
int fetch_wm_pid(Display* dpy, Window w) {
    Atom pid_atom = XInternAtom(dpy, "_NET_WM_PID", False);
    if (pid_atom == None) return -1;
    Atom actual_type;
    int actual_fmt;
    unsigned long nitems, after;
    unsigned char *data = nullptr;
    if (XGetWindowProperty(dpy, w, pid_atom, 0, 1, False, XA_CARDINAL,
                           &actual_type, &actual_fmt,
                           &nitems, &after, &data) == Success && data) {
        int pid = *(int*)data;
        XFree(data);
        return pid;
    }
    return -1;
}

// current focus window
Window current_focus(Display* dpy) {
    Window focus;
    int revert;
    XGetInputFocus(dpy, &focus, &revert);
    return focus;
}

// ——— The main function ———

void get_visible_windows(Display* dpy, std::vector<Windows>& out) {
    Window root = DefaultRootWindow(dpy);
    Window *kids = nullptr, dummy;
    unsigned int nkids;

    Window focused = current_focus(dpy);

    if (!XQueryTree(dpy, root, &dummy, &dummy, &kids, &nkids))
        return;

    for (unsigned int i = 0; i < nkids; ++i) {
        Window w = kids[i];

        // must be mapped (visible) and not an InputOnly
        XWindowAttributes a;
		// XGetWindowAttributes(dpy, w, &a);
// if (!XGetWindowAttributes(dpy, w, &a) ||
//     a.map_state != IsViewable ||
//     a.c_class == InputOnly)
//     continue;
		if(!is_top_level_window(dpy,w))continue;

        Windows W;
        W.window_id = w;
        W.title     = get_window_title(dpy, w);
        W.appname   = fetch_wm_class(dpy, w);
        W.pid       = get_pid_from_window(dpy, w);
        W.isactive  = (w == focused);

        out.push_back(std::move(W));
    }

    if (kids) XFree(kids);
}
void get_all_windows_recursive(Display* dpy, Window win, std::vector<Window>& windows) {
    if (is_top_level_window(dpy, win)) {
        windows.push_back(win);
    }

    Window dummy_root, dummy_parent, *children = nullptr;
    unsigned int nchildren;

    if (XQueryTree(dpy, win, &dummy_root, &dummy_parent, &children, &nchildren)) {
        for (unsigned int i = 0; i < nchildren; ++i) {
            get_all_windows_recursive(dpy, children[i], windows);
        }
        if (children) XFree(children);
    }
}

// Usage:
// std::vector<Window> all_windows;
// get_all_windows_recursive(dpy, DefaultRootWindow(dpy), all_windows);

int main() {
    Display* dpy = XOpenDisplay(nullptr);
    if (!dpy) return 1;

	std::vector<Window> vw;
    if (dpy) {
		perf();
        // get_visible_windows(dpy, vws); 
get_all_windows_recursive(dpy, DefaultRootWindow(dpy), vw);
        // get_top_level_windows(dpy, vw);

		perf("get_top_level_windows");
    }

	// lop(i,0,vw.size()){
	// 	cotm("Window ID",vw[i]);
	// 	getAllAttr(dpy, vw[i]);

	// }
	for (const auto& w : vws) {
		cotm(w.window_id, w.pid, w.isactive, w.title, w.appname);
	// printf("ID: 0x%lx, PID: %d, Active: %d\nTitle: %s\nApp: %s\n\n",
            //    w.window_id, w.pid, w.isactive,
            //    w.title.c_str(), w.appname.c_str());
    }
	cotm(vws.size());

    // Window root = DefaultRootWindow(dpy);
    // Window parent, *children;
    // unsigned int n;
    // if (XQueryTree(dpy, root, &root, &parent, &children, &n)) {
    //     for (unsigned int i = 0; i < n; ++i) {
    //         std::cout << "\nWindow ID: " << children[i] << "\n";
    //         getAllAttr(dpy, children[i]);
    //     }
    //     if (children) XFree(children);
    // }

    XCloseDisplay(dpy);
    return 0;
}
