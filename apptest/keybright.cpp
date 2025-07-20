#include <X11/Xlib.h>
#include <X11/XF86keysym.h>
#include <cstdlib>
#include <iostream>

void increaseBrightness() {
    std::system("xrandr --output eDP1 --brightness 1.0");  // Replace eDP1 as needed
    std::cout << "Brightness Up\n";
}

void decreaseBrightness() {
    std::system("xrandr --output eDP1 --brightness 0.5");  // Example lower brightness
    std::cout << "Brightness Down\n";
}

void increaseVolume() {
    std::system("pactl set-sink-volume @DEFAULT_SINK@ +5%");
    std::cout << "Volume Up\n";
}

void decreaseVolume() {
    std::system("pactl set-sink-volume @DEFAULT_SINK@ -5%");
    std::cout << "Volume Down\n";
}

int main() {
    Display* dpy = XOpenDisplay(nullptr);
    if (!dpy) {
        std::cerr << "Cannot open display.\n";
        return 1;
    }

    Window root = DefaultRootWindow(dpy);

    // Grab multimedia keys
    int keys[] = {
        XF86XK_MonBrightnessUp,
        XF86XK_MonBrightnessDown,
        XF86XK_AudioRaiseVolume,
        XF86XK_AudioLowerVolume
    };

    for (int key : keys) {
        KeyCode kc = XKeysymToKeycode(dpy, key);
        XGrabKey(dpy, kc, AnyModifier, root, True, GrabModeAsync, GrabModeAsync);
    }

    XSelectInput(dpy, root, KeyPressMask);

    XEvent ev;
    while (true) {
        XNextEvent(dpy, &ev);
        if (ev.type == KeyPress) {
            KeySym ks = XKeycodeToKeysym(dpy, ev.xkey.keycode, 0);
            switch (ks) {
                case XF86XK_MonBrightnessUp:    increaseBrightness(); break;
                case XF86XK_MonBrightnessDown:  decreaseBrightness(); break;
                case XF86XK_AudioRaiseVolume:   increaseVolume(); break;
                case XF86XK_AudioLowerVolume:   decreaseVolume(); break;
            }
        }
    }

    XCloseDisplay(dpy);
    return 0;
}
