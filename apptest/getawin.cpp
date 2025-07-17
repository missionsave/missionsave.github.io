#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <iostream>

Window getActiveWindow(Display* display) {
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    unsigned char* prop = nullptr;

    Atom net_active_window = XInternAtom(display, "_NET_ACTIVE_WINDOW", True);
    if (net_active_window == None) return 0;

    if (XGetWindowProperty(display, DefaultRootWindow(display),
                           net_active_window, 0, (~0L), False,
                           AnyPropertyType, &actual_type, &actual_format,
                           &nitems, &bytes_after, &prop) != Success || !prop)
        return 0;

    Window active = *(Window*)prop;
    XFree(prop);
    return active;
}

int main() {
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        std::cerr << "Cannot open display\n";
        return 1;
    }

    Atom net_active_window = XInternAtom(display, "_NET_ACTIVE_WINDOW", True);
    Window root = DefaultRootWindow(display);

    // Listen for property changes on the root window
    XSelectInput(display, root, PropertyChangeMask);

    while (true) {
        XEvent event;
        XNextEvent(display, &event);

        if (event.type == PropertyNotify &&
            event.xproperty.atom == net_active_window) {
            Window focused = getActiveWindow(display);
            std::cout << "Focus changed to window: " << focused << std::endl;
        }
    }

    XCloseDisplay(display);
    return 0;
}
