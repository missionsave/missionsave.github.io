// g++ flxpm.cpp  -o flxpm -lfltk_images -lfltk -lX11 -lXtst -Os -s  -std=c++20 -lXext -lXft -lXrender -lXcursor -lXinerama -lXfixes -lfontconfig -lfreetype -lz -lm -ldl -lpthread -lstdc++ -Os -w -Wfatal-errors -DNDEBUG -lasound  -lXss -lXi -ludev  -ljpeg -lz

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Pixmap.H>

// Hardcoded XPM image (tiny 3x3 checkerboard)
static const char *checker_xpm[] = {
"24 16 17 1 12 8",
"  c None",
"s c #282a2e",
"S c #373b41",
"r c #a54242",
"R c #cc6666",
"g c #8c9440",
"G c #b5bd68",
"y c #de935f",
"Y c #f0c674",
"b c #5f819d",
"B c #81a2be",
"m c #85678f",
"M c #b294bb",
"c c #5e8d87",
"C c #8abeb7",
"w c #707880",
"W c #c5c8c6",
"                        ",
" wwwwwwwwwwwwwwwwwwww   ",
" w                  w   ",
" w SSSSSSSS         w   ",
" w SSSSSSSS         www ",
" w SSSSSSSS           w ",
" w SSSSSSSS           w ",
" w SSSSSSSS           w ",
" w SSSSSSSS           w ",
" w SSSSSSSS           w ",
" w SSSSSSSS           w ",
" w SSSSSSSS         www ",
" w SSSSSSSS         w   ",
" w                  w   ",
" wwwwwwwwwwwwwwwwwwww   ",
"                        ",
};

int main(int argc, char **argv) {
    Fl_Window *win = new Fl_Window(200, 200, "XPM Hardcoded Example");

    // Wrap the XPM data in an Fl_Pixmap
    Fl_Pixmap *pixmap = new Fl_Pixmap(checker_xpm);

    // Display it inside a box
    Fl_Box *box = new Fl_Box(20, 20, 160, 160);
    box->image(pixmap);

    win->end();
    win->show(argc, argv);
    return Fl::run();
}
