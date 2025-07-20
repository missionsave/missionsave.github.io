#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <iostream>
#include <string>

std::string getWindowTitle(Display* display, Window window) {
    Atom atomName = XInternAtom(display, "_NET_WM_NAME", True);
    Atom actualType;
    int actualFormat;
    unsigned long nItems, bytesAfter;
    unsigned char* prop = nullptr;

    if (atomName != None &&
        Success == XGetWindowProperty(display, window, atomName, 0, (~0L), False,
                                      AnyPropertyType, &actualType, &actualFormat,
                                      &nItems, &bytesAfter, &prop)) {
        if (prop) {
            std::string title(reinterpret_cast<char*>(prop), nItems);
            XFree(prop);
            return title;
        }
    }

    // Fallback to WM_NAME
    char* name = nullptr;
    if (XFetchName(display, window, &name) && name) {
        std::string title(name);
        XFree(name);
        return title;
    }

    return "";
}

int main() {
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        std::cerr << "Cannot open display\n";
        return 1;
    }

    Window root = DefaultRootWindow(display);
    Window parent;
    Window* children;
    unsigned int nchildren;

    if (XQueryTree(display, root, &root, &parent, &children, &nchildren)) {
        for (unsigned int i = 0; i < nchildren; ++i) {
            std::string title = getWindowTitle(display, children[i]);
            if (!title.empty()) {
                std::cout << "Window ID: " << children[i]
                          << "  Title: " << title << "\n";
            }
        }
        XFree(children);
    }

    XCloseDisplay(display);
    return 0;
}
