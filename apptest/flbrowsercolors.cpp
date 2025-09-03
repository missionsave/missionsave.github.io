// g++ flbrowsercolors.cpp -o flbrowsercolors -lfltk -lX11 -lXinerama -lXrender -lXfixes -lfontconfig -lfreetype -lXft -lXcursor

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Browser.H>
#include <FL/fl_draw.H>
#include <sstream>

int main() {
    Fl_Window win(800, 600, "Teste @B e @C");
    Fl_Browser browser(10, 10, 780, 580);

        // for (int c = 1; c <= 255; ++c) {
    for (int b = 1; b <= 255; ++b) {
        for (int c = 1; c <= 255; ++c) {
            std::ostringstream oss;
            oss << "@B" << b << "@C" << c
                << "H\t Texto B" << b << " C" << c;
            browser.add(oss.str().c_str());
        }
    }

    win.end();
    win.show();
    return Fl::run();
}
