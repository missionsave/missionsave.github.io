// g++ flgltest.cpp -o flgltest `fltk-config --cxxflags --ldflags --use-gl`

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Gl_Window.H>
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#include <GL/glext.h>
#endif
#include <cstdio>

class SimpleGLWindow : public Fl_Gl_Window {
public:
    SimpleGLWindow(int X, int Y, int W, int H, const char* L = 0)
        : Fl_Gl_Window(X, Y, W, H, L) 
    {
        // Try to request 32-bit depth buffer
        mode(FL_RGB | FL_DOUBLE | FL_DEPTH32);
        if (!can_do(mode())) {
            fprintf(stderr, "FL_DEPTH32 not supported, falling back to FL_DEPTH\n");
            mode(FL_RGB | FL_DOUBLE | FL_DEPTH);
        }
    }

private:
    void draw() override {
        if (!valid()) {
            make_current();

#ifndef __APPLE__ 
            GLint depthBits = 0;
            glGetIntegerv(GL_DEPTH_BITS, &depthBits);
            printf("Window depth buffer precision: %d bits\n", depthBits);
#endif
            glEnable(GL_DEPTH_TEST);
            glViewport(0, 0, w(), h());
        }

        glClearColor(0.08f, 0.10f, 0.14f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBegin(GL_TRIANGLES);
            glColor3f(0.9f, 0.2f, 0.2f); glVertex2f(-0.6f, -0.6f);
            glColor3f(0.2f, 0.9f, 0.2f); glVertex2f( 0.6f, -0.6f);
            glColor3f(0.2f, 0.2f, 0.9f); glVertex2f( 0.0f,  0.6f);
        glEnd();
    }
};

int main(int argc, char** argv) {
    Fl::visual(FL_DOUBLE | FL_RGB);

    Fl_Window window(640, 480, "FLTK OpenGL Depth Precision Demo");
    SimpleGLWindow glview(10, 10, window.w() - 20, window.h() - 20);
    window.resizable(glview);
    window.end();

    window.show(argc, argv);
    return Fl::run();
}
