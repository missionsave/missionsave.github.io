#pragma region Includes_globals

#ifndef SC_CP_UTF8
#define SC_CP_UTF8 65001
#endif
#ifndef RGB
#define RGB(r, g, b) ((int)(((unsigned char)(r) | ((unsigned short)((unsigned char)(g)) << 8)) | (((unsigned int)(unsigned char)(b)) << 16)))
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#ifdef __MWERKS__
# define FL_DLL
#endif

#include "Scintilla.h"
#include "Fl_Scintilla.h"

#include <FL/Fl.H>
#include <FL/x.H> // for fl_open_callback
#include <FL/Fl_Group.H>
#include <FL/Fl_Double_Window.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/filename.H> 
#include <FL/Fl_Input_Choice.H> 

#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <stack>
#include <cctype>

#include <filesystem>
#include <unordered_map>
// #include <tuple>

#include <chrono>
#include <algorithm>

namespace fs = std::filesystem;

#include "general.hpp"

#include "fl_scintilla.hpp"

#define MARGIN_FOLD_INDEX 3

using namespace Scintilla;
using namespace std;

static void cb_editor(Scintilla::SCNotification *scn, void *data); 
struct fl_scintilla;
void setscint(fl_scintilla* editor,string filename);

struct sfiles{
	sptr_t p;
	bool notsaved=0;
};
unordered_map<string,sfiles> files;
bool loading=0;


//  fc-scan "DejaVuSans-Bold.ttf" | grep family
//  fc-scan "Cascadia Mono PL SemiBold 600.otf" | grep family
//         family: "Cascadia Mono PL"(s) "Cascadia Mono PL SemiBold"(s)
//         familylang: "en"(s) "en"(s)
std::string load_app_font(const std::string& filename);


#pragma endregion Includes_globals

#pragma region find

class FixedSubWindow;
FixedSubWindow* wFind=0;

class FixedSubWindow : public Fl_Window {
public:
class searchinput : public Fl_Input {
public:
	FixedSubWindow* wFind; 
    bool showing_placeholder = true;
    const char* placeholder_text = "Find";
    searchinput(int x, int y, int w, int h, const char* label = 0)
        : Fl_Input(x, y, w, h, label) { 
		box(FL_FLAT_BOX);         // Removes 3D bevel
		color(FL_WHITE);          // Sets background to white
		textcolor(FL_BLACK);      // Optional: text color
		textfont(FL_HELVETICA_BOLD);   // Optional: font
		textsize(14);
    }

    int handle(int event) override {
		// cotm(event,FL_FOCUS,FL_SHOW,FL_ENTER);
		if(event==FL_FOCUS){
			bool nothing=strlen(value()) == 0;
			if (nothing)value(placeholder_text);
			position(0);                    // Move cursor to start
			mark(strlen(value()));
			if(!nothing && wFind){
				wFind->scint->SendEditor(SCI_HOME);
				// wFind->scint->SendEditor(SCI_LINEUP);
				cotm("callb")
				wFind->bdown->do_callback(); 
			}
			cotm(value())
			return 1;
		}

 
        return Fl_Input::handle(event);
    }
};


    int fixed_w, fixed_h;
	searchinput* search;
	fl_scintilla* scint;
	Fl_Box* tmi;
	Fl_Button* bdown;
	Fl_Button* bup;

    FixedSubWindow(int X, int Y, int W, int H, const char* L = 0)
        : Fl_Window(X, Y, W, H, L), fixed_w(W), fixed_h(H) {
			Fl_Button* bclose=new Fl_Button(w()-21,1,20,20,"x"); 
			bclose->callback([](Fl_Widget *widget, void* v){
				((FixedSubWindow*)v)->scint->take_focus();
				// focus(((FixedSubWindow*)v)->parent());
				((FixedSubWindow*)v)->hide();
			},this);

			search=new searchinput(1,1,w()*0.45,20); 
			search->wFind=this; 
			// search->box(FL_FLAT_BOX);         // Removes 3D bevel
			// search->color(FL_WHITE);          // Sets background to white
			// search->textcolor(FL_BLACK);      // Optional: text color
			// search->textfont(FL_HELVETICA_BOLD);   // Optional: font
			// search->textsize(14);
			// search->value("Find");
			int xs=search->w()+1;
			bup=new Fl_Button(xs,1,20,20,"@-38->"); 
			bup->labelcolor(FL_RED);
			// bup->labeltype(FL_NORMAL_LABEL);
			bdown=new Fl_Button(xs+20,1,20,20,"@-32->");
			bdown->labelcolor(FL_RED);
			// bdown->labeltype(FL_NORMAL_LABEL);
			bdown->callback([](Fl_Widget *widget, void* v){
				FixedSubWindow* wFind=((FixedSubWindow*)v);
				auto [totalMatches, matchIndex] = wFind->scint->csearch( wFind->search->value(),1  );
				string tm=to_string(matchIndex+1)+"/"+to_string(totalMatches);
				wFind->tmi->copy_label(tm.c_str());
			},this);
			bup->callback([](Fl_Widget *widget, void* v){
				FixedSubWindow* wFind=((FixedSubWindow*)v);
				auto [totalMatches, matchIndex] = wFind->scint->csearch( wFind->search->value(),0 );
				string tm=to_string(matchIndex+1)+"/"+to_string(totalMatches);
				wFind->tmi->copy_label(tm.c_str());
			},this);
			tmi=new Fl_Box(xs+20+20,1,20,20);
			// tmi->label("1/4");

			
			// search->callback([](Fl_Widget *widget, void* v){
			// 	FixedSubWindow* wFind=((FixedSubWindow*)v);
			// 	wFind->scint->csearch( wFind->search->value(),1  );

			// },this);
 
		}
	int handle(int event) override {
		// cotm(event,FL_SHOW);
			if (event==FL_SHOW) {
			}
		if(event==FL_KEYDOWN && Fl::event_key()==FL_Escape){ 
			if(wFind)wFind->hide();
		}
		return Fl_Window::handle(event); 
	}

