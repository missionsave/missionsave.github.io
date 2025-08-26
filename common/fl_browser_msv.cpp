#include "fl_browser_msv.hpp"
#include <FL/Fl.H>
#include <FL/fl_draw.H> 
#include <sstream>
using namespace std; 

// std::string strip_fltk_format(const char* src) {
//     std::string out;
//     if (!src) return out;

//     for (const char* p = src; *p; ) {
//         if (*p == '@') {
//             ++p; // skip '@'
//             // skip all format code chars (letters, digits, punctuation allowed by FLTK)
//             while (*p && (std::isalnum((unsigned char)*p) || *p == '_' || *p == '-'))
//                 ++p;
//         } else {
//             out.push_back(*p++);
//         }
//     }
//     return out;
// }
int fl_browser_msv::handle(int event){ 
	if(event==FL_PUSH){
		int scrollPos = position();
		line_height = full_height()/ size();
		int relative_y = Fl::event_y() - this->y() +scrollPos;
		int clicked_line = relative_y / line_height;
		clicked_line++;

		cotm(clicked_line,cache.size());
		if(clicked_line>cache.size())return 1;

		// cout<<full_height()<<endl;
		// cout<<selected(2)<<endl;
		int x = Fl::event_x() - this->x();
		int y = Fl::event_x() - this->y();
		tclicked = x;
		// select(clicked_line);  /// test only


// for (int i = 1; i <= size(); ++i) {
//     int col = 0;
//     size_t prefix_len = vcols[col].prefix.size();

//     const char* rowtxt = text(i);
//     if (!rowtxt) continue;

//     std::string s(rowtxt);

//     if (s.find(msel, prefix_len) == prefix_len) {
//         s.erase(prefix_len, msel.size());
//     }

//     text(i, s.c_str());
// }

		//remove msel
		for (int i = 1; i <= size(); ++i) {
			string toreplace=text(i);
			replace_All(toreplace,msel,"");
			text(i, toreplace.c_str());
		}

		// string msel="@b@u";
		// lop(i, 1, size()+1) {
		// 	std::string s = text(i);
		// 	if (s.rfind(msel, 0) == 0) {  // check if starts with msel
		// 		s.erase(0, msel.size());			  // remove firsts
		// 	}
		// 	text(i,s.c_str());
		// }
		// string str=strip_fltk_format(text(clicked_line));

		stringstream strm;
		lop(i,0,cache[clicked_line-1].size()){
		strm<<vcols[i].prefix<<msel<<cache[clicked_line-1][i]; 
		if(i<cache[clicked_line-1].size()-1)strm<<"\t";
		}
		text(clicked_line,strm.str().c_str());


		int code = -1;
		int tt = 0;
		for (size_t i = 0; i < vcols.size(); ++i) {
			tt += vcols[i].width; // typo fixed: width, not witdh
			if (x < tt || i == vcols.size() - 1) { // last column catches all
				code = static_cast<int>(i);
				break;
			}
		}

		if (clicked_line > 0 && clicked_line <= size()) {
			data_ptr = data(clicked_line);
			if (callbackFunc) {
				int idx = clicked_line - 1;
				on_off[idx][code] = !on_off[idx][code];
				int on = on_off[idx][code];
				if (on) {
					stringstream strm;
					lop(i, 0, cache[clicked_line - 1].size()) {
						strm << ((code==i)?(vcols[i].prefix_on):(vcols[i].prefix) ) << msel << cache[clicked_line - 1][i];
						if (i < cache[clicked_line - 1].size() - 1) strm << "\t";
					}
					text(clicked_line, strm.str().c_str());
				}
				callbackFunc(data_ptr, code,(void*)this);  // pass clicked line index
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
