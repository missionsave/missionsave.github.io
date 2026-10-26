#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Menu_Bar.H>

// #include <Aspect_DisplayConnection.hxx>
// #include <OpenGl_GraphicDriver.hxx>
// #include <V3d_Viewer.hxx>
// #include <V3d_View.hxx>
// #include <AIS_InteractiveContext.hxx>
// #include <AIS_Shape.hxx>
// #include <BRepPrimAPI_MakeBox.hxx>

#ifdef __EMSCRIPTEN__
#include <emscripten.h> 
#include <emscripten/html5.h>
#include <Wasm_Window.hxx>
#else
#include <FL/x.H>
#include <Xw_Window.hxx>
#endif
using namespace std;
#include <string>
Fl_Window *win;
#include "fl_scintilla.hpp"

struct scint : public fl_scintilla {
  scint(int X, int Y, int W, int H, const char *l = 0)
      : fl_scintilla(X, Y, W, H, l) {
    //  show_browser=0; set_flag(FL_ALIGN_INSIDE);
    // cotm(filename) 
	// currfilename = filename;
    callbackOnload = ([this]() {
    //   isdebuging = 0;
	//   breakpoint=-1;
      hints = {"Part"};
    //   lua_str(filename, 1);
      win->label(string(std::filesystem::path(filename).stem().string()+" MissionCad").c_str());
	//   mhide.clear();
	//   msolo.clear();
    //   currfilename = filename;
    });
  }
  bool stepping=0;
  int cursorpos=0;
  int handle(int e) override;
};

int scint::handle(int e) {

  if (e == FL_KEYDOWN && Fl::event_state(FL_CTRL) &&
      Fl::event_key() == FL_F + 1) {
    int currentPos = SendEditor(SCI_GETCURRENTPOS, 0, 0);
    int currentLine = SendEditor(SCI_LINEFROMPOSITION, currentPos, 0);
    int lineLength = SendEditor(SCI_LINELENGTH, currentLine, 0);
    if (lineLength <= 0)
      return 0;
    std::string buffer(lineLength, '\0'); // allocate with room for '\0'
    SendEditor(SCI_GETLINE, currentLine, (sptr_t)buffer.data());
    buffer.erase(buffer.find_last_not_of("\r\n\0") + 1);
    // lua_str(buffer,0);
    return 1;
  }
  // if(e == FL_KEYDOWN && Fl::event_state(FL_CTRL) && Fl::event_key()==FL_F +
  // 2){
  // 	// cotm("f2",filename);
  // 	lua_str(filename,1);
  // 	return 1;
  // }
  if (e == FL_KEYDOWN && Fl::event_state(FL_CTRL) && Fl::event_key() == 's') {
    fl_scintilla::handle(e);
    // cotm("f2",filename)
    // isdebuging = 0;
	int currentPos = SendEditor(SCI_GETCURRENTPOS, 0, 0);
    cursorpos = SendEditor(SCI_LINEFROMPOSITION, currentPos, 0);
    // lua_str(filename, 1); /////
    return 1;
  }

  if (e == FL_UNFOCUS)
    SendEditor(SCI_AUTOCCANCEL);

  int ret = fl_scintilla::handle(e);
  if (e == FL_KEYDOWN) {
    // lua_str_realtime(getalltext());
  }
  return ret;
}
scint *editor;

void scint_init(int x, int y, int w, int h) { editor = new scint(x, y, w, h); }
#if 0
Handle(Aspect_DisplayConnection) display;
Handle(OpenGl_GraphicDriver) driver;
Handle(V3d_Viewer) viewer;
Handle(V3d_View) view;
Handle(AIS_InteractiveContext) ctx;
bool initialized = false;