    void resize(int X, int Y, int W, int H) override {
        // Ignore W and H, keep fixed size 
        Fl_Window::resize(scint->x()+scint->w()-fixed_w-16, Y, fixed_w, fixed_h);
        // Fl_Window::resize(parent()->x()+parent()->w()-fixed_w-16, Y, fixed_w, fixed_h);
    }
};
void fl_scintilla::searchshow(){
	if(!wFind){
		Fl_Group::current(parent()); // important
		int sz=190;
		wFind=new FixedSubWindow(x()+w()-sz-16,22,sz,22,"@25-> ⚲");
		wFind->scint=this; 
		wFind->color(FL_RED);
		// cotm("findinit")	 
	}  
	string gs=getSelected(); 
	wFind->show();
	// wFind->wait_for_expose ();
	wFind->scint=this;
	wFind->search->value(gs.c_str());
	wFind->search->take_focus();
}
std::tuple<int,int> fl_scintilla::csearch(const char* needle, bool dirDown, int flags ) {
    if (!needle || !*needle) return {0, -1};

    const int docLen   = SendEditor(SCI_GETTEXTLENGTH);
    const int matchLen = (int)strlen(needle);

    if (matchLen <= 0 || docLen <= 0) return {0, -1};

    // Collect all non-overlapping match starts
    std::vector<int> posList;
    SendEditor(SCI_SETSEARCHFLAGS, flags, 0);
    SendEditor(SCI_SETTARGETSTART, 0, 0);
    SendEditor(SCI_SETTARGETEND, docLen, 0);

    int pos = SendEditor(SCI_SEARCHINTARGET, matchLen, (sptr_t)needle);
    while (pos != -1) {
        posList.push_back(pos);
        SendEditor(SCI_SETTARGETSTART, pos + matchLen, 0);
        SendEditor(SCI_SETTARGETEND, docLen, 0);
        pos = SendEditor(SCI_SEARCHINTARGET, matchLen, (sptr_t)needle);
    }

    if (posList.empty()) return {0, -1};

    // Use selection bounds to avoid re-selecting the same match
    const int selStart = SendEditor(SCI_GETSELECTIONSTART, 0, 0);
    const int selEnd   = SendEditor(SCI_GETSELECTIONEND, 0, 0);

    // Choose next index symmetrically (VSCode-style)
    int idx = -1;
    if (dirDown) {
        // Next is the first match whose start is >= selection end
        for (int i = 0; i < (int)posList.size(); ++i) {
            if (posList[i] >= selEnd) { idx = i; break; }
        }
        if (idx == -1) idx = 0; // wrap to first
    } else {
        // Previous is the last match whose start is < selection start
        for (int i = (int)posList.size() - 1; i >= 0; --i) {
            if (posList[i] < selStart) { idx = i; break; }
        }
        if (idx == -1) idx = (int)posList.size() - 1; // wrap to last
    }

    const int foundPos = posList[idx];

    // Move and select
    SendEditor(SCI_GOTOPOS, foundPos, 0);
    SendEditor(SCI_SETSEL, foundPos, foundPos + matchLen);

    // Return (total matches, 0-based index of the current hit)
    return { (int)posList.size(), idx };
}


#pragma endregion find

#include <FL/Fl_Menu_Bar.H>
void menu_callback(Fl_Widget* w, void* data) {
    Fl_Menu_Item* item = (Fl_Menu_Item*)data;
    printf("You selected: %s\n", item->label());
}

class FixedHeight_Menu_Bar : public Fl_Menu_Bar {
public:
    FixedHeight_Menu_Bar(int X, int Y, int W, int H, const char* L = 0)
        : Fl_Menu_Bar(X, Y, W, H, L) {}
    
    void resize(int X, int Y, int W, int H) override {
        // Keep original Y position and fixed height (22)
        Fl_Menu_Bar::resize(X, Y, W, 22);
    }
};

fl_scintilla::fl_scintilla(int X, int Y, int W, int H, const char* l): Fl_Scintilla(X, Y, W, H, l) {	
	SendEditor(SCI_SETCODEPAGE, SC_CP_UTF8, 0);
    
	// floaded = load_app_font("Cascadia Mono PL SemiBold 600.otf");

	floaded = Fl::get_font(FL_HELVETICA_BOLD);
	cotm(floaded)
	floaded = Fl::get_font_name(FL_HELVETICA);
	floaded = Fl::get_font_name(FL_COURIER_BOLD);
	// floaded = Fl::get_font_name(FL_HELVETICA_BOLD);
	cotm(floaded)
	// string fstd=load_app_font("DejaVuSans-Bold.ttf");
	// string fstd=load_app_font("DejaVuSans.ttf");
	// Fl::set_font(FL_HELVETICA, fstd.c_str());
	// Fl::set_font(FL_HELVETICA, "Noto Color Emoji");




    set_lua();
	cotm("set_lua")
    SetNotify(cb_editor, this); 	
	cotm("cb_editor")
    // helperinit();
	// cotm("helperinit")
	update_menu();
	SendEditor(SCI_AUTOCSETAUTOHIDE,0);
	cotm("SCI_AUTOCSETAUTOHIDE")

}
void fl_scintilla::toggle_comment() {
    int doc_length = SendEditor(SCI_GETLENGTH);
    if (doc_length <= 0) return;

    int start = SendEditor(SCI_GETSELECTIONSTART);
    int end = SendEditor(SCI_GETSELECTIONEND);
    if (start < 0 || end < 0 || start > doc_length || end > doc_length) {
        return;
    }

    int start_line = SendEditor(SCI_LINEFROMPOSITION, start);
    int end_line   = SendEditor(SCI_LINEFROMPOSITION, end);
    if (start_line < 0 || end_line < 0) return;

    if (start_line == end_line && start == end) {
        end_line = start_line;
    }

    SendEditor(SCI_BEGINUNDOACTION);

    bool all_commented = true;

    // --- First pass: check if every line is already commented ---
    for (int line = start_line; line <= end_line; line++) {
        int pos = SendEditor(SCI_POSITIONFROMLINE, line);
        int line_end = SendEditor(SCI_GETLINEENDPOSITION, line);
        int line_length = line_end - pos;
        if (line_length <= 0) continue;

        // Skip whitespace at start
        int non_ws_pos = SendEditor(SCI_GETLINEINDENTPOSITION, line);

        char buf[16] = {0};
        TextRange tr;
        tr.chrg.cpMin = non_ws_pos;
        tr.chrg.cpMax = non_ws_pos + (int)comment.size();
        tr.lpstrText = buf;
        SendEditor(SCI_GETTEXTRANGE, 0, (sptr_t)&tr);

        if (strncmp(buf, comment.c_str(), comment.size()) != 0) {
            all_commented = false;
            break;
        }
    }

    // --- Second pass: apply action to all lines ---
    for (int line = start_line; line <= end_line; line++) {
        int pos = SendEditor(SCI_POSITIONFROMLINE, line);
        int line_end = SendEditor(SCI_GETLINEENDPOSITION, line);
        int line_length = line_end - pos;
        if (line_length <= 0) continue;

        int non_ws_pos = SendEditor(SCI_GETLINEINDENTPOSITION, line);

        if (all_commented) {
            // Remove comment
            char buf[16] = {0};
            TextRange tr;
            tr.chrg.cpMin = non_ws_pos;
            tr.chrg.cpMax = non_ws_pos + (int)comment.size();
            tr.lpstrText = buf;
            SendEditor(SCI_GETTEXTRANGE, 0, (sptr_t)&tr);

            if (strncmp(buf, comment.c_str(), comment.size()) == 0) {
                SendEditor(SCI_SETTARGETSTART, non_ws_pos);
                SendEditor(SCI_SETTARGETEND, non_ws_pos + (int)comment.size());
                SendEditor(SCI_REPLACETARGET, 0, (sptr_t)"");
            }
        } else {
            // Add comment at indent position
            SendEditor(SCI_INSERTTEXT, non_ws_pos, (sptr_t)comment.c_str());
        }
    }

    SendEditor(SCI_ENDUNDOACTION);
}




