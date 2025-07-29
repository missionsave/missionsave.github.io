#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>

class BatteryWidget : public Fl_Widget {
public:
    BatteryWidget(int X, int Y, int W, int H)
        : Fl_Widget(X, Y, W, H), charge_level(75) {}

    void set_charge(int percent) {
        charge_level = percent;
        redraw();
    }

protected:
    void draw() override {
        const int pad = 4;
        int bw = w() - 2 * pad; // bateria width
        int bh = h() - 2 * pad; // bateria height
        int bx = x() + pad;
        int by = y() + pad;

        int tip_w = bw / 10;
        int tip_h = bh / 3;

        // Borda da bateria
        fl_color(FL_BLACK);
        fl_line_style(FL_SOLID, 2);
        fl_rect(bx, by, bw, bh);

        // Pino da bateria (em cima)
        int tip_x = bx + (bw - tip_w) / 2;
        int tip_y = by - tip_h;
        fl_rectf(tip_x, tip_y, tip_w, tip_h);

        // Preenchimento com base no nível de carga
        int fill_w = (bw - 4) * charge_level / 100;
        fl_color(charge_level < 20 ? FL_RED : FL_GREEN);
        fl_rectf(bx + 2, by + 2, fill_w, bh - 4);

        // Texto opcional de % no meio
        fl_color(FL_BLACK);
        fl_font(FL_HELVETICA, 12);
        char label[16];
        snprintf(label, sizeof(label), "%d%%", charge_level);
        fl_draw(label, bx, by, bw, bh, FL_ALIGN_CENTER);
    }

private:
    int charge_level;
};

int main(int argc, char **argv) {
    Fl_Window *win = new Fl_Window(200, 120, "Bateria");

    BatteryWidget *battery = new BatteryWidget(50, 30, 100, 40);
    battery->set_charge(75); // ou altere dinamicamente

    win->end();
    win->show(argc, argv);
    return Fl::run();
}
