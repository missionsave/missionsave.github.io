#include <string>
#include <vector>
#include <functional>

// #include <FL/Fl_Window.H>
// #include <FL/Fl_Group.H>
#include <FL/Fl_Browser.H> 
    
#include "general.hpp"




class fl_browser_msv : public Fl_Browser {
public:
	void* data_ptr=0;
    std::function<void(void* data_ptr, int)> callbackFunc;
	// std::vector<
	int line_height;
	int tclicked=-1;
    fl_browser_msv(int X, int Y, int W, int H, const char *L = 0)
        : Fl_Browser(X, Y, W, H, L) {}

	int handle(int event) override;
	// void triggerCallback(std::function<void(void* data_ptr,int)> callbackFunc) {
	// 	callbackFunc(data_ptr,tclicked);
	// }

    void setCallback(std::function<void(void* data_ptr, int)> func) {
        callbackFunc = func;
    }
	int item_height(void *) const override {
        return 18;
    }
};