void fl_scintilla::update_menu(){
	    window()->begin(); 
		fmb=new Fl_Menu_Bar(x(),y(),w(),22);
		size(w(),h()-22);
		position(x(),y()+22);
	// FixedHeight_Menu_Bar* fmb=new FixedHeight_Menu_Bar(X,0,W,22);
	// fmb->add("File/Open", FL_ALT + 'o', [](Fl_Widget*, void*) {  });
	    // Initial menu items
    fmb->add("Files/Open folder",0, menu_callback);
    // fmb->add("Files/Quit", 0, menu_callback);



	lfiles=list_files_in_dir(folder);
	vector<FileEntry> &files=lfiles;
	// std::sort(files.begin(), files.end(), sortByFilename);
    // for (const auto& f : files) {
	// 	editor->bfiles->add(fs::path(f.filename).filename().string().c_str());
    //     // std::cout << f.filename << " - " << std::asctime(std::localtime(&f.modified));
    // }

	//open first last modified
    std::sort(files.begin(), files.end());
    for (const auto& f : files) {
		string str=(fs::path(f.filename).filename().string());
		cotm("FICH", folder+str);
		setscint(this,folder+str);
		break;


        // std::cout << std::asctime(std::localtime(&f.modified)) << " " << f.filename << '\n';
    }


    
    // Dynamically add more items later
    // fmb->add("Edits/Copy", FL_COMMAND + 'c', menu_callback);
    // fmb->add("Edits/Paste", FL_COMMAND + 'v', menu_callback);
}
bool isFilenameValid(const char *filename) {
    if (!filename) return false;
    size_t len = strlen(filename);
    if (len > 255) return false;
    try {
        return std::filesystem::exists(filename);
    } catch (...) {
        return false;
    }
}
string fl_scintilla::getSelected(){
	int start = SendEditor(SCI_GETSELECTIONSTART, 0, 0);
	int end = SendEditor(SCI_GETSELECTIONEND, 0, 0);
	int length = end - start;

	char* buffer = new char[length + 1];
	SendEditor(SCI_GETSELTEXT, 0, (sptr_t)buffer);

	std::string selectedText(buffer);
	delete[] buffer;
	return selectedText;
}

    //  void fl_scintilla::resize(int x, int y, int w, int h)  {
    //     // Redimensiona o box para manter margens de 10 px
    //    Fl_Window* p=window(); 
    // resize(x,y,w,h-44);
    // toggleSearchGroup->resize(x,y+h,w,44);
    // navigator->resize(x,y+h+44,w,p->h()-(y+h+44));

    //     // Fl_Window::resize(x, y, w, h);
    // }
	int fl_scintilla::handle(int e)  { 
		// if(e==FL_MOUSEWHEEL) {
        //         int dy = Fl::event_dy();  // positive = scroll down
		// 		cotm(dy)
        //         // offset += dy * 10;        // adjust scroll speed
        //         // redraw();
        //         return 1; // event handled
        //     }



		if (e == FL_KEYDOWN && Fl::event_key() == FL_Shift_L) {
			printf("Shift pressionado\n");

		}
		
        if(e == FL_KEYDOWN && Fl::event_state(FL_CTRL) && Fl::event_key()=='b'){
            helperinit();
            return 0;
        }

        if(e == FL_KEYDOWN && Fl::event_state(FL_CTRL) && Fl::event_key()=='s'){
            save();
			// cotm("f1");
            return 0;
        }

        if(e == FL_KEYDOWN && Fl::event_state(FL_CTRL) && Fl::event_key()=='f'){
			// cotm("f") 
			searchshow();
            return 1;
        }
		
        if(e == FL_KEYDOWN && Fl::event_state(FL_CTRL) && Fl::event_key()=='q'){ 
			toggle_comment();
            return 1;
        }

		if(e==FL_KEYDOWN && Fl::event_key()==FL_Escape){
			cotm("esc") 
			if(wFind)wFind->hide();
			SendEditor(SCI_AUTOCCANCEL);
			return 1;
		}

		if(e==FL_KEYDOWN){
			int key = Fl::event_key();
			if (key == FL_BackSpace || key == FL_Delete || key == FL_Tab){
				// cotm("backspace") 
				Fl_Scintilla::handle(e);
				navigatorSetUpdated();
				return 1;
			}
		}

        

        if (e == FL_PASTE){
			// cotm("p")
            const char *filename = Fl::event_text();
            if(!isFilenameValid(filename)){
				// cotm("nofile")
				// getchar();
                Fl_Scintilla::handle(FL_PASTE);
				navigatorSetUpdated();
				return 1;
			}
            if (filename) {
                fprintf(stderr, "Dropped: %s\n", filename);
                std::string path = filename;
                setscint(this,path);
                return 1;

                // OPTIONAL: Remove trailing newline or quotes
                if (!path.empty() && path.back() == '\n')
                    path.pop_back();

                std::ifstream file(path);
                if (file.is_open()) {
                    std::stringstream buffer;
                    buffer << file.rdbuf();
                    std::string contents = buffer.str();

                    this->SendEditor(SCI_SETTEXT, 0, (sptr_t)contents.c_str());
                    // set_lua();
                    return 1;
                }
            }
            return Fl_Scintilla::handle(FL_PASTE);  
        }
 
		return Fl_Scintilla::handle(e);
	}

	std::string fl_scintilla::getalltext() {
		// Get the total length (without null terminator)
		const auto length = static_cast<size_t>(SendEditor(SCI_GETTEXTLENGTH, 0, 0));

		// Allocate a string with space for the text + null terminator
		std::string allText(length, '\0');

		// Fill it (length+1 to include null terminator)
		SendEditor(SCI_GETTEXT, length + 1, reinterpret_cast<sptr_t>(allText.data()));

		// Now allText contains the editor’s text
		return allText;
	}
