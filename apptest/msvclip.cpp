// g++ -std=c++20 -O2 msvclip.cpp -o msvclip -lX11 -lXfixes
#include <string>
// #include <vector>
#include <iostream>


using namespace std;
// vector<string> vclipboard; 

#include <X11/extensions/Xfixes.h>
// #include <X11/extensions/XTest.h>
#include <X11/keysym.h>
#include <fstream>
#include <filesystem>

void save_to_file(const std::string& text) {
    std::string path = std::string(getenv("HOME")) + "/msvclip.txt";

    // Read existing file (if any)
    std::string old;
    if (std::filesystem::exists(path)) {
        std::ifstream fin(path, std::ios::binary);
        old.assign((std::istreambuf_iterator<char>(fin)),
                    std::istreambuf_iterator<char>());
    }

    // Write new content at top, followed by \0, then old content
    std::ofstream fout(path, std::ios::binary | std::ios::trunc);
    fout << text << '\0' << old;
}

 

std::string getClipboardText(Display* display, Window window, Atom property) {
    Atom clipboard = XInternAtom(display, "CLIPBOARD", False);
    Atom utf8 = XInternAtom(display, "UTF8_STRING", False);
    
    // Clear previous property value
    XDeleteProperty(display, window, property);
    
    // Request clipboard content
    XConvertSelection(display, clipboard, utf8, property, window, CurrentTime);
    XFlush(display);

    XEvent event;
    while (true) {
        XNextEvent(display, &event);
        
        if (event.type == SelectionNotify && event.xselection.selection == clipboard) {
            if (event.xselection.property == None) {
                return ""; // Clipboard owner didn't respond
            }
            
            Atom actualType;
            int actualFormat;
            unsigned long nItems, bytesAfter;
            unsigned char* data = nullptr;

            XGetWindowProperty(display, window, property, 0, (~0L), False,
                               AnyPropertyType, &actualType, &actualFormat,
                               &nItems, &bytesAfter, &data);
            
            if (!data) return "";
            
            std::string result(reinterpret_cast<char*>(data), nItems);
            XFree(data);
            XDeleteProperty(display, window, property);
            return result;
        }
    }
}

// int listenclipboard() {
int main() {
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        std::cerr << "Cannot open X display\n";
        return 1;
    }

    // Check for XFixes extension
    int event_base, error_base;
    if (!XFixesQueryExtension(display, &event_base, &error_base)) {
        std::cerr << "XFixes extension not available\n";
        return 1;
    }

    // Create a hidden window
    Window window = XCreateSimpleWindow(display, DefaultRootWindow(display),
                                        0, 0, 1, 1, 0, 0, 0);
    Atom property = XInternAtom(display, "CLIPBOARD_PROPERTY", False);
    std::string lastClipboardContent;
    
    // Select clipboard change notifications
    Atom clipboard = XInternAtom(display, "CLIPBOARD", False);
    XFixesSelectSelectionInput(display, window, clipboard, 
                               XFixesSetSelectionOwnerNotifyMask);
    
    // Main event loop
    while (true) {
        XEvent event;
        XNextEvent(display, &event);
        
        // Check if it's a clipboard change event
        if (event.type == event_base + XFixesSelectionNotify) {
            XFixesSelectionNotifyEvent *sev = (XFixesSelectionNotifyEvent *)&event;
            if (sev->subtype == XFixesSetSelectionOwnerNotify) {
                std::string content = getClipboardText(display, window, property);
                if (!content.empty() && content != lastClipboardContent) {
                    // std::cout << "Clipboard contents: " << content << "\n";
					// vclipboard.insert(vclipboard.begin(), content); 
                    save_to_file(content);
                    lastClipboardContent = content;

					// vclipboard.push_back(content);
                    lastClipboardContent = content;
                }
            }
        }
    }

    XDestroyWindow(display, window);
    XCloseDisplay(display);
    return 0;
}
 