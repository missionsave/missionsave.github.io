// sudo apt install libx11-dev libjpeg-turbo8-dev libxclip-dev libfltk1.3-dev


#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XTest.h>
#include <turbojpeg.h>
#include <cstdlib>
#include <fstream>
#include <iostream>

void copy_to_clipboard(const std::string& path) {
    std::string cmd = "xclip -selection clipboard -t image/jpeg -i " + path;
    system(cmd.c_str());
}

XImage* capture(Display* display, Window win, int& width, int& height) {
    XWindowAttributes attr;
    XGetWindowAttributes(display, win, &attr);
    width = attr.width;
    height = attr.height;
    return XGetImage(display, win, 0, 0, width, height, AllPlanes, ZPixmap);
}

Window get_focused_window(Display* display) {
    Window focused;
    int revert;
    XGetInputFocus(display, &focused, &revert);
    return focused;
}

void handle_screenshot(bool focused_only) {
    Display* display = XOpenDisplay(nullptr);
    if (!display) return;

    Window target = focused_only ? get_focused_window(display) : DefaultRootWindow(display);
    int width, height;
    XImage* img = capture(display, target, width, height);

    unsigned char* rgb = new unsigned char[width * height * 3];
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x) {
            unsigned long pixel = XGetPixel(img, x, y);
            int idx = (y * width + x) * 3;
            rgb[idx + 0] = (pixel & img->red_mask) >> 16;
            rgb[idx + 1] = (pixel & img->green_mask) >> 8;
            rgb[idx + 2] = (pixel & img->blue_mask);
        }

    tjhandle tj = tjInitCompress();
    unsigned char* jpegBuf = nullptr;
    unsigned long jpegSize = 0;
    tjCompressRGB(tj, rgb, width, 0, height, TJPF_RGB, &jpegBuf, &jpegSize, TJSAMP_444, 90, TJFLAG_FASTDCT);

    std::ofstream out("screenshot.jpg", std::ios::binary);
    out.write(reinterpret_cast<char*>(jpegBuf), jpegSize);
    out.close();

    copy_to_clipboard("screenshot.jpg");

    tjFree(jpegBuf);
    tjDestroy(tj);
    delete[] rgb;
    XDestroyImage(img);
    XCloseDisplay(display);
}

int main(int argc, char** argv) {
    Fl::add_handler([](int event) {
        if (event == FL_KEYDOWN) {
            int key = Fl::event_key();
            bool ctrl = Fl::event_state(FL_CTRL);

            if (key == FL_Print) {
                handle_screenshot(ctrl); // ctrl = focused window, else full screen
                return 1;
            }
        }
        return 0;
    });

    Fl_Window win(200, 100, "Press PrintScreen");
    win.show();
    return Fl::run();
}