void fl_scintilla::save(){ 
	// ScintillaObject *sci=scicurr;
	// if(ptfcurr==0)return;
	if(!SendEditor(SCI_GETMODIFY,0,0))return;
	if(!fs::exists(filename))return;
	// cotm(filename) 
	FILE *fp = fopen(filename.c_str(), "wb");
	if (fp) {
		const int blockSize = 1310720;
		char data[blockSize + 1];
		int lengthDoc = SendEditor(SCI_GETLENGTH,0,0);
		for (int i = 0; i < lengthDoc; i += blockSize) {
			int grabSize = lengthDoc - i;
			if (grabSize > blockSize)
				grabSize = blockSize;
			Sci_TextRange  tr;
			tr.chrg.cpMin = i;
			tr.chrg.cpMax = i + grabSize;
			tr.lpstrText = data;
			SendEditor(SCI_GETTEXTRANGE, 0, reinterpret_cast<sptr_t>(&tr)); //SCI_GETTEXTRANGE long
			// sciGetRange(i, i + grabSize, data);
			/*if (props.GetInt("strip.trailing.spaces"))
				grabSize = StripTrailingSpaces(
									data, grabSize, grabSize != blockSize);*/
			fwrite(data, grabSize, 1, fp);
		}
		fclose(fp);
		SendEditor(SCI_SETSAVEPOINT,0,0);
		// cotm(files[filename].notsaved)
		files[filename].notsaved=SendEditor(SCI_GETMODIFY  , 0,0);
		setnsaved();
		// cotm(files[filename].notsaved)
		// if(callbackOnSave)voidToFunc(callbackOnSave,string)(filenamecurrent); 
	}
}




