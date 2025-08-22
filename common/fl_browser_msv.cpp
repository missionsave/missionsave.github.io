#include "fl_browser_msv.hpp"
#include <FL/Fl.H>
#include <FL/fl_draw.H> 
#include <sstream>
using namespace std; 


int fl_browser_msv::handle(int event){
	// if(event==FL_RELEASE){
	if(event==FL_PUSH){
		int scrollPos = position();
		line_height = full_height()/ size();
		int relative_y = Fl::event_y() - this->y() +scrollPos;
		int clicked_line = relative_y / line_height;
		clicked_line++;

		// cout<<full_height()<<endl;
		// cout<<selected(2)<<endl;
		int x = Fl::event_x() - this->x();
		int y = Fl::event_x() - this->y();
		tclicked = x;
		// select(clicked_line);  /// test only

		string msel="@b@u";
		lop(i, 1, size()+1) {
			std::string s = text(i);
			if (s.rfind(msel, 0) == 0) {  // check if starts with msel
				s.erase(0, msel.size());			  // remove firsts
			}
			text(i,s.c_str());
		}

		stringstream strm;
		strm<<msel<<this->text(clicked_line); 

		this->text(clicked_line,strm.str().c_str());

		if (clicked_line > 0 && clicked_line <= size()) {
			data_ptr = data(clicked_line);
			if (callbackFunc) {
				callbackFunc(data_ptr, x);  // pass clicked line index
			}
			// if (data_ptr) {
			//     // Cast back to your original type
			//     WifiEntry* obj = static_cast<WifiEntry*>(data_ptr);
			// 	cout<<obj->ssid<<endl;
			// }
		}

	// 	break;
	// 		// int line=
    //         int selected = value(); // 1-based index of selected line
	// 		// cout<<line<<endl;
	// 		// cout<<selected<<endl;
    //         if (selected > 0) {
    //             const char* item_text = text(selected);
    //             std::cout << x<<" Selected line: " << selected
    //                       << ", Text: " << (item_text ? item_text : "(null)") << std::endl;
	// 			// this->text(selected, "New content for line 3");
    //         }
    //         break;
    }
    return Fl_Browser::handle(event);
}
	

    
    // // Override item height for narrow spacing
    // int fl_browser_msv::item_height(void *item) {
    //     return line_height;
    // }
    
    // // Custom drawing to fit text in narrow space
    // void fl_browser_msv::item_draw(void *item, int X, int Y, int W, int H){
    //     const char *text = (const char *)item;
    //     if (text) {
    //         // Draw text vertically centered in the narrow space
    //         // fl_font(FL_HELVETICA, 10); // Small font
    //         fl_draw(text, X + 2, Y + (20 + 8) / 2);
    //     }
    // }
