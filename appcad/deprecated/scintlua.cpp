// #ifdef _WIN32 
// #include <lua.hpp>
// 	#else
// #include <lua5.4/lua.hpp> 
// #endif
// #include "fl_scintilla.hpp" 

#include "includes.hpp"
using namespace std;

void lua_str(const string &str,bool isfile);
void lua_str_realtime(const string &str);


string currfilename="";

struct scint : public fl_scintilla { 
	scint(int X, int Y, int W, int H, const char* l = 0)
	: fl_scintilla(X, Y, W, H, l) {
		//  show_browser=0; set_flag(FL_ALIGN_INSIDE); 
		cotm(filename)
		currfilename=filename;
		// lua_str(filename,1);
		}
	int handle(int e)override;
};

int scint::handle(int e){



	if(e == FL_KEYDOWN && Fl::event_state(FL_CTRL) && Fl::event_key()==FL_F + 1){
		int currentPos = SendEditor( SCI_GETCURRENTPOS, 0, 0);
		int currentLine = SendEditor(SCI_LINEFROMPOSITION, currentPos, 0);
		int lineLength = SendEditor(SCI_LINELENGTH, currentLine, 0);
		if (lineLength <= 0) return 0; 
		std::string buffer(lineLength, '\0'); // allocate with room for '\0'
		SendEditor( SCI_GETLINE, currentLine, (sptr_t)buffer.data()); 
		buffer.erase(buffer.find_last_not_of("\r\n\0") + 1);
		// lua_str(buffer,0);
		return 1;
	}
	// if(e == FL_KEYDOWN && Fl::event_state(FL_CTRL) && Fl::event_key()==FL_F + 2){
	// 	// cotm("f2",filename);
	// 	lua_str(filename,1);
	// 	return 1; 
	// }
	if(e == FL_KEYDOWN && Fl::event_state(FL_CTRL) && Fl::event_key()=='s'){
		fl_scintilla::handle(e);
		// cotm("f2",filename)
		lua_str(filename,1);
		return 1;
	}


	if(e==FL_UNFOCUS)SendEditor(SCI_AUTOCCANCEL);


	int ret= fl_scintilla::handle(e);
	if(e == FL_KEYDOWN){
		lua_str_realtime(getalltext());
	}
	return ret;
}


scint* editor;

void scint_init(int x,int y,int w,int h){ 
	editor = new scint(x,y,w,h);
}
#include <string> 

void gopart(const std::string& str) {
    if (str.empty()) return;

    const int docLen = editor->SendEditor(SCI_GETTEXTLENGTH);
    if (docLen <= 0) return;

    // Enable regex search
    editor->SendEditor(SCI_SETSEARCHFLAGS, SCFIND_REGEXP);

    // Search the entire document
    editor->SendEditor(SCI_SETTARGETSTART, 0);
    editor->SendEditor(SCI_SETTARGETEND, docLen);

    // Build regex: allow spaces/tabs, then Part, then optional space/paren, then your text
    std::string pattern = "^[ \\t]*Part[ \\t]*" + str;

    // Perform regex search
    const int pos = editor->SendEditor(
        SCI_SEARCHINTARGET,
        pattern.size(),
        (sptr_t)pattern.c_str()
    );

    if (pos == -1) return;  // no match

    // Get the matched line
    const int line = editor->SendEditor(SCI_LINEFROMPOSITION, pos);

    // Ensure the line is visible (important if folded)
    editor->SendEditor(SCI_ENSUREVISIBLE, line);

    // Scroll so the matched line is at the top
    const int visual_line        = editor->SendEditor(SCI_VISIBLEFROMDOCLINE, line);
    const int current_visual_top = editor->SendEditor(SCI_GETFIRSTVISIBLELINE);
    const int delta              = visual_line - current_visual_top;

    if (delta != 0)
        editor->SendEditor(SCI_LINESCROLL, 0, delta-2);

// editor->SendEditor(SCI_SETSEL, pos, pos); //not working
// Move caret (this is usually the most reliable way)
    editor->SendEditor(SCI_GOTOPOS, pos);

    // Optional: force redraw / update UI if still not showing
    // editor->SendEditor(SCI_SCROLLCARET);
	// editor->SendEditor(SCI_SETEMPTYSELECTION, pos); 
	editor->take_focus();
	Fl::focus(editor);
}