void fl_scintilla::set_lua(){ 
// 	std::ifstream f("DejaVuSans.ttf", std::ios::binary);
// if (!f) throw std::runtime_error("Cannot open font file at path: " );
	// string loaded = load_app_font("DejaVuSans-Bold.ttf");
	// string floaded = load_app_font("Cascadia Mono PL SemiBold 600.otf");
	// string loaded = load_app_font("DejaVuSansMono.ttf");




	// string loaded = load_app_font("Cascadia Mono PL SemiBold 600.otf");
	// cotm(floaded);
	// const char* fntname="Consolas";
	// const char* fntname="Cascadia Mono PL SemiBold";

	const char* fntname=floaded.c_str();
// sleepms(500);
	#ifdef __linux__
	// const char* fntname="DejaVu Sans";
	// const char* fntname=loaded.c_str();
	// const char* fntname="Cascadia Mono PL SemiBold";
	#else
	// const char* fntname="DejaVu Sans";
	// const char* fntname=loaded.c_str();
	#endif
	
	// const char* fntname="";

	int fntsize=9;
	#ifdef _WIN32
	fntsize=8;
	#endif

	// const char* fntname="Cascadia Code";
    SendEditor(SCI_SETCODEPAGE, SC_CP_UTF8, 0); //allways when opening new file
    SendEditor(SCI_SETLEXER, SCLEX_LUA);
	// printf("STYLE_LASTPREDEFINED %d\n",STYLE_LASTPREDEFINED);
	for (int i = 0; i < STYLE_LASTPREDEFINED; ++i) {
		SendEditor(SCI_STYLESETFONT, i, (sptr_t)fntname);
		SendEditor(SCI_STYLESETFORE, STYLE_DEFAULT, RGB(0, 0, 0));
		SendEditor(SCI_STYLESETSIZE, i, fntsize);  
		// SendEditor(SCI_STYLESETBOLD, STYLE_DEFAULT, 1); // 1 = true
	} 
 
SendEditor(SCI_SETEXTRAASCENT, -2);
SendEditor(SCI_SETEXTRADESCENT, -2);


	string keys0="function end if then local print for do while break return until else in elseif true false goto require";
	SendEditor(SCI_SETKEYWORDS, 0, (sptr_t)keys0.c_str());
	SendEditor(SCI_STYLESETFORE, SCE_LUA_WORD, RGB(0, 0, 255));

	int version = SendEditor( 267, 0, 0);
	// Version is encoded as: MMmm (e.g., 0x030700 for version 3.7.0)
	int major = (version >> 8) & 0xFF;
	int minor = version & 0xFF;	
	// printf("Scintilla version: %d.%d\n", major, minor);

	SendEditor(SCI_SETMARGINTYPEN,2, SC_MARGIN_NUMBER);
    SendEditor(SCI_SETMARGINWIDTHN,2, 15);
	// char fontname[100] = {0};
	// SendEditor(SCI_STYLEGETFONT,STYLE_LINENUMBER,(sptr_t)fontname);
	// printf("fontname %s\n",fontname);
	SendEditor(SCI_STYLESETFONT, STYLE_LINENUMBER, (sptr_t)fntname);
    SendEditor(SCI_STYLESETSIZE, STYLE_LINENUMBER, fntsize-2);

    SendEditor(SCI_SETTABWIDTH, 4);

    SendEditor(SCI_SETPROPERTY,(sptr_t)"fold",(sptr_t)"1");
    SendEditor(SCI_SETMARGINTYPEN, MARGIN_FOLD_INDEX, SC_MARGIN_SYMBOL);
    SendEditor(SCI_SETMARGINMASKN, MARGIN_FOLD_INDEX, SC_MASK_FOLDERS);
    SendEditor(SCI_SETMARGINWIDTHN, MARGIN_FOLD_INDEX, 11);
    SendEditor(SCI_SETMARGINSENSITIVEN, MARGIN_FOLD_INDEX, true);    
	SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDER, SC_MARK_CIRCLEPLUS);
    SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPEN, SC_MARK_ARROW);
    SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEREND,  SC_MARK_CIRCLEPLUSCONNECTED);
    SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPENMID, SC_MARK_CIRCLEMINUSCONNECTED);
    SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_TCORNERCURVE);
    SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERSUB, SC_MARK_VLINE);
    SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERTAIL, SC_MARK_LCORNERCURVE);
	SendEditor(SCI_MARKERSETBACK, SC_MARKNUM_FOLDERSUB, 0xa0a0a0);
    SendEditor(SCI_MARKERSETBACK, SC_MARKNUM_FOLDERMIDTAIL, 0xa0a0a0);
    SendEditor(SCI_MARKERSETBACK, SC_MARKNUM_FOLDERTAIL, 0xa0a0a0);
    SendEditor(SCI_SETFOLDFLAGS, 16|4, 0);
	
	SendEditor( SCI_SETPROPERTY, reinterpret_cast<sptr_t>("fold.compact"), reinterpret_cast<sptr_t>("0"));
	SendEditor(SCI_SETPROPERTY, reinterpret_cast<sptr_t>("fold.by.indentation"), reinterpret_cast<sptr_t>("1"));

	comment="--";

    SendEditor(SCI_STYLESETFONT, SCE_LUA_COMMENTLINE, (sptr_t)fntname); 
    SendEditor(SCI_STYLESETSIZE, SCE_LUA_COMMENTLINE, fntsize-2);
    SendEditor(SCI_STYLESETSIZE, SCE_LUA_COMMENT, fntsize-2);
    SendEditor(SCI_STYLESETSIZE, SCE_LUA_COMMENTDOC, fntsize-2);
	SendEditor(SCI_STYLESETFORE, SCE_LUA_COMMENT, 0x00008000);
    SendEditor(SCI_STYLESETFORE, SCE_LUA_COMMENTLINE, 0x00008000);
    SendEditor(SCI_STYLESETFORE, SCE_LUA_COMMENTDOC, 0x00008000);

    SendEditor(SCI_SETCARETLINEVISIBLE, 1);
    SendEditor(SCI_SETCARETLINEBACK, 0xb0ffff);
    SendEditor(SCI_SETMARGINTYPEN,0,SC_MARGIN_SYMBOL);

	// word wrap
	SendEditor(SCI_SETWRAPMODE,SC_WRAP_WORD,0);    
    // Enable word wrap
    SendEditor(  SCI_SETWRAPVISUALFLAGS, SC_WRAPVISUALFLAG_END, 0);    
    // Optionally set wrap indent mode
    SendEditor(  SCI_SETWRAPINDENTMODE, SC_WRAPINDENT_INDENT, 0);
}
void fl_scintilla::getfuncs(){
	if(!bfunctions)return;
    const int lineCount = SendEditor(SCI_GETLINECOUNT);
    std::stack<int> blockStack;
	vint vlinel;
    int currentLevel = SC_FOLDLEVELBASE;
    std::string tres; 
    static std::string pres;
    for (int line = 0; line < lineCount; ++line)
    {
        // Obter texto da linha
        int length = SendEditor(SCI_LINELENGTH, line);
        if (length <= 0) {
            // SendEditor(SCI_SETFOLDLEVEL, line, currentLevel);
            continue;
        } 
        char* buffer = new char[length + 1];
        Sci_TextRange tr;
        tr.chrg.cpMin = SendEditor(SCI_POSITIONFROMLINE, line);
        tr.chrg.cpMax = tr.chrg.cpMin + length;
        tr.lpstrText = buffer;
        SendEditor(SCI_GETTEXTRANGE, 0, reinterpret_cast<sptr_t>(&tr));

        std::string lineText(buffer);
        delete[] buffer;

        // Remover espaços
        lineText.erase(0, lineText.find_first_not_of(" \t"));
        lineText.erase(lineText.find_last_not_of(" \t\r\n") + 1);

        // Converter para minúsculas para facilitar comparação
        std::string lowerLine = lineText;
        for (char& c : lowerLine) c = std::tolower(c);
 
        if (lowerLine.find("function") == 0 ) {
            if(lowerLine.size()<=9)continue;
            std::string res=std::string(lowerLine.begin()+9,lowerLine.end());
            tres+=res+"\n"; 
			vlinel.push_back(line);
        }
    }
	// cotm(0)
    if(tres!=pres){
        pres=tres;
		vline=vlinel;
		bfunctions->clear(); 
		vstring v=split(tres,"\n");
		lop(i,0,v.size()-1){ 
			bfunctions->add(v[i].c_str());
		}
        // printf("%s\n",pres.c_str());
    }
}
static const char *minus_xpm[] = {
	/* width height num_colors chars_per_pixel */
	"     9     9       16            1",
	/* colors */
	"` c #8c96ac",
	". c #c4d0da",
	"# c #daecf4",
	"a c #ccdeec",
	"b c #eceef4",
	"c c #e0e5eb",
	"d c #a7b7c7",
	"e c #e4ecf0",
	"f c #d0d8e2",
	"g c #b7c5d4",
	"h c #fafdfc",
	"i c #b4becc",
	"j c #d1e6f1",
	"k c #e4f2fb",
	"l c #ecf6fc",
	"m c #d4dfe7",
	/* pixels */
	"hbc.i.cbh",
	"bffeheffb",
	"mfllkllfm",
	"gjkkkkkji",
	"da`````jd",
	"ga#j##jai",
	"f.k##k#.a",
	"#..jkj..#",
	"hemgdgc#h"
};
/* XPM */
static const char *plus_xpm[] = {
	/* width height num_colors chars_per_pixel */
	"     9     9       16            1",
	/* colors */
	"` c #242e44",
	". c #8ea0b5",
	"# c #b7d5e4",
	"a c #dcf2fc",
	"b c #a2b8c8",
	"c c #ccd2dc",
	"d c #b8c6d4",
	"e c #f4f4f4",
	"f c #accadc",
	"g c #798fa4",
	"h c #a4b0c0",
	"i c #cde5f0",
	"j c #bcdeec",
	"k c #ecf1f6",
	"l c #acbccc",
	"m c #fcfefc",
	/* pixels */
	"mech.hcem",
	"eldikille",
	"dlaa`akld",
	".#ii`ii#.",
	"g#`````fg",
	".fjj`jjf.",
	"lbji`ijbd",
	"khb#idlhk",
	"mkd.ghdkm"
};

