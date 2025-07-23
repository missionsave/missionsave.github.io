#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/x.H>
#include <iostream>
#include <unordered_set>

// Variáveis globais
Display* dpy;
Window root;
std::unordered_set<Window> managed_clients;

bool should_manage_window(Window win) {
    if (win == root) return false;

    XWindowAttributes attr;
    if (!XGetWindowAttributes(dpy, win, &attr)) return false;

    if (attr.override_redirect) return false;
    if (attr.map_state == IsViewable) return false; // já visível, provavelmente já tem moldura

    if (managed_clients.count(win)) return false;   // já está gerenciado

    return true;
}

void manage_window(Window client) {
    if (!should_manage_window(client)) return;

    managed_clients.insert(client);

    XWindowAttributes attr;
    XGetWindowAttributes(dpy, client, &attr);

    int client_w = (attr.width <= 1) ? 640 : attr.width;
    int client_h = (attr.height <= 1) ? 480 : attr.height;

    const int border = 2;
    const int title_height = 30;

    int total_w = client_w + 2 * border;
    int total_h = client_h + title_height + border;

    // Criar moldura: apenas título e bordas
    Fl_Window* frame = new Fl_Window(attr.x, attr.y, total_w, total_h);
    frame->border(0);           // Sem borda de sistema
    frame->override();
    frame->begin();

    Fl_Box* title = new Fl_Box(border, border, client_w, title_height, "Janela X11");
    title->box(FL_UP_BOX);
    title->color(FL_DARK_RED);
    title->labelcolor(FL_WHITE);
    title->labelfont(FL_BOLD);

    frame->end();
    frame->show();

    Window frame_id = fl_xid(frame);

    XSetWindowAttributes swa;
    swa.override_redirect = True;
    XChangeWindowAttributes(dpy, frame_id, CWOverrideRedirect, &swa);

    // Reparentar cliente diretamente abaixo da barra de título
    XReparentWindow(dpy, client, frame_id, border, title_height);

    // Mapear cliente dentro do espaço disponível
    XMapWindow(dpy, client);

    XMapRaised(dpy, frame_id);
    XSync(dpy, False);

    std::cout << "✔️ Cliente " << client << " dentro da moldura " << frame_id << std::endl;
}

int main() {
    dpy = XOpenDisplay(nullptr);
    if (!dpy) return 1;
    root = DefaultRootWindow(dpy);

    XSelectInput(dpy, root,
        SubstructureRedirectMask |  // Para interceptar MapRequest
        SubstructureNotifyMask     // Para DestroyNotify, UnmapNotify
    );

    while (true) {
        XEvent ev;
        XNextEvent(dpy, &ev);

        if (ev.type == MapRequest) {
            manage_window(ev.xmaprequest.window);
        }
    }

    XCloseDisplay(dpy);
    return 0;
}
