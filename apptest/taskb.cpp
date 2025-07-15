#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Menu_Button.H> // For right-click menu
#include <FL/Fl_Timeout.H>     // For auto-hide timeout

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm> // For std::remove_if

// --- Global Xlib Variables (as before) ---
Display* display = nullptr;
Window root_window;

// --- Window Info Struct (as before) ---
struct WindowInfo {
    Window xid;
    std::string title;
    // Add more properties like pid, icon if needed
};

// --- Global Maps for FLTK and Xlib (as before) ---
std::map<Window, Fl_Button*> window_button_map;
std::map<Window, WindowInfo> current_windows; // To keep track of what's currently displayed

// Forward declaration for the timeout callback
void auto_hide_timeout_cb(void*);

// --- Helper Functions (as before) ---
std::string get_window_property_string(Display* dpy, Window w, Atom prop) { /* ... same as before ... */ return ""; }
void send_client_message(Display* dpy, Window w, Atom message_type, long data0, long data1, long data2, long data3, long data4) { /* ... same as before ... */ }

// --- FLTK Taskbar Window Class ---
class TaskbarWindow : public Fl_Window {
public:
    Fl_Group* window_buttons_group;
    Fl_Timeout_Handler hide_timeout_handler; // For auto-hide

    TaskbarWindow(int w, int h, const char* title) : Fl_Window(w, h, title) {
        // Set window type hint for Openbox (as a dock/panel)
        // This should be done before showing the window
        Atom net_wm_window_type = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
        Atom net_wm_window_type_dock = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DOCK", False);
        XChangeProperty(display, xid(), net_wm_window_type, XA_ATOM, 32, PropModeReplace,
                        (unsigned char*)&net_wm_window_type_dock, 1);

        // Optionally, set _NET_WM_STATE_STICKY if you want it to appear on all desktops
        // Atom net_wm_state = XInternAtom(display, "_NET_WM_STATE", False);
        // Atom net_wm_state_sticky = XInternAtom(display, "_NET_WM_STATE_STICKY", False);
        // XChangeProperty(display, xid(), net_wm_state, XA_ATOM, 32, PropModeAppend,
        //                 (unsigned char*)&net_wm_state_sticky, 1);

        // Set override_redirect if you want to bypass the window manager completely for positioning
        // This makes your window unmanaged, so Openbox won't decorate it or manage its position.
        // You'll have to manually position it perfectly.
        // XSetWindowAttributes attrs;
        // attrs.override_redirect = True;
        // XChangeWindowAttributes(display, xid(), CWOverrideRedirect, &attrs);

        // Set initial window position (e.g., bottom center)
        // You'll need to query screen dimensions
        int screen_width = XDisplayWidth(display, DefaultScreen(display));
        int screen_height = XDisplayHeight(display, DefaultScreen(display));
        position((screen_width - w) / 2, screen_height - h - 5); // 5 pixels from bottom edge

        begin();
        window_buttons_group = new Fl_Group(0, 0, w, h);
        window_buttons_group->type(FL_HORIZONTAL); // Horizontal for taskbar
        window_buttons_group->align(FL_ALIGN_LEFT | FL_ALIGN_TOP);
        window_buttons_group->spacing(5); // Space between buttons
        end();

        resizable(window_buttons_group); // Allow the group to resize with the window if needed
        color(FL_DARK3); // Simple background color

        // Add a callback for focus changes to detect when it loses focus
        callback(static_cast<Fl_Callback*>(&TaskbarWindow::focus_callback), this);
    }

    // Override handle to detect focus changes (FL_UNFOCUS event)
    int handle(int event) override {
        int ret = Fl_Window::handle(event);
        if (event == FL_UNFOCUS) {
            // Taskbar lost focus, initiate auto-hide/close
            Fl::add_timeout(0.2, auto_hide_timeout_cb, this); // Small delay to allow clicks
        } else if (event == FL_FOCUS) {
            // Taskbar gained focus, remove any pending auto-hide
            Fl::remove_timeout(auto_hide_timeout_cb, this);
        }
        return ret;
    }

    // Static callback for the window itself
    static void focus_callback(Fl_Widget* w, void* data) {
        // This callback is for the window's own events if you set it with callback()
        // The handle() method is generally better for low-level event interception like FL_UNFOCUS.
    }
};