//
const size_t FUNCSIZE=2;
char* g_szFuncList[FUNCSIZE]= { //函数名
	"file-",
	"MoveWindow("
};
char* g_szFuncDesc[FUNCSIZE]= { //函数信息
	"HWND CreateWindow-"
	"LPCTSTR lpClassName,"
	" LPCTSTR lpWindowName,"
	" DWORD dwStyle, "
	" PVOID lpParam"
	"="
	,
	"BOOL MoveWindow-"
	"HWND hWnd,"
	" int X,"
	" int Y,"
	" int nWidth,"
	" int nHeight,"
	" BOOL bRepaint"
	"="
};
void fl_scintilla::move_item(Fl_Browser* browser, string str) {
	int from=0;
	int to=1;

	lop(i,1,browser->size()+1){
		stringstream strm;
		strm<<folder<<browser->text(i);
		// cotm(str,strm.str())
		if(str==strm.str() || str+"*"==strm.str()){
			from=i;
			break;
		}
	}
	// cotm(from)

    if (from == to || from < 1 || to < 1 || from > browser->size() || to > browser->size() + 1)
        return;

    const char* text = browser->text(from);
    if (!text) return;

    std::string moved_text = text;
    browser->remove(from);

    // Adjust target index if we're moving down (because list shrinks)
    if (to > from) --to;

    browser->insert(to, moved_text.c_str());
}
void fl_scintilla::navigatorSetUpdated(){
		cotm("v1")
	    filesfirstline[curr_file_pointer]=SendEditor(SCI_GETFIRSTVISIBLELINE,0,0);
        filesfirstline[curr_file_pointer]=SendEditor(SCI_DOCLINEFROMVISIBLE,filesfirstline[curr_file_pointer],0);
        // filescaret[curr_file_pointer]=SendEditor(SCI_GETCURRENTPOS);
        files[filename].notsaved=SendEditor(SCI_GETMODIFY  , 0,0);
		cotm("v2")
		setnsaved();
		cotm("v3")
		if(bfilesmodified){
			cotm("v4")
			move_item(bfilesmodified,filename);
			cotm("v5")
		}
}
static void cb_editor(Scintilla::SCNotification *scn, void *data)
{
	if(loading)	return;

    fl_scintilla* editor=(fl_scintilla*)data;
	Scintilla::SCNotification *notify = scn;

	// if (scn->nmhdr.code!=2013)printf("scn %d\n",scn->nmhdr.code);
	// if (scn->nmhdr.code==SCN_UPDATEUI)printf("SCN_UPDATEUI %d\n",scn->nmhdr.code);

 
	if (scn->nmhdr.code == SCN_CHARADDED && editor->curr_file_pointer>0)
		editor->getfuncs();

    if ((scn->nmhdr.code == SCN_CHARADDED) && editor->curr_file_pointer>0){
        // cotm("chad\n")
		editor->navigatorSetUpdated();

    }

	//auto ident
	if (scn->nmhdr.code == SCN_CHARADDED && scn->ch == '\n') {
		int curPos = editor->SendEditor(  SCI_GETCURRENTPOS, 0, 0);
		int currentLine = editor->SendEditor(  SCI_LINEFROMPOSITION, curPos, 0);
	
		if (currentLine > 0) {
			char buffer[256] = {0};
			int prevLen = editor->SendEditor(  SCI_LINELENGTH, currentLine - 1, 0);
	
			if (prevLen > 0 && prevLen < 256) {
				editor->SendEditor(  SCI_GETLINE, currentLine - 1, (sptr_t)buffer);
	
				std::string indent;
				for (int i = 0; i < prevLen; ++i) {
					if (buffer[i] == ' ' || buffer[i] == '\t')
						indent += buffer[i];
					else
						break;
				}
                vstring lookfor={"function","if","for","lop","else","elseif"};
                // if(contains())
	
				editor->SendEditor(  SCI_REPLACESEL, 0, (sptr_t)indent.c_str());
			}
		}
	}
	

	if(notify->nmhdr.code == SCN_MARGINCLICK ) {
		if ( notify->margin == 3) {
			// 确定是页边点击事件
			const int line_number = editor->SendEditor(SCI_LINEFROMPOSITION,notify->position);
			editor->SendEditor(SCI_TOGGLEFOLD, line_number);
		}
	}

	if(notify->nmhdr.code == SCN_CHARADDED) {
		// editor->SendEditor(SCI_COLOURISE, 0, -1);
		
		// 函数提示功能
		static const char* pCallTipNextWord = NULL;//下一个高亮位置
		static const char* pCallTipCurDesc = NULL;//当前提示的函数信息
		if(notify->ch == '-') { //如果输入了左括号，显示函数提示
			char word[1000]; //保存当前光标下的单词(函数名)
			Scintilla::TextRange tr;    //用于SCI_GETTEXTRANGE命令
			int pos = editor->SendEditor(SCI_GETCURRENTPOS); //取得当前位置（括号的位置）
			int startpos = editor->SendEditor(SCI_WORDSTARTPOSITION,pos-1);//当前单词起始位置
			int endpos = editor->SendEditor(SCI_WORDENDPOSITION,pos-1);//当前单词终止位置
			tr.chrg.cpMin = startpos;  //设定单词区间，取出单词
			tr.chrg.cpMax = endpos;
			tr.lpstrText = word;
			editor->SendEditor(SCI_GETTEXTRANGE,0, sptr_t(&tr));

			for(size_t i=0; i<FUNCSIZE; i++) { //找找有没有我们认识的函数？
				if(memcmp(g_szFuncList[i],word,strlen(g_szFuncList[i])) == 0) {
					printf("show all\n");
					//找到啦，那么显示提示吧
					pCallTipCurDesc = g_szFuncDesc[i]; //当前提示的函数信息
					editor->SendEditor(SCI_CALLTIPSHOW,pos,sptr_t(pCallTipCurDesc));//显示这个提示
					const char *pStart = strchr(pCallTipCurDesc,'-')+1; //高亮第一个参数
					const char *pEnd = strchr(pStart,',');//参数列表以逗号分隔
					if(pEnd == NULL) pEnd = strchr(pStart,'='); 
					editor->SendEditor(SCI_CALLTIPSETHLT, pStart-pCallTipCurDesc, pEnd-pCallTipCurDesc);
					pCallTipNextWord = pEnd+1; 
					break;
				}
			}
		} else if(notify->ch == '=') {  
			printf("close\n");
			editor->SendEditor(SCI_CALLTIPCANCEL);
			pCallTipCurDesc = NULL;
			pCallTipNextWord = NULL;
		} else if(notify->ch == ',' && editor->SendEditor(SCI_CALLTIPACTIVE) && pCallTipCurDesc) {
			printf("show param\n"); 
			const char *pStart = pCallTipNextWord;
			const char *pEnd = strchr(pStart,',');
			if(pEnd == NULL) pEnd = strchr(pStart,'=');
			if(pEnd == NULL)  
				editor->SendEditor(SCI_CALLTIPCANCEL);
			else {
				printf("show param, %d %d\n", pStart-pCallTipCurDesc, pEnd-pCallTipCurDesc);
				editor->SendEditor(SCI_CALLTIPSETHLT,pStart-pCallTipCurDesc, pEnd-pCallTipCurDesc);
				pCallTipNextWord = pEnd+1;
			}
		}

		if(notify->ch == '.') {
			char word[1000]; //保存当前光标下的单词
			Scintilla::TextRange tr;    //用于SCI_GETTEXTRANGE命令
			int pos = editor->SendEditor(SCI_GETCURRENTPOS); //取得当前位置
			int startpos = editor->SendEditor(SCI_WORDSTARTPOSITION,pos-1);//当前单词起始位置
			int endpos = editor->SendEditor(SCI_WORDENDPOSITION,pos-1);//当前单词终止位置
			tr.chrg.cpMin = startpos;  //设定单词区间，取出单词
			tr.chrg.cpMax = endpos;
			tr.lpstrText = word;
			editor->SendEditor(SCI_GETTEXTRANGE,0, sptr_t(&tr));
			if(strcmp(word,"file.") == 0) { //输入file.后提示file对象的几个方法
				editor->SendEditor(SCI_REGISTERIMAGE, 2, sptr_t(minus_xpm));
				editor->SendEditor(SCI_REGISTERIMAGE, 5, sptr_t(plus_xpm));
				editor->SendEditor(SCI_AUTOCSHOW,0,
				                   sptr_t(
				                           "close?2 "
				                           "eof?4 "
				                           "goodjhfg "
				                           "open?5 "
				                           "rdbuf1中文 "
				                           "size "
										   "t1 "
										   "t2 "
										   "t3 "
										   "t4 "
										   "t5?5"
				                   ));
			}
		}
	}

	
}

