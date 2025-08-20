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

#include <vector>
#include <tuple>
 struct fl_scintilla : public Fl_Scintilla {  
    std::string filename="";
	std::string floaded="";
	bool show_browser=1;
	std::string folder="lua/";
    sptr_t curr_file_pointer=0;
    std::string comment;
    std::unordered_map<sptr_t,uptr_t> filesfirstline;  
    // std::unordered_map<sptr_t,uptr_t> filescaret;  
    fl_scintilla(int X, int Y, int W, int H, const char* l = 0);
    // void resize(int x, int y, int w, int h) override;
    int handle(int e)override;
	std::string getSelected();

	std::tuple<int,int> csearch(const char* needle, bool dirDown = true, int flags = SCFIND_MATCHCASE);
    void searchshow(); 

	std::vector<FileEntry> list_files_in_dir(const std::string path);
	void update_menu();
	Fl_Menu_Bar* fmb;
	void move_item(Fl_Browser* browser, std::string str);

    void save();
    void setnsaved();
    void set_lua(); 
    Fl_Window * navigator;  
    // Fl_Group * navigator;  
    Fl_Button* btntop; 
    void helperinit();
    Fl_Browser* bfiles;
    Fl_Browser* bfilesmodified;
    Fl_Browser* bfunctions;
    vint vline;
    void getfuncs();
    void navigatorSetUpdated();
    std::vector<FileEntry> lfiles; 
};


