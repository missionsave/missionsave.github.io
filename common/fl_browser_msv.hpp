#include <string>
#include <vector>
#include <functional>

// #include <FL/Fl_Window.H>
// #include <FL/Fl_Group.H>
#include <FL/Fl_Browser.H> 
#include <FL/fl_draw.H>
    
#include "general.hpp"

struct cols{
	int width;
	std::string prefix="";
	std::string prefix_on="";
};


class fl_browser_msv : public Fl_Browser {
public:
	int* cols_arr=0;
	void* data_ptr=0;
    std::function<void(void* data_ptr, int, void* self)> callbackFunc;
	std::vector<cols> vcols;
	int line_height;
	int tclicked=-1;
	bool isrightclick=0;
    fl_browser_msv(int X, int Y, int W, int H, const char *L = 0)
        : Fl_Browser(X, Y, W, H, L) {has_scrollbar(Fl_Browser::VERTICAL);}

	int handle(int event) override;
	// void triggerCallback(std::function<void(void* data_ptr,int)> callbackFunc) {
	// 	callbackFunc(data_ptr,tclicked);
	// }

	std::vector<std::vector<std::string>> cache;
	std::vector<std::vector<bool>> on_off;

	std::string msel = "@b@u";
	void toggleon_v1(int line,int code,bool on=1, bool selectit=1) {
		// if(code==2)return;
		on_off[line-1][code]=on;
		if (on) {
			std::stringstream strm;
			lop(i, 0, cache[line - 1].size()) {
				strm << ((code==i)?(vcols[i].prefix_on):(vcols[i].prefix) ) << ((selectit)?msel:"") << cache[line - 1][i];
				if (i < cache[line - 1].size() - 1) strm << "\t";
			}
			text(line, strm.str().c_str());
		}else{ //here not working well
			std::stringstream strm;
			lop(i, 0, cache[line - 1].size()) {
				strm << ((code==i)?(vcols[i].prefix):(vcols[i].prefix_on) ) << ((selectit)?msel:"") << cache[line - 1][i];
				if (i < cache[line - 1].size() - 1) strm << "\t";
			}
			text(line, strm.str().c_str());

		}
	}

	void toggleon(int line, int code, bool on = true, bool selectit=1) {
    on_off[line - 1][code] = on;

    std::stringstream strm;
    lop(i, 0, cache[line - 1].size()) {
        if (code == i) {
            // Only the selected column changes
            strm << (on ? vcols[i].prefix_on : vcols[i].prefix);
        } else {
            // All others stay in their normal prefix
            strm << vcols[i].prefix;
        }
        strm << ((selectit)?msel:"") << cache[line - 1][i];
        if (i < cache[line - 1].size() - 1) strm << "\t";
    }
    text(line, strm.str().c_str());
}



	void addnew(vstring vadd){
		on_off.push_back(vbool(vadd.size()));
		cache.push_back(vadd);
		std::stringstream strm;
		lop(i,0,vadd.size()){
			strm<<vcols[i].prefix<<vadd[i];
			if(i<vadd.size()-1)strm<<"\t";
		}
		add(strm.str().c_str());
	}

	void insert_at(int line, vstring vadd) {
		// Clamp line index to valid range
		if (line < 0) line = 0;
		if (line > (int)cache.size()) line = cache.size();

		// Insert into your parallel data structures
		on_off.insert(on_off.begin() + line, vbool(vadd.size()));
		cache.insert(cache.begin() + line, vadd);

		// Build the display string
		std::stringstream strm;
		lop(i, 0, vadd.size()) {
			strm << vcols[i].prefix << vadd[i];
			if (i < vadd.size() - 1) strm << "\t";
		}

		// Insert into the FLTK widget at the given line
		insert(line, strm.str().c_str()); 
		// ^ Assuming your widget has an insert(row, text) method
	}


	void clear_all(){
		on_off.clear();
		cache.clear();
		clear();
	}
	void init(){
		if(cols_arr || vcols.size()==0)return;

		lop(i,0,vcols.size()){
			if(vcols[i].prefix_on=="")vcols[i].prefix_on=vcols[i].prefix;
		}

		cols_arr = new int[vcols.size() + 1];

		// Copy elements from vector
		for (size_t i = 0; i < vcols.size(); ++i) {
			cols_arr[i] = vcols[i].width;
		}

		// Add the trailing zero
		cols_arr[vcols.size()] = 0;

		// Test output
		// for (size_t i = 0; i < v.size() + 1; ++i) {
		// 	std::cout << cols_arr[i] << " ";
		// }
		column_widths(cols_arr);
	}
    void setCallback(std::function<void(void* data_ptr, int, void* self)> func) {
        callbackFunc = func;
    }
	int item_height(void *) const override {
        return 18;
    }
	};