void setscint(fl_scintilla* editor,string filename){ 
	loading=1;
    // cotm("setscint");	
    // return;
	if(files[filename].p==NULL){
		FILE *fp = fopen(filename.c_str(), "rb");
		if (fp) { 
			sptr_t p=editor->SendEditor(SCI_GETDOCPOINTER,0,0);
			editor->SendEditor(SCI_ADDREFDOCUMENT,0, p);
			p=editor->SendEditor(SCI_CREATEDOCUMENT,0,0);
			editor->SendEditor(SCI_SETDOCPOINTER,0, p); 
			// Disable UNDO
			editor->SendEditor(SCI_SETUNDOCOLLECTION, 0,0);

			std::string path = filename;
			if (!path.empty() && path.back() == '\n')
				path.pop_back();

			std::ifstream file(path);
			if (file.is_open()) {
				std::stringstream buffer;
				buffer << file.rdbuf();
				std::string contents = buffer.str();

				editor->SendEditor(SCI_SETTEXT, 0, (sptr_t)contents.c_str()); 
				file.close();
			}
			// pausa

			// char data[128*1024];
			// int lenFile = fread(data, 1, sizeof(data), fp);
			// while (lenFile > 0) {
			// 	editor->SendEditor(SCI_ADDTEXT, lenFile,
			// 	reinterpret_cast<sptr_t>(static_cast<char *>(data)));
			// 	lenFile = fread(data, 1, sizeof(data), fp);
			// }
			// fclose(fp);

			editor->SendEditor(SCI_SETSEL, 0, 0);
			// Re-Enable UNDO
			editor->SendEditor(SCI_SETUNDOCOLLECTION, 1,0);
			editor->SendEditor(SCI_SETSAVEPOINT,0,0);
			files[filename].p=editor->SendEditor(SCI_GETDOCPOINTER,0,0);
		}
	}else{ 
		editor->SendEditor(SCI_SETDOCPOINTER,0, files[filename].p); 
        // printf("fl %s\n",editor->filename.c_str());
		editor->SendEditor(SCI_SETFIRSTVISIBLELINE, editor->filesfirstline[files[filename].p], 0); 
		// editor->SendEditor(SCI_SETANCHOR, editor->filescaret[files[filename].p], 0); 
		// editor->SendEditor(SCI_SETCURRENTPOS, editor->filescaret[files[filename].p], 0); 
		// Fl::focus(editor);
	}
    editor->curr_file_pointer=files[filename].p;
    editor->filename=filename;
    editor->set_lua();
		// sci_setup(SCINTILLA(editor[ieditor]));
		// sci_setStyleCommon(SCINTILLA(editor[ieditor]));
		// sci_setStyleC(SCINTILLA(editor[ieditor]));
	thread([]{
		sleepms(300);//500
		loading=0;
	}).detach();
} 





