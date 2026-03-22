// g++ ncnn3.cpp ncnn3_linked.cpp -o ncnn3l -I/home/super/msv/ncnn/build/install/include -L/home/super/msv/ncnn/build/install/lib -lncnn -lglslang -lMachineIndependent -lGenericCodeGen  -lOSDependent -lSPIRV -lpthread -fopenmp -lfltk -lfltk_images -lX11 -lXext -lXfixes -lXcursor -lXrender -lXinerama -lXft -lfontconfig -O3 -march=native



#include <FL/Fl.H>
#include <FL/Fl_Window.H> 
using namespace std; 
extern Fl_Window* ncnnwin;
void initncnn();

int main(){
	Fl::lock();
	Fl_Window* win=new Fl_Window(0,20,600,400);    
	initncnn(); 
	win->add(ncnnwin);
	ncnnwin->position(600-320,400-240);
	win->show();
	ncnnwin->show();

	return Fl::run();
}