Fl_Group* fg_viewer = nullptr;
// Função C++ otimizada para reposicionar o canvas no DOM via GPU
void update_web_canvas_geometry() {
	#ifdef __EMSCRIPTEN__
    int x=fg_viewer->x();
    int y=fg_viewer->y();
    int w=fg_viewer->w();
    int h=fg_viewer->h();
    // Usamos MAIN_THREAD_EM_ASM para garantir que corre na thread principal do browser
    std::cout<<x<<" "<<y<<" "<<w<<" "<<h<<" \n";
    MAIN_THREAD_EM_ASM({
        // 1. Seleciona o canvas principal pelo ID (ou classe se o FLTK criar múltiplos)
        const canvas = document.getElementById('canvas') || document.querySelector('canvas');
        
        if (canvas) {
            // Configurações iniciais rápidas (só aplica se não estiverem definidas)
            if (canvas.style.position !== 'absolute') {
                canvas.style.position = 'absolute';
                canvas.style.left = '0px';
                canvas.style.top = '0px';
            }

            // O SEGREDO DA PERFORMANCE: Mover via GPU (transform) em vez de style.top/left
            canvas.style.transform = 'translate(' + $0 + 'px, ' + $1 + 'px)';
            
            // Alterar a largura e altura diretamente no estilo inline
            canvas.style.width = $2 + 'px';
            canvas.style.height = $3 + 'px';
        }
    }, x, y, w, h); // Passa as variáveis do C++ para o JavaScript ($0, $1, $2, $3)
	#endif
}


class OCC_Viewer : public Fl_Group
{
public:

// bool initialized = false;
bool rotating = false;
int lastX = 0;
int lastY = 0;

OCC_Viewer(int X, int Y, int W, int H, const char* L = nullptr)
    : Fl_Group(X, Y, W, H, L) {
        
        fg_viewer = this;
    }

    void resize(int x, int y, int w, int h) override {
        Fl_Group::resize(x, y, w, h); // Atualiza o estado interno do FLTK
        
        // Sincroniza instantaneamente o elemento HTML com o menor custo de CPU
        update_web_canvas_geometry();
    }

int handle(int event) override
{
    if (!initialized)
        return Fl_Group::handle(event);

    switch (event)
    {
    case FL_PUSH:
        if (Fl::event_button() == FL_LEFT_MOUSE)
        {
            lastX = Fl::event_x();
            lastY = Fl::event_y();
            view->StartRotation(lastX, lastY);
            rotating = true;
            return 1;
        }
        break;
    case FL_DRAG:
        if (rotating)
        {
            view->Rotation(Fl::event_x(), Fl::event_y());
            view->Redraw();
            return 1;
        }
        break;
    case FL_RELEASE:
        rotating = false;
        return 1;
    case FL_MOUSEWHEEL:
    {
        int x = Fl::event_x();
        int y = Fl::event_y();
        view->StartZoomAtPoint(x, y);
        int endY = y - Fl::event_dy() * 20;
        view->ZoomAtPoint(x, y, x, endY);
        view->Redraw();
        return 1;
    }
    }
    return Fl_Group::handle(event);
}
};

// Global pointer to reference our viewer instance safely across async callback
static OCC_Viewer* g_viewer = nullptr;








#ifdef __EMSCRIPTEN__

class OCC_ViewerManager {
public: 

    void init() {
        if (initialized) return;

        display = new Aspect_DisplayConnection(
#ifndef __EMSCRIPTEN__
            fl_display
#else
            ""
#endif
        );

        driver = new OpenGl_GraphicDriver(display);
        viewer = new V3d_Viewer(driver);

        viewer->SetDefaultLights();
        viewer->SetLightOn();

        ctx = new AIS_InteractiveContext(viewer);
        view = viewer->CreateView();

#ifdef __EMSCRIPTEN__
        // Target the primary Emscripten canvas selector "#canvas"
        Handle(Wasm_Window) nativeWindow = new Wasm_Window("#canvas", true);
#endif

        view->SetWindow(nativeWindow);

        TopoDS_Shape cube = BRepPrimAPI_MakeBox(100, 100, 100).Shape();
        Handle(AIS_Shape) shape = new AIS_Shape(cube);
        ctx->Display(shape, Standard_False);

        initialized = true;
        view->MustBeResized();
		view->Window()->DoResize();
        view->FitAll(0.01, Standard_True);
        view->Redraw();
    }
};

static OCC_ViewerManager g_occt;