// --- Auto-hide Timeout Callback ---
void auto_hide_timeout_cb(void* data) {
    TaskbarWindow* taskbar = static_cast<TaskbarWindow*>(data);
    if (!Fl::pushed()) { // Ensure no mouse button is currently down
        taskbar->hide();
        // Or exit the application if you want it to completely close
        // exit(0);
    } else {
        // If a button is being held down, re-arm the timeout after a short delay
        Fl::add_timeout(0.1, auto_hide_timeout_cb, taskbar);
    }
}


// --- Main Window List Update Function ---
void update_window_list() {
    // ... (Same Xlib code as before to get _NET_CLIENT_LIST) ...
    Atom net_client_list = XInternAtom(display, "_NET_CLIENT_LIST", False);
    Atom net_wm_name = XInternAtom(display, "_NET_WM_NAME", False);
    // Add _NET_WM_WINDOW_TYPE atom to filter out specific window types (like docks/desktops)
    Atom net_wm_window_type = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
    Atom net_wm_window_type_dock = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DOCK", False);
    Atom net_wm_window_type_desktop = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DESKTOP", False);
    Atom net_wm_window_type_utility = XInternAtom(display, "_NET_WM_WINDOW_TYPE_UTILITY", False);


    Atom actual_type;
    int actual_format;
    unsigned long nitems;
    unsigned long bytes_after;
    unsigned char* data = nullptr;

    if (XGetWindowProperty(display, root_window, net_client_list, 0, (~0L), False,
                           XA_WINDOW, &actual_type, &actual_format, &nitems,
                           &bytes_after, &data) != Success || !data) {
        // No windows found or error
        return;
    }

    Window* window_list_xids = reinterpret_cast<Window*>(data);
    std::vector<WindowInfo> new_window_infos;

    for (unsigned int i = 0; i < nitems; ++i) {
        Window xid = window_list_xids[i];
        std::string title = get_window_property_string(display, xid, net_wm_name);
        if (title.empty()) {
            title = get_window_property_string(display, xid, XA_WM_NAME);
        }

        // --- Filter out unwanted windows ---
        Atom* window_types = nullptr;
        unsigned long num_types = 0;
        if (XGetWindowProperty(display, xid, net_wm_window_type, 0, (~0L), False,
                               XA_ATOM, &actual_type, &actual_format, &num_types,
                               &bytes_after, (unsigned char**)&window_types) == Success && window_types) {
            bool is_dock_or_desktop = false;
            for (unsigned long j = 0; j < num_types; ++j) {
                if (window_types[j] == net_wm_window_type_dock ||
                    window_types[j] == net_wm_window_type_desktop ||
                    window_types[j] == net_wm_window_type_utility) { // You might want to include utility windows or not
                    is_dock_or_desktop = true;
                    break;
                }
            }
            XFree(window_types);
            if (is_dock_or_desktop) continue; // Skip these windows
        }

        if (!title.empty()) { // Only add if it has a title and passed filters
            new_window_infos.push_back({xid, title});
        }
    }
    XFree(data);

    // --- Update FLTK GUI (as before, but within this triggered context) ---
    TaskbarWindow* taskbar_win = (TaskbarWindow*)Fl::first_window();
    if (!taskbar_win) return; // Should not happen if main() initializes it

    // Remove old buttons for windows that are no longer open
    // Use a set for faster lookup of present XIDs
    std::set<Window> present_xids;
    for (const auto& info : new_window_infos) {
        present_xids.insert(info.xid);
    }

    // Iterate through existing buttons and remove those whose XIDs are not in present_xids
    for (auto it = window_button_map.begin(); it != window_button_map.end(); ) {
        if (present_xids.find(it->first) == present_xids.end()) {
            taskbar_win->window_buttons_group->remove(it->second);
            delete it->second;
            it = window_button_map.erase(it);
        } else {
            ++it;
        }
    }

    // Add new buttons or update existing ones
    for (const auto& info : new_window_infos) {
        if (window_button_map.find(info.xid) == window_button_map.end()) {
            // New window, create button
            Fl_Button* button = new Fl_Button(0, 0, 120, 25, info.title.c_str());
            button->tooltip(info.title.c_str()); // Show full title on hover
            button->align(FL_ALIGN_CENTER | FL_ALIGN_WRAP);
            button->box(FL_THIN_UP_BOX); // Give it a subtle border

            // Right-click menu setup
            Fl_Menu_Button* menu = new Fl_Menu_Button(0, 0, 1, 1); // A dummy, hidden menu button
            menu->type(Fl_Menu_Button::POPUP1); // Make it a popup
            menu->add("Minimize", 0, [](Fl_Widget* w, void* data){
                Window xid = (Window)data;
                Atom net_wm_state = XInternAtom(display, "_NET_WM_STATE", False);
                Atom net_wm_state_hidden = XInternAtom(display, "_NET_WM_STATE_HIDDEN", False);
                send_client_message(display, xid, net_wm_state, 1, net_wm_state_hidden, 0, 0, 0); // _NET_WM_STATE_ADD
            }, (void*)info.xid);
            menu->add("Close", 0, [](Fl_Widget* w, void* data){
                Window xid = (Window)data;
                Atom wm_protocols = XInternAtom(display, "WM_PROTOCOLS", False);
                Atom wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
                send_client_message(display, xid, wm_protocols, wm_delete_window, CurrentTime, 0, 0, 0);
            }, (void*)info.xid);

            // Add "Pin to Taskbar" option - This needs persistent storage and logic
            menu->add("Pin to Taskbar", 0, [](Fl_Widget* w, void* data){
                Window xid = (Window)data;
                const WindowInfo* win_info = nullptr;
                // Find the WindowInfo based on xid
                for(const auto& pair : current_windows) {
                    if (pair.first == xid) {
                        win_info = &pair.second;
                        break;
                    }
                }
                if (win_info) {
                    // This is where you would save win_info->title and a command to launch the app
                    Fl::warning("Pinning '%s' - Logic for persistent storage and launch command needed.", win_info->title.c_str());
                }
            }, (void*)info.xid);

            button->callback([](Fl_Widget* w, void* data) {
                Window xid = (Window)((void**)data)[0]; // First element is XID
                Fl_Menu_Button* menu = (Fl_Menu_Button*)((void**)data)[1]; // Second is menu

                if (Fl::event_button() == FL_RIGHT_MOUSE) {
                    menu->popup();
                } else {
                    // Left click: Activate/Focus window
                    Atom net_active_window = XInternAtom(display, "_NET_ACTIVE_WINDOW", False);
                    send_client_message(display, root_window, net_active_window, xid, 0, 0, 0, 0);
                }
            }, (void*[]){(void*)info.xid, (void*)menu}); // Pass both XID and menu to callback

            taskbar_win->window_buttons_group->add(button);
            window_button_map[info.xid] = button;
            current_windows[info.xid] = info;
        } else {
            // Existing window, update its title if necessary
            Fl_Button* button = window_button_map[info.xid];
            if (std::string(button->label()) != info.title) {
                button->label(info.title.c_str());
                button->tooltip(info.title.c_str()); // Update tooltip too
            }
        }
    }
    taskbar_win->window_buttons_group->redraw();
    taskbar_win->window_buttons_group->init_sizes(); // Re-layout children
    taskbar_win->redraw();
}

// --- Main Application Entry Point ---
int main(int argc, char** argv) {
    display = XOpenDisplay(nullptr);
    if (!display) {
        Fl::error("Cannot open X display. Is your X server running?");
        return 1;
    }
    root_window = DefaultRootWindow(display);

    // Create the taskbar window (initially hidden)
    TaskbarWindow* taskbar = new TaskbarWindow(800, 30, "Openbox On-Demand Taskbar");
    // Do not call taskbar->show() here. It will be shown on demand.

    // 1. Initial detection of windows
    update_window_list();

    // 2. If no windows are open, or if you want to explicitly hide it when empty
    if (window_button_map.empty()) {
        // Maybe just exit if there's nothing to show
        delete taskbar; // Clean up
        XCloseDisplay(display);
        return 0;
    } else {
        // Show the taskbar if there are windows
        taskbar->show();
        Fl::add_timeout(5.0, auto_hide_timeout_cb, taskbar); // Auto-hide after 5 seconds
    }

    // The FLTK event loop will run. The taskbar will hide itself
    // when it loses focus or after the timeout.
    return Fl::run(); // This will block until the window is hidden/closed
}