#include <string>
#include <unordered_map>

#include <FL/Fl_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Button.H>
#include "Fl_Scintilla.h"
    
#include "general.hpp"

inline void child_to_local(Fl_Group* wd) {
    int gx = wd->x();
    int gy = wd->y();

    if (wd->parent()) {
        gx += wd->parent()->x();
        gy += wd->parent()->y();
    } 
    for (int i = 0; i < wd->children(); ++i) {
        Fl_Widget *o = wd->child(i);
        // cotm(o->label());
        o->position(o->x() + gx, o->y() + gy);
    }
}

struct FileEntry {
    std::string filename;
    std::time_t modified;

    // Sort by modified time (ascending)
    bool operator<(const FileEntry& other) const {
        return modified > other.modified;
    }

    // Sort by filename (lexicographically)
    bool operator==(const FileEntry& other) const {
        return filename == other.filename;
    }
};



#include <FL/Fl_Menu_Button.H>

// compile: g++ -std=c++17 `fltk-config --cxxflags --ldflags` menu_add_on_click.cpp -o menu_add_on_click

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_Item.H>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>


// dynamic_options_playmenu.cpp
// compile: g++ -std=c++17 `fltk-config --cxxflags --ldflags` dynamic_options_playmenu.cpp -o dynamic_options_playmenu

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <cstring>
#include <cstdio>

// precise_options_region.cpp
// compile: g++ -std=c++17 `fltk-config --cxxflags --ldflags` precise_options_region.cpp -o precise_options_region

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_Item.H>
#include <cstdio>
#include <cstring>
#include <cstdlib>

class MyMenuBar : public Fl_Menu_Bar {
public:
    using Fl_Menu_Bar::Fl_Menu_Bar;

protected:
 static void item_cb(Fl_Widget* w, void* data) {
	 Fl_Menu_Bar* menu = (Fl_Menu_Bar*)w;
	 Fl_Menu_Item* item = (Fl_Menu_Item*)menu->mvalue();
	 printf("selected: %s\n", static_cast<char*>(data));
	 bool clicked_on_checkbox = (Fl::event_x() <= 25);
	 if (clicked_on_checkbox) {
		 menu->play_menu(item);
		 return;
	 }
 }

	// approximate horizontal padding FLTK uses around each label
    static constexpr int H_PAD = 14; // ~7px left + ~7px right

    int handle(int ev) override {
        if (ev == FL_PUSH) {
            int ex = Fl::event_x();    // x relative to this widget
            // find index of the "Functions" header in the flat menu array
            int opt_idx = find_index("Functions");
            if (opt_idx < 0) return Fl_Menu_Bar::handle(ev);

            // access the internal flat Fl_Menu_Item array
            const Fl_Menu_Item* items = this->menu();
            if (!items) return Fl_Menu_Bar::handle(ev);

            // compute the x origin of the "Functions" header by summing widths of preceding top-level headers
            int origin_x = this->x(); // left edge of menubar in parent coords
            fl_font(FL_HELVETICA, 14); // use same font/size as the menu bar (adjust if your menubar uses different)
            for (int i = 0; i < opt_idx; ++i) {
                if (!items[i].text) break;
                // Treat entries with FL_SUBMENU as top-level headers
                if (items[i].flags & FL_SUBMENU) {
                    origin_x += static_cast<int>(fl_width(items[i].text)) + H_PAD;
                }
            }

            // header width
            int hdr_w = static_cast<int>(fl_width(items[opt_idx].text)) + H_PAD;

            // check whether click X falls inside the computed header rectangle
            if (ex >= origin_x && ex < origin_x + hdr_w) {
                // Click landed on "Functions"
                static int counter = 0;
                ++counter;

				clear_submenu(opt_idx);

                char buf[64];
                std::snprintf(buf, sizeof(buf), "Functions/Item %d", counter);
                char* path = strdup(buf); // persist string for FLTK

                this->add(path, 0, item_cb, path,FL_MENU_TOGGLE  );
                this->menu_end();
                this->redraw();

                Fl_Menu_Item* child = (Fl_Menu_Item*)this->find_item(path);
                if (child) this->play_menu(child);

                return 1; // we handled the click
            }
        }
        return Fl_Menu_Bar::handle(ev);
    }
};

// int main(int argc, char** argv) {
//     Fl_Window win(640, 240, "Precise Functions Region");
//     MyMenuBar* mb = new MyMenuBar(0, 0, 640, 30);

//     // create some left headers so "Functions" won't necessarily be first
//     mb->add("File/New", 0, nullptr);
//     mb->add("Edit/Copy", 0, nullptr);
//     mb->add("View/Zoom", 0, nullptr);

//     // create Functions header
//     mb->add("Functions", 0, 0, 0, FL_SUBMENU);

//     mb->menu_end();
//     win.end();
//     win.show(argc, argv);
//     return Fl::run();
// }


// int main(int argc, char** argv) {
//     Fl_Window win(520, 240, "Dynamic Functions Example");
//     MyMenuBar* menubar = new MyMenuBar(0, 0, 520, 30);

//     menubar->add("File/New");
//     menubar->add("File/Open");

//     // create "Functions" header as an (initially empty) submenu header
//     menubar->add("Functions", 0, 0, 0, FL_SUBMENU);
//     menubar->menu_end();

//     win.end();
//     win.show(argc, argv);
//     return Fl::run();
// }



// int main(int argc, char** argv) {
//     Fl_Window win(640, 200, "Add items on Functions click");

//     MyMenuBar menubar(0, 0, 640, 30);
//     // create some initial menu groups
//     menubar.add("File/New", 0, nullptr);
//     menubar.add("File/Open", 0, nullptr);

//     // create an Functions header (as a submenu container with no children initially)
//     menubar.add("Functions", 0, nullptr, 0, FL_SUBMENU);

//     // add a control item to demonstrate selection of created items is handled
//     menubar.add("Control/Demo", 0, [](Fl_Widget*, void*){ puts("Demo pressed"); });

//     menubar.menu_end();

//     win.end();
//     win.show(argc, argv);
//     return Fl::run();
// }


#include <vector>
#include <tuple>
 struct fl_scintilla : public Fl_Scintilla {  
    std::string filename="";
	std::string floaded="";
	bool show_browser=1;
	std::string folder="lua/";
    sptr_t curr_file_pointer=0;
    std::string comment;
	void toggle_comment();
    std::unordered_map<sptr_t,uptr_t> filesfirstline;  
    // std::unordered_map<sptr_t,uptr_t> filescaret;  
    fl_scintilla(int X, int Y, int W, int H, const char* l = 0);
    // void resize(int x, int y, int w, int h) override;
    int handle(int e)override;
	std::string getSelected();
	std::string getalltext();

	std::tuple<int,int> csearch(const char* needle, bool dirDown = true, int flags = SCFIND_MATCHCASE);
    void searchshow(); 

	std::vector<FileEntry> list_files_in_dir(const std::string path);
	void update_menu();
	MyMenuBar* fmb;
	void move_item(Fl_Browser* browser, std::string str);

    void save();
    void setnsaved();
    void set_lua(); 
    Fl_Window * navigator;  
    // Fl_Group * navigator;  
    Fl_Button* btntop; 
    void helperinit();
    Fl_Browser* bfiles;
    Fl_Browser* bfilesmodified=0;
    Fl_Browser* bfunctions=0;
    vint vline;
    void getfuncs();
    void navigatorSetUpdated();
    std::vector<FileEntry> lfiles; 
};


