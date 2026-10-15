#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Menu_Bar.H>
#include <emscripten.h>

#include <Aspect_DisplayConnection.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <V3d_Viewer.hxx>
#include <V3d_View.hxx>
#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <BRepPrimAPI_MakeBox.hxx>

#ifdef __EMSCRIPTEN__
#include <Wasm_Window.hxx>
#else
#include <FL/x.H>
#include <Xw_Window.hxx>
#endif

Handle(Aspect_DisplayConnection) display;
Handle(OpenGl_GraphicDriver) driver;
Handle(V3d_Viewer) viewer;
Handle(V3d_View) view;
Handle(AIS_InteractiveContext) ctx;
bool initialized = false;

Fl_Window *win;
Fl_Group* fg_viewer = nullptr;
// Função C++ otimizada para reposicionar o canvas no DOM via GPU
void update_web_canvas_geometry() {
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
}

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

int main(int argc, char **argv) {
	Fl::event_dispatch(meu_filtro_de_eventos);
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
		  janela_bloqueada = win;
          fl_message("Trying to");		  
		  janela_bloqueada = nullptr;

        },
        0, 0);
    
    Fl_Button *btn = new Fl_Button(10, 40, 80, 25, "Click");
    btn->callback(button_cb);

    
    OCC_Viewer *occv = new OCC_Viewer(100,100,200, 200);
    // occv->border(0);

    win->end();
    win->resizable(win);
    win->show(argc, argv);

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

    return Fl::run();
}