// Optional: Sort function by filename
bool sortByFilename(const FileEntry& a, const FileEntry& b) {
    return a.filename < b.filename;
}

std::vector<FileEntry> fl_scintilla::list_files_in_dir(const std::string path) {
    std::vector<FileEntry> entries;

    for (const auto& entry : fs::directory_iterator(path)) {
        if (entry.is_regular_file()) {
            auto ftime = entry.last_write_time();
            auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
            );
            std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);

            entries.push_back(FileEntry{
                entry.path().filename().string(),
                cftime
            });
        }
    }

    return entries;
}

void updatenavigator(fl_scintilla* editor){
	if(!editor->bfilesmodified)return;
	editor->lfiles=editor->list_files_in_dir(editor->folder);
	vector<FileEntry> &files=editor->lfiles;
	std::sort(files.begin(), files.end(), sortByFilename);
    for (const auto& f : files) {
		editor->bfiles->add(fs::path(f.filename).filename().string().c_str());
        // std::cout << f.filename << " - " << std::asctime(std::localtime(&f.modified));
    }

	
    std::sort(files.begin(), files.end());
    for (const auto& f : files) {
		editor->bfilesmodified->add(fs::path(f.filename).filename().string().c_str());


        // std::cout << std::asctime(std::localtime(&f.modified)) << " " << f.filename << '\n';
    }
}

void fl_scintilla::setnsaved(){
	if(!bfilesmodified)return;

	lop(i,1,bfiles->size()+1){
		string str=bfiles->text(i);
		if (!str.empty() && str.back() == '*') {
			str.pop_back();
		}
		// cotm(str);
		if(files[folder+str].notsaved){
			str+="*";
		}
		bfiles->text(i,str.c_str());
	}
	lop(i,1,bfilesmodified->size()+1){
		string str=bfilesmodified->text(i);
		if (!str.empty() && str.back() == '*') {
			str.pop_back();
		}
		// cotm(str);
		// cotm(files[folder+str].notsaved)
		if(files[folder+str].notsaved){
			str+="*";
		}
		bfilesmodified->text(i,str.c_str());
	}
}

// extern Fl_Menu_Bar* menu;


void fl_scintilla::helperinit(){   





	if(!show_browser)return;
	show_browser=0;
	// Fl_Browser
    window()->begin(); 
	int sz=22*3;
    size(w(),h()-sz); 
    navigator=new Fl_Window(x(), h()+22*1 ,w(),sz);
    // navigator=new Fl_Group(x(), h()+22*1 ,w(),sz);
    
    navigator->box(FL_FLAT_BOX);
    navigator->color(FL_BLUE);

	bfiles=new Fl_Browser(0,0,w()*0.33,navigator->h());
	bfiles->type(FL_HOLD_BROWSER);

	bfiles->callback([](Fl_Widget *widget, void* v){
		fl_scintilla* vs=(fl_scintilla*)v;
		Fl_Browser* b=vs->bfiles;
		// cotm(b->value())
		stringstream strm;
		strm<<vs->folder<<b->text(b->value());
		string str=strm.str();
		if (!str.empty() && str.back() == '*') {
			str.pop_back();
		}
        setscint(vs,str);
		vs->bfilesmodified->deselect();
		vs->getfuncs();
    },this); 

	// bfiles->add("luafunc0.lua");
	// bfiles->add("json.lua");


	bfilesmodified=new Fl_Browser(w()*0.33,0,w()*0.33,navigator->h());
	bfilesmodified->type(FL_HOLD_BROWSER);
	bfilesmodified->callback([](Fl_Widget *widget, void* v){
		fl_scintilla* vs=(fl_scintilla*)v;
		Fl_Browser* b=vs->bfilesmodified;
		// cotm(b->value())
		stringstream strm;
		strm<<vs->folder<<b->text(b->value());
		string str=strm.str();
		if (!str.empty() && str.back() == '*') {
			str.pop_back();
		}
        setscint(vs,str);
		vs->bfiles->deselect();
		vs->getfuncs();
    },this); 

	updatenavigator(this);


	bfunctions=new Fl_Browser(w()*0.66,0,w()*0.34,navigator->h());
	bfunctions->type(FL_HOLD_BROWSER);
	bfunctions->callback([](Fl_Widget *widget, void* v){
		fl_scintilla* editor=(fl_scintilla*)v;
		Fl_Browser* b=editor->bfunctions;
		int sel=b->value()-1;
		if(sel<editor->vline.size()){
			// cotm(vs->vline[sel])
			int line = editor->vline[sel];  // line you want at the top (0-based)
			
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
		// vs->getfuncs();
		// cotm(b->value()) 
    },this); 

	navigator->show();
    // window()->end();
    // child_to_local(navigator);
	Fl_Group::current(nullptr);
	window()->redraw();

}





