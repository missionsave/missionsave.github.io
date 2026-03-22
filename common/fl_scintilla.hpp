
#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Menu_Button.H> 
#include <FL/Fl_Window.H>

#include <string>
#include <unordered_map>
#include <functional>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <tuple>
#include <vector>

#include "Fl_Scintilla.h"
#include "general.hpp"

struct _FileEntry {
	std::string filename;
	std::time_t modified;
    std::time_t accesstime;  
};
 
struct fl_scintilla : public Fl_Scintilla {
	std::function<void()> callbackOnload;
	std::string filename = "";
	std::string floaded = "";
	bool show_browser = 1;
	std::string folder = "lua/";
	std::vector<std::string> tail_functions;
	sptr_t curr_file_pointer = 0;
	std::unordered_map<sptr_t, uptr_t> filesfirstline;
	std::unordered_map<sptr_t,uptr_t> filescaret;
	std::string comment;
	fl_scintilla(int X, int Y, int W, int H, const char* l = 0);
	// void resize(int x, int y, int w, int h) override;
	int handle(int e) override;
	std::string getSelected();
	std::string getalltext();
	void toggle_comment();

	std::tuple<int, int> csearch(const char* needle, bool dirDown = true, int flags = SCFIND_MATCHCASE);
	void searchshow();

	std::vector<_FileEntry> list_files_in_dir(const std::string path);
	void update_menu();
	// void move_item(Fl_Browser* browser, std::string str);

	void save();
	void setnsaved();
	void set_lua();  
	vint vline;
	vstring vlinestring;
	vstring vtoggled;
	vstring getfuncs();
	std::string sfunctions = "";

	void navigatorSetUpdated();
	// std::vector<_FileEntry> lfiles;

	class MyMenuBar : public Fl_Menu_Bar {
	   public:
		fl_scintilla* parent;
		using Fl_Menu_Bar::Fl_Menu_Bar;

		MyMenuBar(int X, int Y, int W, int H, const char* L, fl_scintilla* p) : Fl_Menu_Bar(X, Y, W, H, L), parent(p) {}

	   protected:
		static void item_cb(Fl_Widget* w, void* data) {
			Fl_Menu_Bar* menu = (Fl_Menu_Bar*)w;
			fl_scintilla* editor = (fl_scintilla*)data;
			Fl_Menu_Item* item = (Fl_Menu_Item*)menu->mvalue();
			//  printf("selected: %s\n", static_cast<char*>(data));
			bool clicked_on_checkbox = (Fl::event_x() <= 25);
			// item->set();
			cotm(item->value(),Fl::event_x())
			if (clicked_on_checkbox) {
				menu->play_menu(item);
				cotm(7)
					lop(i,0,editor->vtoggled.size()){
						cotm(editor->vtoggled[i])
					}
				return;
			}

			// Walk the flat menu array
			{  // Full path of the selected item, e.g. "File/Open"
				char path[256];
				menu->item_pathname(path, sizeof(path));  // e.g. "File/Open"

				printf("Path: %s\n", path);

				const char* slash = strchr(path, '/');
				if (slash) {
					printf("Submenu: %.*s\n", (int)(slash - path), path);
					printf("Item: %s\n", slash + 1);
				} else {
					printf("Item: %s\n", path);
				}
			}

			// Functions
			// fl_scintilla* editor = (fl_scintilla*)data;
			// Fl_Browser* b=editor->bfunctions;
			int sel = -1;
			// int sel=b->value()-1;

			lop(i, 0, editor->vlinestring.size()) {
				if (editor->vlinestring[i] == item->label()) {
					sel = i;
					break;
				}
			}
			if (sel == -1) return;

			// cotm(editor->vline.size(),item->label())
			if (sel < editor->vline.size()) {
				// cotm(vs->vline[sel])
				int line = editor->vline[sel];	// line you want at the top (0-based)

				// Get visual line number (taking wrap into account)
				int visual_line = editor->SendEditor(SCI_VISIBLEFROMDOCLINE, line);

				// Get current first visible visual line
				int current_visual_top = editor->SendEditor(SCI_GETFIRSTVISIBLELINE);

				// Calculate how many lines to scroll
				int delta = visual_line - current_visual_top;

				// Scroll so the line is at the top
				editor->SendEditor(SCI_LINESCROLL, 0, delta);

				// Optionally move caret there too
				int pos = editor->SendEditor(SCI_POSITIONFROMLINE, line);
				editor->SendEditor(SCI_SETSEL, pos, pos);
			}
		}

		// approximate horizontal padding FLTK uses around each label
		static constexpr int H_PAD = 14;  // ~7px left + ~7px right

		int handle(int ev) override {
			// return Fl_Menu_Bar::handle(ev);
			if (ev == FL_PUSH) {
				int ex = Fl::event_x();	 // x relative to this widget
				// find index of the "Functions" header in the flat menu array
				int opt_idx = find_index("Functions");
				if (opt_idx < 0) return Fl_Menu_Bar::handle(ev);

				// access the internal flat Fl_Menu_Item array
				const Fl_Menu_Item* items = this->menu();
				if (!items) return Fl_Menu_Bar::handle(ev);

				// compute the x origin of the "Functions" header by summing widths of preceding top-level headers
				int origin_x = this->x();  // left edge of menubar in parent coords
				fl_font(FL_HELVETICA,
						14);  // use same font/size as the menu bar (adjust if your menubar uses different)
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
					char* path;

					// 				vstring val=parent->getfuncs();
					// cotm(val)
					// 				lop(i,0,val.size()){

					//                 char buf[64];
					//                 std::snprintf(buf, sizeof(buf), "%s", val[i].c_str());

					//                 // std::snprintf(buf, sizeof(buf), "Functions/Item %d", i);
					//                 path = strdup(buf); // persist string for FLTK
					// 				// path=(char*)val[i].c_str();

					//                 this->add(path, 0, item_cb, path,FL_MENU_TOGGLE  );
					// 				}

					vstring val = parent->getfuncs();
					if (val.size() < 2) return 0;
					// cotm(val);

					for (size_t i = 0; i < val.size() - 1; ++i) {
						char buf[64];
						std::snprintf(buf, sizeof(buf), "Functions/%s", val[i].c_str());

						path = strdup(buf);	 // persist string for FLTK

						this->add(path, 0, item_cb, (void*)parent, FL_MENU_TOGGLE);
					}

					this->menu_end();
					this->redraw();

					Fl_Menu_Item* child = (Fl_Menu_Item*)this->find_item(path);
					if (child) this->play_menu(child);

					return 1;  // we handled the click
				}
			}
			return Fl_Menu_Bar::handle(ev);
		}
	};
	
	std::vector<std::string> hints; 
	MyMenuBar* fmb=nullptr;
};
