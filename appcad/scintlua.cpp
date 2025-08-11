#ifdef _WIN32 
#include <lua.hpp>
	#else
#include <lua5.4/lua.hpp> 
#endif
#include "fl_scintilla.hpp"

using namespace std;

void lua_str(string str,bool isfile);

struct scint : public fl_scintilla { 
	scint(int X, int Y, int W, int H, const char* l = 0)
	: fl_scintilla(X, Y, W, H, l) { }
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
	if(e == FL_KEYDOWN && Fl::event_state(FL_CTRL) && Fl::event_key()==FL_F + 2){
		cotm("f2",filename);
		lua_str(filename,1);
		return 1; 
	}
	if(e==FL_UNFOCUS)SendEditor(SCI_AUTOCCANCEL);


	return fl_scintilla::handle(e);
}


scint* editor;

void scint_init(int x,int y,int w,int h){ 
	editor = new scint(x,y,w,h);
}