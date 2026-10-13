// em++ test.cpp   -o app.mjs   -O2   -I/home/super/msv/emsdk/downloads/fltk_wasm32_emscripten   -I/home/super/msv/emsdk/downloads/fltk_wasm32_emscripten/build_wasm   /home/super/msv/emsdk/downloads/fltk_wasm32_emscripten/build_wasm/lib/libfltk.a   --bind   -s MODULARIZE=1   -s ASYNCIFY=1   -s ALLOW_MEMORY_GROWTH=1 -s EXPORTED_RUNTIME_METHODS=ccall  -std=c++17

// g++ test.cpp   -o test   -O2 -lfltk -lfltk_images -lfltk -lX11 -lXtst -Os -s  -std=c++20 -lXext -lXft -lXrender -lXcursor -lXinerama -lXfixes -lfontconfig -lfreetype -lz -lm -ldl -lpthread -lstdc++ -Os -w -Wfatal-errors -DNDEBUG -lasound  -lXss -lXi -ludev  -ljpeg -lz



#include <FL/Fl.H>
#include <FL/x.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Menu_Bar.H>

#include <FL/fl_ask.H>
#include <iostream>
#include <fstream>
#include <cstdint> 
#include <iostream>
#include <iomanip>

#if defined(__EMSCRIPTEN__)
    #include <emscripten/emscripten.h>
    #include <emscripten/html5.h>
#endif

Fl_Window* janela_bloqueada = nullptr;
int meu_filtro_de_eventos(int event, Fl_Window* w) {
    // Se o evento for um clique/arrastar E for na janela alvo, cancela (retorna 1)
    if (w == janela_bloqueada && (event == FL_PUSH || event == FL_RELEASE || event == FL_DRAG)) {
        return 1; // Evento consumido e cancelado!
    }
    // Caso contrário, deixa o FLTK processar o evento normalmente
    return Fl::handle_(event, w);
}
// Callback function triggered when the button is clicked
void button_cb(Fl_Widget* w, void* data) {
    Fl_Box* box = static_cast<Fl_Box*>(data);
    box->label("Hello from FLTK WebAssembly!");
    w->label("Success!");
     
	Fl_Window* win = w->window();
    janela_bloqueada = win;
    fl_message("hi");
    janela_bloqueada = nullptr;
}
 
// -------------------------------------------------
// Universal event logger (works on all FLTK versions)
// -------------------------------------------------
int event_logger(int event) {
  // Simple name lookup table for the most common events
  const char* name = "???";
  switch (event) {
    case FL_NO_EVENT:      name = "NO_EVENT";      break;
    case FL_PUSH:          name = "PUSH";          break;
    case FL_RELEASE:       name = "RELEASE";       break;
    case FL_ENTER:         name = "ENTER";         break;
    case FL_LEAVE:         name = "LEAVE";         break;
    case FL_DRAG:          name = "DRAG";          break;
    case FL_FOCUS:         name = "FOCUS";         break;
    case FL_UNFOCUS:       name = "UNFOCUS";       break;
    case FL_KEYDOWN:       name = "KEYDOWN";       break;
    case FL_KEYUP:         name = "KEYUP";         break;
    case FL_CLOSE:         name = "CLOSE";         break;
    case FL_MOVE:          name = "MOVE";          break;
    case FL_SHORTCUT:      name = "SHORTCUT";      break;
    case FL_DEACTIVATE:    name = "DEACTIVATE";    break;
    case FL_ACTIVATE:      name = "ACTIVATE";      break;
    case FL_HIDE:          name = "HIDE";          break;
    case FL_SHOW:          name = "SHOW";          break;
    case FL_PASTE:         name = "PASTE";         break;
    case FL_SELECTIONCLEAR:name = "SELECTIONCLEAR";break;
    case FL_MOUSEWHEEL:    name = "MOUSEWHEEL";    break;
    case FL_DND_ENTER:     name = "DND_ENTER";     break;
    case FL_DND_DRAG:      name = "DND_DRAG";      break;
    case FL_DND_LEAVE:     name = "DND_LEAVE";     break;
    case FL_DND_RELEASE:   name = "DND_RELEASE";   break;
    case FL_SCREEN_CONFIGURATION_CHANGED: name = "SCREEN_CONFIG"; break;
    case FL_FULLSCREEN:    name = "FULLSCREEN";    break;
    case FL_ZOOM_GESTURE:  name = "ZOOM_GESTURE";  break;
    default:               name = "OTHER";         break;
  }

  std::cout << "FLTK " 
  << std::left << std::setw(3) << event << " "
  << std::left << std::setw(14) << name << " "
  << "x=" << std::right << std::setw(4) << Fl::event_x() << " "
  << "y=" << std::right << std::setw(4) << Fl::event_y() << " "
  << "key=" << Fl::event_key() << " "
  << "state=0x" << std::setfill('0') << std::setw(4) << std::hex << Fl::event_state() 
  << std::dec << std::setfill(' ') << "\n";
  return 0;   // continue normal handling
}
int main(int argc, char **argv) {
    Fl::lock();
	// Fl::add_handler(event_logger);
    // Fl::use_high_res_GL(1); // seem problem in wasm
    // setenv("LIBGL_ALWAYS_SOFTWARE", "1", 1);
    std::cout << "FLTK version: " << FL_MAJOR_VERSION << "." << FL_MINOR_VERSION
              << "." << FL_PATCH_VERSION << std::endl;
    std::cout << "FLTK ABI version: " << FL_ABI_VERSION << std::endl;
    
    Fl::visual(FL_DOUBLE | FL_INDEX);
    // Fl::gl_visual(FL_RGB8 | FL_DOUBLE | FL_DEPTH | FL_STENCIL );
    // Fl::gl_visual(FL_RGB8 | FL_DOUBLE | FL_DEPTH | FL_STENCIL | FL_MULTISAMPLE);
    Fl::scheme("oxy"); 
  
    // Fl_Group::current(content);
	Fl::event_dispatch(meu_filtro_de_eventos);
    // Global gray widget border
    // Fl::set_boxtype(FL_BORDER_BOX, gray_border_box);
    // Create a standard non-GL window
    Fl_Window *window = new Fl_Window(0,100,400, 250, "FLTK Non-GL GUI Test");
    // window->set_modal();
    window->clear_border();
	// window->set_override();
    Fl_Menu_Bar* menu = new Fl_Menu_Bar(0, 0, 400, 22);
    // menu->color(FL_DARK3);
    // menu->textcolor(FL_WHITE);
    menu->add(
        "File/Save", FL_CTRL + 's',
        // "Tools/FEM ", FL_ALT + 'm',
        [](Fl_Widget *mnu, void *ud) {
          printf("success"); 

		  Fl_Window* win = mnu->window();
		  janela_bloqueada = win;
          fl_message("Trying to");		  
		  janela_bloqueada = nullptr;

        },
        0, 0);
    // Add a text box message
    Fl_Box *box = new Fl_Box(30, 40, 340, 50, "Click the button below!!");
    box->labelsize(16);
    box->labelfont(FL_BOLD);

    // Add an interactive button
    Fl_Button *btn = new Fl_Button(120, 130, 160, 50, "Click Me");
    btn->callback(button_cb, box);
    
    printf("successinit");

    window->end();
	
	// window->resizable(window);
    window->show(argc, argv);
    // window->maximize();

    // return Fl::run();
    #ifndef __EMSCRIPTEN__
	return Fl::run();
#else
emscripten_set_main_loop([]() {
    Fl::flush();
	// Fl::wait(0.01);
}, 23, true);
#endif
  return 0;
}