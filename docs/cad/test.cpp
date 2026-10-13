// em++ test.cpp   -o app.mjs   -O2   -I/home/super/msv/emsdk/downloads/fltk_wasm32_emscripten   -I/home/super/msv/emsdk/downloads/fltk_wasm32_emscripten/build_wasm   /home/super/msv/emsdk/downloads/fltk_wasm32_emscripten/build_wasm/lib/libfltk.a   --bind   -s MODULARIZE=1   -s ASYNCIFY=1   -s ALLOW_MEMORY_GROWTH=1 -s EXPORTED_RUNTIME_METHODS=ccall  -std=c++17

// g++ test.cpp   -o test   -O2 -lfltk -lfltk_images -lfltk -lX11 -lXtst -Os -s  -std=c++20 -lXext -lXft -lXrender -lXcursor -lXinerama -lXfixes -lfontconfig -lfreetype -lz -lm -ldl -lpthread -lstdc++ -Os -w -Wfatal-errors -DNDEBUG -lasound  -lXss -lXi -ludev  -ljpeg -lz



#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Menu_Bar.H>

#include <FL/fl_ask.H>
#include <iostream>
#include <fstream>
#include <cstdint>

#if defined(__EMSCRIPTEN__)
    #include <emscripten/emscripten.h>
    #include <emscripten/html5.h>
#endif
// Callback function triggered when the button is clicked
void button_cb(Fl_Widget* w, void* data) {
    Fl_Box* box = static_cast<Fl_Box*>(data);
    box->label("Hello from FLTK WebAssembly!");
    w->label("Success!");
     
    fl_message("hi");
}
 
int main(int argc, char **argv) {
    Fl::use_high_res_GL(1);
    // setenv("LIBGL_ALWAYS_SOFTWARE", "1", 1);
    std::cout << "FLTK version: " << FL_MAJOR_VERSION << "." << FL_MINOR_VERSION
              << "." << FL_PATCH_VERSION << std::endl;
    std::cout << "FLTK ABI version: " << FL_ABI_VERSION << std::endl;
    
    Fl::visual(FL_DOUBLE | FL_INDEX);
    // Fl::gl_visual(FL_RGB8 | FL_DOUBLE | FL_DEPTH | FL_STENCIL );
    // Fl::gl_visual(FL_RGB8 | FL_DOUBLE | FL_DEPTH | FL_STENCIL | FL_MULTISAMPLE);
    Fl::scheme("oxy"); 
  
    // Fl_Group::current(content);
  
    Fl::lock();
    // Global gray widget border
    // Fl::set_boxtype(FL_BORDER_BOX, gray_border_box);
    // Create a standard non-GL window
    Fl_Window *window = new Fl_Window(0,100,400, 250, "FLTK Non-GL GUI Test");
    // window->box(FL_FLAT_BOX);
    Fl_Menu_Bar* menu = new Fl_Menu_Bar(0, 0, 400, 22);
    // menu->color(FL_DARK3);
    // menu->textcolor(FL_WHITE);
    menu->add(
        "File/Save", FL_CTRL + 's',
        // "Tools/FEM ", FL_ALT + 'm',
        [](Fl_Widget *mnu, void *ud) {
          printf("success"); 
          fl_message("Trying to");
        },
        0, 0);
    // Add a text box message
    Fl_Box *box = new Fl_Box(30, 40, 340, 50, "Click the button below!");
    box->labelsize(16);
    box->labelfont(FL_BOLD);

    // Add an interactive button
    Fl_Button *btn = new Fl_Button(120, 130, 160, 50, "Click Me");
    btn->callback(button_cb, box);
    
    printf("successinit");

    window->end();
	// window->resizable(window);
    window->show(argc, argv);
    window->maximize();

    return Fl::run();
//     #ifndef __EMSCRIPTEN__
//   while (Fl_X::first) wait(FOREVER);
// #else
//   emscripten_set_main_loop([]() { Fl::wait(); }, 0, true);
// #endif
//   return 0;
}