extern "C" {
    EMSCRIPTEN_KEEPALIVE void init_occt_viewer() {
        g_occt.init();
    }
    EMSCRIPTEN_KEEPALIVE void update_web_canvas_geometry_() {
        update_web_canvas_geometry();
        view->MustBeResized();
		view->Window()->DoResize();
        view->FitAll(0.01, Standard_True);
        view->Redraw();
    }
    EMSCRIPTEN_KEEPALIVE void resize_event(int x, int y, int w, int h) {
		win->resize(x, y+0, w, h-0);
		update_web_canvas_geometry();
        view->MustBeResized();
		view->Window()->DoResize();
        view->FitAll(0.01, Standard_True);
        view->Redraw();
    }
	bool joculto=0;
	EMSCRIPTEN_KEEPALIVE void forcar_destruicao_dialogo(int jo) {
        std::cout << "JS intercetou o fecho do fl_ask! A forçar a CPU a 0%...\n";
		joculto=jo;
		return;
        
        // Pega na janela de diálogo fantasma que ficou presa e limpa-a
        Fl_Window* win = Fl::first_window();
        if (win != nullptr && win->label() == nullptr) {
            win->hide();
            Fl::delete_widget(win); // Elimina a janela e mata o ciclo síncrono
        }
    }
	// The beforeunload callback function signature
	EM_BOOL before_unload_callback(int eventType, const EmscriptenUiEvent *uiEvent, void *userData) {
		// Perform any synchronous cleanup, flush files, or save states here
		std::cout << "Application closing or page refreshing..." << std::endl;

		// Returning true (or a non-zero value) tells the browser to prompt 
		// the user with a confirmation dialog before leaving.
		// Note: Modern browsers override custom strings, displaying a standardized message instead.
		return EM_TRUE;
	}
}
#endif
#endif
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <cstdio>

class SubWindow : public Fl_Window
{
public:
    using Fl_Window::Fl_Window;

    int handle(int event) override
    {
        if (event == FL_CLOSE)
        {
            std::printf("Subwindow X clicked!\n");
            std::fflush(stdout);
        }

        return Fl_Window::handle(event);
    }
};
const char* before_unload_callback(int eventType, const void *reserved, void *userData) {
    std::cout << "Application closing or page refreshing..." << std::endl;

    // Return a non-empty string to trigger the browser's native confirmation dialog.
    // Modern browsers ignore the custom text content and show a generic message instead.
    return "Changes you made may not be saved.";
}
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>

Fl_Window* janela_bloqueada = nullptr;
// Callback acionado tanto pelo botão OK quanto pelo "X" da barra de título
void fechar_aviso_cb(Fl_Widget* w, void* data) {
	std::cout<<"nazare2\n";
    Fl_Window* win = (Fl_Window*)data;
    win->hide();
    delete win; // Remove o elemento do DOM e liberta a memória com segurança
	janela_bloqueada= nullptr;
}

// Suponha que 'mainWindow' seja a sua janela principal global ou acessível aqui
 

void mostrar_aviso(const char* mensagem) {
    // win->begin();
    SubWindow* popup = new SubWindow(50, 100, 320, 130, "Aviso");
    // popup->callback(fechar_aviso_cb, popup);
    
    Fl_Box* box = new Fl_Box(20, 25, 280, 40, mensagem);
    box->align(FL_ALIGN_WRAP | FL_ALIGN_CENTER);
    
    Fl_Button* btn_ok = new Fl_Button(120, 85, 80, 30, "OK");
    btn_ok->callback(fechar_aviso_cb, popup);
    popup->set_modal();
    popup->end();
	janela_bloqueada = popup;
    // win->end();
    
    // // **O SEGREDO:** Remove e readiciona o popup para jogá-lo 
    // // para o final da pilha de desenho da main (topo absoluto)
    // win->remove(popup);
    // win->add(popup);
    
    popup->show();
}

