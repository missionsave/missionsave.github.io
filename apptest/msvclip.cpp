// gcc -O2 msvclip.cpp -o msvclip -lX11 -lXfixes && strip msvclip && pkill msvclip && sudo cp ./msvclip /usr/local/bin && msvclip &
#include <X11/Xlib.h>
#include <X11/extensions/Xfixes.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Stream-based prepending to keep RAM usage near zero
void save_to_file(const char* text, size_t len) {
    char path[512];
    snprintf(path, sizeof(path), "%s/msvclip.txt", getenv("HOME"));
    char temp_path[512];
    snprintf(temp_path, sizeof(temp_path), "%s.tmp", path);

    FILE* f_out = fopen(temp_path, "wb");
    if (!f_out) return;

    // Write new content
    fwrite(text, 1, len, f_out);
    fputc('\0', f_out);

    // Stream old content directly from disk to disk
    FILE* f_in = fopen(path, "rb");
    if (f_in) {
        char buffer[4096];
        size_t n;
        while ((n = fread(buffer, 1, sizeof(buffer), f_in)) > 0) {
            fwrite(buffer, 1, n, f_out);
        }
        fclose(f_in);
    }

    fclose(f_out);
    rename(temp_path, path);
}

void handle_selection(Display* display, Window window, Atom property) {
    Atom clipboard = XInternAtom(display, "CLIPBOARD", False);
    Atom utf8 = XInternAtom(display, "UTF8_STRING", False);

    XConvertSelection(display, clipboard, utf8, property, window, CurrentTime);
    XFlush(display);

    XEvent event;
    while (1) {
        XNextEvent(display, &event);
        if (event.type == SelectionNotify && event.xselection.selection == clipboard) {
            if (event.xselection.property == None) return;

            Atom actual_type;
            int actual_format;
            unsigned long n_items, bytes_after;
            unsigned char* data = NULL;

            // Read property (limiting to ~256KB per grab to prevent RAM spikes)
            XGetWindowProperty(display, window, property, 0, 65536, False,
                               AnyPropertyType, &actual_type, &actual_format,
                               &n_items, &bytes_after, &data);

            if (data) {
                if (n_items > 0) {
                    save_to_file((char*)data, n_items);
                }
                XFree(data);
            }
            XDeleteProperty(display, window, property);
            return;
        }
    }
}

int main() {
    Display* display = XOpenDisplay(NULL);
    if (!display) return 1;

    int event_base, error_base;
    if (!XFixesQueryExtension(display, &event_base, &error_base)) return 1;

    Window window = XCreateSimpleWindow(display, DefaultRootWindow(display), 0, 0, 1, 1, 0, 0, 0);
    Atom property = XInternAtom(display, "CLIPBOARD_PROPERTY", False);
    Atom clipboard = XInternAtom(display, "CLIPBOARD", False);

    XFixesSelectSelectionInput(display, window, clipboard, XFixesSetSelectionOwnerNotifyMask);

    while (1) {
        XEvent event;
        XNextEvent(display, &event);

        if (event.type == event_base + XFixesSelectionNotify) {
            XFixesSelectionNotifyEvent* sev = (XFixesSelectionNotifyEvent*)&event;
            if (sev->subtype == XFixesSetSelectionOwnerNotify) {
                handle_selection(display, window, property);
            }
        }
    }

    XCloseDisplay(display);
    return 0;
}