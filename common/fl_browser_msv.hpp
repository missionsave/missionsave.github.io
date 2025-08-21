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
	// std::vector<
	int line_height;
    fl_browser_msv(int X, int Y, int W, int H, const char *L = 0)
        : Fl_Browser(X, Y, W, H, L) {}

	int handle(int event) override;
	void triggerCallback(std::function<void(void* data_ptr,int)> callbackFunc) {
		callbackFunc(data_ptr,99);
	}
	    int item_height(void *) const override {
        return 18; // Set exact pixel height for each line
    }
};