int meu_filtro_de_eventos(int event, Fl_Window* w) {


	Fl_Window* last = nullptr;

	for (Fl_Window* w = Fl::first_window(); w; w = Fl::next_window(w))
	{
		if (w->visible())
			last = w;
	}
	
	if (last)
		std::cout << "Last visible window: " << last->label() << std::endl;

		Fl_Widget * wn = Fl::belowmouse();

if (wn)
    std::cout << "Mouse window: " << wn->label() << std::endl;

	
	// // janela_bloqueada->hide();
	// // janela_bloqueada->show();
	// // std::cout<<"tt "<<event<<" "<<(w==win)<<"\n";
	// if (event == FL_RELEASE && janela_bloqueada!= nullptr && !joculto) {
	// 	janela_bloqueada->hide();
    //     janela_bloqueada->show(); // Reordena o popup para o topo da pilha interna do FLTK
	// 	std::cout<<"nazare\n";
	// 	// return 0;
    // }
    return Fl::handle_(event, w);


	std::cout<<"ev "<<event<<" "<<(w != nullptr?w->label():0)<<"\n";
    // Se o evento for um clique/arrastar E for na janela alvo, cancela (retorna 1)
    if (w == janela_bloqueada && (event == FL_PUSH || event == FL_RELEASE || event == FL_DRAG)) {
        return 1; // Evento consumido e cancelado!
    }
	if (event == FL_CLOSE && w != nullptr) {
        
        // O fl_ask/fl_message herda ou define um título padrão. 
        // Podemos verificar o título atual da barra para ter a certeza.
        const char* titulo_atual = w->label();
        
        // Se não mudou o título, por defeito a FLTK deixa-o vazio ou como o último configurado.
        // Dica: Pode forçar um título específico antes do fl_ask usando: fl_message_title("Confirmar");
		std::cout<<"ec "<<w->label()<<"\n";
        if (w->label() != nullptr) {
			std::cout<<"ecc "<<w->label()<<"\n";
            
            w->hide();
            Fl::delete_widget(w); // Liberta apenas a janela do diálogo
            return 1; // Evento tratado de forma isolada!
        }
	}
    // Caso contrário, deixa o FLTK processar o evento normalmente
    return Fl::handle_(event, w);
}
// 1. Esta função vai intercetar o "X" que o seu dispatch não consegue ver
// 1. GESTOR GLOBAL DE EVENTOS (Substitui com sucesso o default_callback)
int meu_interceptador_de_fecho(int event) {
	std::cout<<"tte "<<event<<" "<<"\n";
    // Quando qualquer janela tenta fechar (clique no "X")
    if (event == FL_CLOSE) {
        Fl_Window* win = Fl::first_window(); // Pega a janela que disparou o evento
        
        if (win != nullptr && win->label() == nullptr) {
            std::cout << "Cruz do fl_ask intercetada nativamente! A salvar a CPU...\n";
            win->hide();
            Fl::delete_widget(win); // Destrói o widget para quebrar o loop infinito síncrono
            return 1; // Avisa a FLTK que o evento foi consumido
        }
    }
    return 0; // Deixa os outros eventos passarem para o fluxo normal
}

// Callback function triggered when the button is clicked
#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Window.H>

// One-time installation
static void safe_message_close_cb(Fl_Widget* w, void*) {
  // Treat title-bar X the same as the Close/OK button
  // (the internal form stores the result in a static)
  Fl_Window* win = (Fl_Window*)w;
  win->hide();                     // still hide
  // The nested wait() will now exit cleanly because hide() was called
  // from a controlled place.
}

