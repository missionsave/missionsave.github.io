// g++ dpmsoverride.cpp ../common/general.cpp -I../common -o dpmsoverride -l:libfltk.a -lX11 -lXtst -Os -s -flto=auto -std=c++20 -lXext -lXft -lXrender -lXcursor -lXinerama -lXfixes -lfontconfig -lfreetype -lz -lm -ldl -lpthread -lstdc++ -Os -w -Wfatal-errors -DNDEBUG -lasound 

#include <X11/Xlib.h>
#include <X11/extensions/dpms.h>
#include <unistd.h>
#include <iostream>
#include "general.hpp"

// int main() {
//     Display* dpy = XOpenDisplay(NULL);
//     if (!dpy) {
//         std::cerr << "Cannot open display\n";
//         return 1;
//     }

//     int dummy;
//     if (!DPMSQueryExtension(dpy, &dummy, &dummy)) {
//         std::cerr << "DPMS not supported\n";
//         return 1;
//     }
//                 // DPMSDisable(dpy);
//         // DPMSEnable(dpy);
//         // DPMSForceLevel(dpy, DPMSModeOff);



#include <X11/Xlib.h>
#include <X11/extensions/dpms.h>
#include <unistd.h>
#include <iostream>

int main() {
    Display* dpy = XOpenDisplay(nullptr);
    if (!dpy) {
        std::cerr << "Failed to open X display.\n";
        return 1;
    }

    int dummy;
    if (!DPMSQueryExtension(dpy, &dummy, &dummy)) {
        std::cerr << "DPMS extension not available.\n";
        return 1;
    }

    Window root = DefaultRootWindow(dpy);
    // Request keyboard and mouse motion events
    XGrabKey(dpy, AnyKey, AnyModifier, root, True,
         GrabModeAsync, GrabModeSync);

    time_t lastActivity = time(nullptr);

    while (true) {
        while (XPending(dpy)) {
            XEvent ev;
            XNextEvent(dpy, &ev);
            if (ev.type == KeyPress || ev.type == MotionNotify) {
                lastActivity = time(nullptr);
                // DPMSEnable(dpy); // Make sure DPMS is on
                // DPMSForceLevel(dpy, DPMSModeOn); // Wake screen if needed
                cotm("Activity detected → screen ON");
            }
			// In your event loop:
XAllowEvents(dpy, ReplayKeyboard, CurrentTime);
XFlush(dpy);
        }

        // Turn off screen after 60 seconds of inactivity
        if (time(nullptr) - lastActivity > 60) {
            DPMSEnable(dpy);
            DPMSForceLevel(dpy, DPMSModeOff);
            std::cout << "No activity → screen OFF\n";
            lastActivity = time(nullptr); // Avoid repeated blanking
        }

        usleep(500000); // Sleep 0.5s for responsiveness
    }

    XCloseDisplay(dpy);
    return 0;
}