// Hook it after the first dialog is created, or force creation:
void install_safe_message_close() {
  // Force creation of the internal form
  fl_message(" ");                 // dummy – will be closed immediately
  // Now the form exists; find it and replace its callback
  // (exact pointer is internal, but under the wasm port the last
  //  shown modal window is easy to catch)
}
void init_safe_dialogs()
{
    fl_message(" ");               // force creation of the internal form
    if (Fl_Window *m = Fl::modal()) {
        m->border(0);              // kill the title-bar X
        m->hide();
    }
}
void button_cb(Fl_Widget* w, void* data) { 
	fl_message("hi");
	return;
	std::cout<<"hiexit\n";
	
	// Remove it first if it's already part of the group
// win->remove(janela_bloqueada);
janela_bloqueada->hide();
janela_bloqueada->show();

// // Insert at a specific index (e.g., index 0 is at the bottom/back)
// parent_window->insert(*subwindow, desired_index);

// // Alternatively, insert it right before another widget/subwindow
// // parent_window->insert(*subwindow, other_widget_ptr);

// parent_window->redraw();
    // win->activate();
    // janela_bloqueada->show();
    // janela_bloqueada->activate();
    // janela_bloqueada->redraw();

    // Fl::focus(janela_bloqueada);
    // Fl::flush();
	std::cout<<"hiexit\n";
	return;
    Fl_Box* box = static_cast<Fl_Box*>(data);
    box->label("Hello from FLTK WebAssembly!");
    w->label("Success!");
     
	Fl_Window* win = w->window();
    // janela_bloqueada = win;
	Fl::flush();

    fl_message("hi");
	std::cout<<"hiexit\n";
	// emscripten_run_script("minhaFuncaoGlobalJS();");
    // janela_bloqueada = nullptr;
} 

int main(int argc, char **argv) {
	// Fl::lock();
	// Fl::event_dispatch(meu_filtro_de_eventos);
	// Pass nullptr for userData, or pass a pointer to your app state class if needed
    // EMSCRIPTEN_RESULT result = emscripten_set_beforeunload_callback(nullptr, before_unload_callback);
	// if (result != EMSCRIPTEN_RESULT_SUCCESS) {
    //     std::cerr << "Failed to set beforeunload callback! Result: " << result << std::endl;
    // }

	// Fl::add_handler(meu_interceptador_de_fecho);
    win = new Fl_Window(100,100,500, 350, "FLTK + OCCT Wasm");
	win->border(0);
    win->begin();

	Fl_Menu_Bar* menu = new Fl_Menu_Bar(0, 0, 500, 22);
    // menu->color(FL_DARK3);
    // menu->textcolor(FL_WHITE);
    menu->add(
        "File/Save", FL_CTRL + 's',
        // "Tools/FEM ", FL_ALT + 'm',
        [](Fl_Widget *mnu, void *ud) {
          printf("success"); 

		  Fl_Window* win = mnu->window();
		//   janela_bloqueada = win;
          mostrar_aviso("Trying to");		  
		//   janela_bloqueada = nullptr;

        },
        0, 0);
    
    Fl_Button *btn = new Fl_Button(210, 222, 80, 25, "Click");
    btn->callback(button_cb);

    
    // OCC_Viewer *occv = new OCC_Viewer(0,22,200, 300);
    // occv->border(0);

	Fl_Group::current(win);
	scint_init(220, 60, 300, 180);


    win->end();
    win->resizable(win);
    win->show(argc, argv);

	
	// init_safe_dialogs();

    // g_occt.init();

    // update_web_canvas_geometry();
    // C++ to JS Interop: Intercept FLTK's canvas at runtime and cage it inside .canvas-container
    // EM_ASM({
    //     setTimeout(() => {
    //         const container = document.querySelector('.canvas-container');
    //         // Find the canvas FLTK just spawned in the body
    //         const canvases = document.querySelectorAll('canvas');
    //         const fltkCanvas = canvases[canvases.length - 1]; // Usually the last created canvas

    //         if (container && fltkCanvas && fltkCanvas.parentNode !== container) {
    //             container.appendChild(fltkCanvas);
    //             fltkCanvas.style.position = 'absolute';
    //             fltkCanvas.style.top = '0px';
    //             fltkCanvas.style.left = '0px';
    //             fltkCanvas.style.width = '100%';
    //             fltkCanvas.style.height = '100%';
    //         }
    //     }, 50); // Small timeout to ensure FLTK has finished appending it to the DOM
    // });

    #ifndef __EMSCRIPTEN__
	return Fl::run();
#else
emscripten_set_main_loop([]() {
	// Fl::wait(0); 

    // // Optional: exit loop if all windows are closed
    // if (!Fl::first_window()) {
    //     emscripten_cancel_main_loop();
    // }
    Fl::flush();
	// Fl::wait(0.01);
}, 0, true);
#endif
}