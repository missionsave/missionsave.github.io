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

Fl_Scintilla *editor;


#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <stack>
#include <cctype>

using namespace Scintilla;
using namespace std;

void UpdateLuaFolding(){
	// return;
    const int lineCount = editor->SendEditor(SCI_GETLINECOUNT);
    std::stack<int> blockStack;

    int currentLevel = SC_FOLDLEVELBASE;
    std::string tres;
    static std::string pres;
    for (int line = 0; line < lineCount; ++line)
    {
        // Obter texto da linha
        int length = editor->SendEditor(SCI_LINELENGTH, line);
        if (length <= 0) {
            editor->SendEditor(SCI_SETFOLDLEVEL, line, currentLevel);
            continue;
        }

        char* buffer = new char[length + 1];
        Sci_TextRange tr;
        tr.chrg.cpMin = editor->SendEditor(SCI_POSITIONFROMLINE, line);
        tr.chrg.cpMax = tr.chrg.cpMin + length;
        tr.lpstrText = buffer;
        editor->SendEditor(SCI_GETTEXTRANGE, 0, reinterpret_cast<sptr_t>(&tr));

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
        }
    }
    if(tres!=pres){
        pres=tres;
        printf("%s\n",pres.c_str());
    }
}

#define MARGIN_FOLD_INDEX 3

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
static void cb_editor(Scintilla::SCNotification *scn, void *data)
{
	Scintilla::SCNotification *notify = scn;

	// if (scn->nmhdr.code!=2013)printf("scn %d\n",scn->nmhdr.code);

	if (scn->nmhdr.code==2007)UpdateLuaFolding();


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

void sci_init() {
	editor->SendEditor(SCI_SETMARGINTYPEN,2, SC_MARGIN_NUMBER);
    // editor->SendEditor(SCI_SETMARGINWIDTHN,2, 20);
    // return;

    char *sss = "file\n";
    editor->SendEditor(SCI_APPENDTEXT, strlen(sss), (sptr_t)sss);
    const char* szKeywords1=
            "asm auto break case catch class const "
            "const_cast continue default delete do double "
            "dynamic_cast else enum explicit extern false "
            "for friend goto if inline mutable "
            "namespace new operator private protected public "
            "register reinterpret_cast return signed "
            "sizeof static static_cast struct switch template "
            "this throw true try typedef typeid typename "
            "union unsigned using virtual volatile while";
    const char* szKeywords2=
            "bool char float int long short void wchar_t";

    // 设置全局风格
    //editor->SendEditor(SCI_STYLESETFONT, STYLE_DEFAULT,(sptr_t)"Fixedsys");//"Courier New");
    editor->SendEditor(SCI_STYLESETFONT, STYLE_DEFAULT,(sptr_t)"Courier New");
    editor->SendEditor(SCI_STYLESETSIZE, STYLE_DEFAULT,10);
    editor->SendEditor(SCI_STYLECLEARALL);

    editor->SendEditor(SCI_CALLTIPUSESTYLE, 0);
    editor->SendEditor(SCI_STYLESETFONT, STYLE_CALLTIP,(sptr_t)"Courier New");
    editor->SendEditor(SCI_STYLESETSIZE, STYLE_CALLTIP,9);

    //C++语法解析
    editor->SendEditor(SCI_SETLEXER, SCLEX_CPP,0);
    editor->SendEditor(SCI_SETKEYWORDS, 0, (sptr_t)szKeywords1);//设置关键字
    editor->SendEditor(SCI_SETKEYWORDS, 1, (sptr_t)szKeywords2);//设置关键字

    // 下面设置各种语法元素风格
    // editor->SendEditor(SCI_STYLESETFORE, SCE_C_WORD, 0x00FF0000);   //关键字
    // editor->SendEditor(SCI_STYLESETFORE, SCE_C_WORD2, 0x00800080);   //关键字
    // editor->SendEditor(SCI_STYLESETBOLD, SCE_C_WORD2, 1);   //关键字
    // editor->SendEditor(SCI_STYLESETFORE, SCE_C_STRING, 0x001515A3); //字符串
    // editor->SendEditor(SCI_STYLESETFORE, SCE_C_CHARACTER, 0x001515A3); //字符
    // editor->SendEditor(SCI_STYLESETFORE, SCE_C_PREPROCESSOR, 0x00808080);//预编译开关
    // editor->SendEditor(SCI_STYLESETFORE, SCE_C_COMMENT, 0x00008000);//块注释
    // editor->SendEditor(SCI_STYLESETFORE, SCE_C_COMMENTLINE, 0x00008000);//行注释
    // editor->SendEditor(SCI_STYLESETFORE, SCE_C_COMMENTDOC, 0x00008000);//文档注释（/**开头）

    editor->SendEditor(SCI_SETCARETLINEVISIBLE, 1);
    editor->SendEditor(SCI_SETCARETLINEBACK, 0xb0ffff);
    editor->SendEditor(SCI_SETMARGINTYPEN,0,SC_MARGIN_SYMBOL);
    // return;
    editor->SendEditor(SCI_SETMARGINWIDTHN,0, 9);
    editor->SendEditor(SCI_SETMARGINMASKN,0, 0x01);

    // 1号页边，宽度为9，显示1,2号标记(0..0110B)
    editor->SendEditor(SCI_SETMARGINTYPEN,1, SC_MARGIN_SYMBOL);
    editor->SendEditor(SCI_SETMARGINWIDTHN,1, 9);
    editor->SendEditor(SCI_SETMARGINMASKN,1, 0x06);

    // 2号页边，宽度为20，显示行号
    editor->SendEditor(SCI_SETMARGINTYPEN,2, SC_MARGIN_NUMBER);
    editor->SendEditor(SCI_SETMARGINWIDTHN,2, 20);

    // 设置标记的前景色
    editor->SendEditor(SCI_MARKERSETFORE,0,0x0000ff);//0-红色
    editor->SendEditor(SCI_MARKERSETFORE,1,0x00ff00);//1-绿色
    editor->SendEditor(SCI_MARKERSETFORE,2,0xff0000);//2-蓝色

    editor->SendEditor(SCI_SETPROPERTY,(sptr_t)"fold",(sptr_t)"1");
    editor->SendEditor(SCI_SETMARGINTYPEN, MARGIN_FOLD_INDEX, SC_MARGIN_SYMBOL);//页边类型
    editor->SendEditor(SCI_SETMARGINMASKN, MARGIN_FOLD_INDEX, SC_MASK_FOLDERS); //页边掩码
    editor->SendEditor(SCI_SETMARGINWIDTHN, MARGIN_FOLD_INDEX, 11); //页边宽度
    editor->SendEditor(SCI_SETMARGINSENSITIVEN, MARGIN_FOLD_INDEX, true); //响应鼠标消息

    // 折叠标签样式
    editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDER, SC_MARK_CIRCLEPLUS);
    editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPEN, SC_MARK_ARROW);
    editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEREND,  SC_MARK_CIRCLEPLUSCONNECTED);
    editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPENMID, SC_MARK_CIRCLEMINUSCONNECTED);
    editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_TCORNERCURVE);
    editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERSUB, SC_MARK_VLINE);
    editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERTAIL, SC_MARK_LCORNERCURVE);
    /*
    editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDER, SC_MARK_PIXMAP);
    editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPEN, SC_MARK_PIXMAP);
    editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEREND,  SC_MARK_PIXMAP);
    editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPENMID, SC_MARK_PIXMAP);
    editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_TCORNERCURVE);
    editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERSUB, SC_MARK_VLINE);
    editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERTAIL, SC_MARK_LCORNERCURVE);
    */

    /*
    editor->SendEditor(SCI_MARKERDEFINEPIXMAP, SC_MARKNUM_FOLDER, (sptr_t)plus_xpm);
    editor->SendEditor(SCI_MARKERDEFINEPIXMAP, SC_MARKNUM_FOLDEROPEN, (sptr_t)plus_xpm);
    editor->SendEditor(SCI_MARKERDEFINEPIXMAP, SC_MARKNUM_FOLDEREND, (sptr_t)plus_xpm);
    editor->SendEditor(SCI_MARKERDEFINEPIXMAP, SC_MARKNUM_FOLDEROPENMID, (sptr_t)minus_xpm);
    */

    // 折叠标签颜色
    editor->SendEditor(SCI_MARKERSETBACK, SC_MARKNUM_FOLDERSUB, 0xa0a0a0);
    editor->SendEditor(SCI_MARKERSETBACK, SC_MARKNUM_FOLDERMIDTAIL, 0xa0a0a0);
    editor->SendEditor(SCI_MARKERSETBACK, SC_MARKNUM_FOLDERTAIL, 0xa0a0a0);
    editor->SendEditor(SCI_SETFOLDFLAGS, 16|4, 0); //如果折叠就在折叠行的上下各画一条横线
    //*/

    editor->SendEditor(SCI_SETTABWIDTH, 4);

    //editor->SendEditor(SCI_SETCODEPAGE, SC_CP_UTF8);
    // editor->SendEditor(SCI_SETCODEPAGE, 936);
    //editor->SendEditor(SCI_STYLESETCHARACTERSET, SC_CHARSET_GB2312);

    // editor->SendEditor(SCI_SETHSCROLLBAR, false);










	int version = editor->SendEditor( 267, 0, 0);
	// Version is encoded as: MMmm (e.g., 0x030700 for version 3.7.0)
	int major = (version >> 8) & 0xFF;
	int minor = version & 0xFF;	
	printf("Scintilla version: %d.%d\n", major, minor);

	for (int i = 0; i < STYLE_LASTPREDEFINED; ++i) {
		editor->SendEditor(SCI_STYLESETFONT, i, (sptr_t)"Consolas");
		editor->SendEditor(SCI_STYLESETSIZE, i, 12); 
		editor->SendEditor(SCI_STYLESETBOLD, i, 1);
	}
    editor->SendEditor(SCI_STYLECLEARALL, 0, 0);

}

void sci_test() {

	// Setar o lexer para Lua
editor->SendEditor( SCI_SETLEXER, SCLEX_LUA);
editor->SendEditor( SCI_SETLEXERLANGUAGE, 0, reinterpret_cast<sptr_t>("lua"));
int lexer = editor->SendEditor(SCI_GETLEXER);
printf("Lexer ativo: %d\n", lexer);

// Ativar folding
editor->SendEditor( SCI_SETPROPERTY, reinterpret_cast<sptr_t>("fold"), reinterpret_cast<sptr_t>("1"));
editor->SendEditor( SCI_SETPROPERTY, reinterpret_cast<sptr_t>("fold.compact"), reinterpret_cast<sptr_t>("0"));
editor->SendEditor(SCI_SETPROPERTY, reinterpret_cast<sptr_t>("fold.by.indentation"), reinterpret_cast<sptr_t>("1"));


// Configurar margem para folding
// editor->SendEditor( SCI_SETMARGINTYPEN, 2, SC_MARGIN_SYMBOL);
// editor->SendEditor( SCI_SETMARGINWIDTHN, 2, 16);
// editor->SendEditor( SCI_SETMARGINMASKN, 2, SC_MASK_FOLDERS);
// editor->SendEditor( SCI_SETMARGINSENSITIVEN, 2, 1);

// Marcadores visuais
// editor->SendEditor( SCI_MARKERDEFINE, SC_MARKNUM_FOLDER, SC_MARK_BOXPLUS);
// editor->SendEditor( SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPEN, SC_MARK_BOXMINUS);
// editor->SendEditor( SCI_MARKERDEFINE, SC_MARKNUM_FOLDERSUB, SC_MARK_VLINE);
// editor->SendEditor( SCI_MARKERDEFINE, SC_MARKNUM_FOLDERTAIL, SC_MARK_LCORNER);
// editor->SendEditor( SCI_MARKERDEFINE, SC_MARKNUM_FOLDEREND, SC_MARK_BOXPLUSCONNECTED);
// editor->SendEditor( SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPENMID, SC_MARK_BOXMINUSCONNECTED);
// editor->SendEditor( SCI_MARKERDEFINE, SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_TCORNER);

// Aplicar o texto depois
editor->SendEditor( SCI_SETTEXT, 0, reinterpret_cast<sptr_t>(
    "function test()\n"
    "  print(\"hi\")\n"
    "end\n"
    "\n"
    "function another()\n"
    "  print(\"world\")\n"
    "end\n"
));

// Forçar reanálise
editor->SendEditor( SCI_COLOURISE, 0, -1);

editor->SendEditor(SCI_SETFOLDLEVEL, 0, SC_FOLDLEVELBASE | SC_FOLDLEVELHEADERFLAG);

// Linha 1 (corpo da função)
editor->SendEditor(SCI_SETFOLDLEVEL, 1, SC_FOLDLEVELBASE + 1);

// Linha 2 (end)
editor->SendEditor(SCI_SETFOLDLEVEL, 2, SC_FOLDLEVELBASE);
// return;
	int version = editor->SendEditor( 267, 0, 0);
	// Version is encoded as: MMmm (e.g., 0x030700 for version 3.7.0)
	int major = (version >> 8) & 0xFF;
	int minor = version & 0xFF;	
	printf("Scintilla version: %d.%d\n", major, minor);

	for (int i = 0; i < STYLE_LASTPREDEFINED; ++i) {
		editor->SendEditor(SCI_STYLESETFONT, i, (sptr_t)"Consolas");
		editor->SendEditor(SCI_STYLESETSIZE, i, 12); 
		editor->SendEditor(SCI_STYLESETBOLD, i, 1);
	}
    editor->SendEditor(SCI_STYLECLEARALL, 0, 0);


	//Lua
	// editor->SendEditor( SCI_SETLEXER, SCLEX_LUA);
	// editor->SendEditor(SCI_SETLEXERLANGUAGE, 0, (sptr_t)"LUA");
	// editor->SendEditor(SCI_SETPROPERTY, (sptr_t)"fold.compact",(sptr_t)"0");
    // editor->SendEditor(SCI_SETPROPERTY,(sptr_t)"fold",(sptr_t)"1");


// Margem de folding
editor->SendEditor( SCI_SETMARGINTYPEN, 2, SC_MARGIN_SYMBOL);
editor->SendEditor( SCI_SETMARGINWIDTHN, 2, 16);
editor->SendEditor( SCI_SETMARGINMASKN, 2, SC_MASK_FOLDERS);
editor->SendEditor( SCI_SETMARGINSENSITIVEN, 2, 1);

 
// return;
    //editor->box(FL_FLAT_BOX);
    //*
    editor->SendEditor(SCI_SETMARGINTYPEN,2, SC_MARGIN_NUMBER);
    editor->SendEditor(SCI_SETMARGINWIDTHN,2, 20);
    // return;

    char *sss = "file\n";
    editor->SendEditor(SCI_APPENDTEXT, strlen(sss), (sptr_t)sss);
    const char* szKeywords1=
            "asm auto break case catch class const "
            "const_cast continue default delete do double "
            "dynamic_cast else enum explicit extern false "
            "for friend goto if inline mutable "
            "namespace new operator private protected public "
            "register reinterpret_cast return signed "
            "sizeof static static_cast struct switch template "
            "this throw true try typedef typeid typename "
            "union unsigned using virtual volatile while";
    const char* szKeywords2=
            "bool char float int long short void wchar_t";

    // 设置全局风格
    //editor->SendEditor(SCI_STYLESETFONT, STYLE_DEFAULT,(sptr_t)"Fixedsys");//"Courier New");
    // editor->SendEditor(SCI_STYLESETFONT, STYLE_DEFAULT,(sptr_t)"Courier New");
    // editor->SendEditor(SCI_STYLESETSIZE, STYLE_DEFAULT,10);
    editor->SendEditor(SCI_STYLECLEARALL);

    editor->SendEditor(SCI_CALLTIPUSESTYLE, 0);
    // editor->SendEditor(SCI_STYLESETFONT, STYLE_CALLTIP,(sptr_t)"Courier New");
    // editor->SendEditor(SCI_STYLESETSIZE, STYLE_CALLTIP,9);

    //C++语法解析
    // editor->SendEditor(SCI_SETLEXER, SCLEX_LUA);
    editor->SendEditor(SCI_SETKEYWORDS, 0, (sptr_t)szKeywords1);//设置关键字
    editor->SendEditor(SCI_SETKEYWORDS, 1, (sptr_t)szKeywords2);//设置关键字

    // 下面设置各种语法元素风格
    // editor->SendEditor(SCI_STYLESETFORE, SCE_C_WORD, 0x00FF0000);   //关键字
    // editor->SendEditor(SCI_STYLESETFORE, SCE_C_WORD2, 0x00800080);   //关键字
    // // editor->SendEditor(SCI_STYLESETBOLD, SCE_C_WORD2, 1);   //关键字
    // editor->SendEditor(SCI_STYLESETFORE, SCE_C_STRING, 0x001515A3); //字符串
    // editor->SendEditor(SCI_STYLESETFORE, SCE_C_CHARACTER, 0x001515A3); //字符
    // editor->SendEditor(SCI_STYLESETFORE, SCE_C_PREPROCESSOR, 0x00808080);//预编译开关
    editor->SendEditor(SCI_STYLESETFORE, SCE_C_COMMENT, 0x00008000);//块注释
    editor->SendEditor(SCI_STYLESETFORE, SCE_C_COMMENTLINE, 0x00008000);//行注释
    editor->SendEditor(SCI_STYLESETFORE, SCE_C_COMMENTDOC, 0x00008000);//文档注释（/**开头）

    editor->SendEditor(SCI_SETCARETLINEVISIBLE, 1);
    editor->SendEditor(SCI_SETCARETLINEBACK, 0xb0ffff);
    editor->SendEditor(SCI_SETMARGINTYPEN,0,SC_MARGIN_SYMBOL);
    // return;
    editor->SendEditor(SCI_SETMARGINWIDTHN,0, 9);
    editor->SendEditor(SCI_SETMARGINMASKN,0, 0x01);

    // 1号页边，宽度为9，显示1,2号标记(0..0110B)
    editor->SendEditor(SCI_SETMARGINTYPEN,1, SC_MARGIN_SYMBOL);
    editor->SendEditor(SCI_SETMARGINWIDTHN,1, 9);
    editor->SendEditor(SCI_SETMARGINMASKN,1, 0x06);

    // 2号页边，宽度为20，显示行号
    editor->SendEditor(SCI_SETMARGINTYPEN,2, SC_MARGIN_NUMBER);
    editor->SendEditor(SCI_SETMARGINWIDTHN,2, 20);

    // 设置标记的前景色
    editor->SendEditor(SCI_MARKERSETFORE,0,0x0000ff);//0-红色
    editor->SendEditor(SCI_MARKERSETFORE,1,0x00ff00);//1-绿色
    editor->SendEditor(SCI_MARKERSETFORE,2,0xff0000);//2-蓝色

    editor->SendEditor(SCI_SETPROPERTY,(sptr_t)"fold",(sptr_t)"1");
    editor->SendEditor(SCI_SETMARGINTYPEN, MARGIN_FOLD_INDEX, SC_MARGIN_SYMBOL);//页边类型
    editor->SendEditor(SCI_SETMARGINMASKN, MARGIN_FOLD_INDEX, SC_MASK_FOLDERS); //页边掩码
    editor->SendEditor(SCI_SETMARGINWIDTHN, MARGIN_FOLD_INDEX, 11); //页边宽度
    editor->SendEditor(SCI_SETMARGINSENSITIVEN, MARGIN_FOLD_INDEX, true); //响应鼠标消息

    // 折叠标签样式
    editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDER, SC_MARK_CIRCLEPLUS);
    editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPEN, SC_MARK_ARROW);
    editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEREND,  SC_MARK_CIRCLEPLUSCONNECTED);
    editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPENMID, SC_MARK_CIRCLEMINUSCONNECTED);
    editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_TCORNERCURVE);
    editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERSUB, SC_MARK_VLINE);
    editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERTAIL, SC_MARK_LCORNERCURVE);
    /*
    editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDER, SC_MARK_PIXMAP);
    editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPEN, SC_MARK_PIXMAP);
    editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEREND,  SC_MARK_PIXMAP);
    editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPENMID, SC_MARK_PIXMAP);
    editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_TCORNERCURVE);
    editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERSUB, SC_MARK_VLINE);
    editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERTAIL, SC_MARK_LCORNERCURVE);
    */

    /*
    editor->SendEditor(SCI_MARKERDEFINEPIXMAP, SC_MARKNUM_FOLDER, (sptr_t)plus_xpm);
    editor->SendEditor(SCI_MARKERDEFINEPIXMAP, SC_MARKNUM_FOLDEROPEN, (sptr_t)plus_xpm);
    editor->SendEditor(SCI_MARKERDEFINEPIXMAP, SC_MARKNUM_FOLDEREND, (sptr_t)plus_xpm);
    editor->SendEditor(SCI_MARKERDEFINEPIXMAP, SC_MARKNUM_FOLDEROPENMID, (sptr_t)minus_xpm);
    */

    // 折叠标签颜色
    editor->SendEditor(SCI_MARKERSETBACK, SC_MARKNUM_FOLDERSUB, 0xa0a0a0);
    editor->SendEditor(SCI_MARKERSETBACK, SC_MARKNUM_FOLDERMIDTAIL, 0xa0a0a0);
    editor->SendEditor(SCI_MARKERSETBACK, SC_MARKNUM_FOLDERTAIL, 0xa0a0a0);
    editor->SendEditor(SCI_SETFOLDFLAGS, 16|4, 0); //如果折叠就在折叠行的上下各画一条横线
    //*/

    editor->SendEditor(SCI_SETTABWIDTH, 2);

    //editor->SendEditor(SCI_SETCODEPAGE, SC_CP_UTF8);
    // editor->SendEditor(SCI_SETCODEPAGE, 936);
    //editor->SendEditor(SCI_STYLESETCHARACTERSET, SC_CHARSET_GB2312);

    // editor->SendEditor(SCI_SETHSCROLLBAR, false);


	// //Lua
	// editor->SendEditor( SCI_SETLEXER, SCLEX_LUA);
// editor->SendEditor( SCI_SETMARGINTYPEN, 2, SC_MARGIN_SYMBOL);
// editor->SendEditor( SCI_SETMARGINWIDTHN, 2, 16);
// editor->SendEditor( SCI_SETMARGINMASKN, 2, SC_MASK_FOLDERS);
// editor->SendEditor( SCI_SETMARGINSENSITIVEN, 2, true);
// editor->SendEditor(SCI_SETPROPERTY, reinterpret_cast<sptr_t>("fold"), 1);
// editor->SendEditor(SCI_SETPROPERTY, (sptr_t)"fold.compact",(sptr_t)"0");
// Folding markers
// editor->SendEditor( SCI_MARKERDEFINE, SC_MARKNUM_FOLDER, SC_MARK_BOXPLUS);
// editor->SendEditor( SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPEN, SC_MARK_BOXMINUS);
// editor->SendEditor( SCI_MARKERDEFINE, SC_MARKNUM_FOLDERSUB, SC_MARK_VLINE);
// editor->SendEditor( SCI_MARKERDEFINE, SC_MARKNUM_FOLDERTAIL, SC_MARK_LCORNER);
// editor->SendEditor( SCI_MARKERDEFINE, SC_MARKNUM_FOLDEREND, SC_MARK_BOXPLUSCONNECTED);
// editor->SendEditor( SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPENMID, SC_MARK_BOXMINUSCONNECTED);
// editor->SendEditor( SCI_MARKERDEFINE, SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_TCORNER);
}

struct fl_scintilla : public Fl_Scintilla {
	// Construtor verdadeiro (sem 'void')
	fl_scintilla(int X, int Y, int W, int H, const char* l = 0)
	: Fl_Scintilla(X, Y, W, H, l) { }

	int handle(int e) override {
		// Trate eventos especiais aqui, se quiser
		if (e == FL_KEYDOWN && Fl::event_key() == FL_Shift_L) {
			printf("Shift pressionado\n");
		}
		if(e==FL_KEYDOWN && Fl::event_key()==FL_Escape){
			// cotm("esc")
			SendEditor(SCI_AUTOCCANCEL);
			return 1;
		}
        if (e == FL_PASTE){
            const char *filename = Fl::event_text();
            if (filename) {
                fprintf(stderr, "Dropped: %s\n", filename);

                // OPTIONAL: Remove trailing newline or quotes
                std::string path = filename;
                if (!path.empty() && path.back() == '\n')
                    path.pop_back();

                std::ifstream file(path);
                if (file.is_open()) {
                    std::stringstream buffer;
                    buffer << file.rdbuf();
                    std::string contents = buffer.str();

                    this->SendEditor(SCI_SETTEXT, 0, (sptr_t)contents.c_str());
                    return 1;
                }
            }
            return Fl_Scintilla::handle(FL_PASTE);  
        }

		// Chame o comportamento padrão
		return Fl_Scintilla::handle(e);
	}
};

int main(){
	Fl::scheme("gleam");
    Fl::get_system_colors();

	Fl_Window *win = new Fl_Double_Window(0, 0, 1024, 576, "scintilla for fltk");

	// win->color(fl_rgb_color(0, 128, 128));

	win->begin();
	editor = new fl_scintilla(0, 0, 800, 500);
    // sci_init();
    
	win->end();
	win->resizable(win);
	win->show( );

    editor->SetNotify(cb_editor, 0);



	

	editor->SendEditor(SCI_SETLEXER, SCLEX_LUA);
	printf("STYLE_LASTPREDEFINED %d\n",STYLE_LASTPREDEFINED);
	for (int i = 0; i < STYLE_LASTPREDEFINED; ++i) {
		// editor->SendEditor(SCI_STYLESETFONT, i, (sptr_t)"Courier New");
		editor->SendEditor(SCI_STYLESETFORE, STYLE_DEFAULT, RGB(0, 0, 0));
		editor->SendEditor(SCI_STYLESETSIZE, i, 12);  
	} 
 
	string keys0="function end if then local print for do while break";
	editor->SendEditor(SCI_SETKEYWORDS, 0, (sptr_t)keys0.c_str());
	editor->SendEditor(SCI_STYLESETFORE, SCE_LUA_WORD, RGB(0, 0, 255));

	int version = editor->SendEditor( 267, 0, 0);
	// Version is encoded as: MMmm (e.g., 0x030700 for version 3.7.0)
	int major = (version >> 8) & 0xFF;
	int minor = version & 0xFF;	
	printf("Scintilla version: %d.%d\n", major, minor);

	editor->SendEditor(SCI_SETMARGINTYPEN,2, SC_MARGIN_NUMBER);
    editor->SendEditor(SCI_SETMARGINWIDTHN,2, 20);
	// char fontname[100] = {0};
	// editor->SendEditor(SCI_STYLEGETFONT,STYLE_LINENUMBER,(sptr_t)fontname);
	// printf("fontname %s\n",fontname);
	editor->SendEditor(SCI_STYLESETFONT, STYLE_LINENUMBER, (sptr_t)"");
    editor->SendEditor(SCI_STYLESETSIZE, STYLE_LINENUMBER, 9);

    editor->SendEditor(SCI_SETTABWIDTH, 4);

    editor->SendEditor(SCI_SETPROPERTY,(sptr_t)"fold",(sptr_t)"1");
    editor->SendEditor(SCI_SETMARGINTYPEN, MARGIN_FOLD_INDEX, SC_MARGIN_SYMBOL);
    editor->SendEditor(SCI_SETMARGINMASKN, MARGIN_FOLD_INDEX, SC_MASK_FOLDERS);
    editor->SendEditor(SCI_SETMARGINWIDTHN, MARGIN_FOLD_INDEX, 11);
    editor->SendEditor(SCI_SETMARGINSENSITIVEN, MARGIN_FOLD_INDEX, true);    
	editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDER, SC_MARK_CIRCLEPLUS);
    editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPEN, SC_MARK_ARROW);
    editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEREND,  SC_MARK_CIRCLEPLUSCONNECTED);
    editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPENMID, SC_MARK_CIRCLEMINUSCONNECTED);
    editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_TCORNERCURVE);
    editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERSUB, SC_MARK_VLINE);
    editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERTAIL, SC_MARK_LCORNERCURVE);
	editor->SendEditor(SCI_MARKERSETBACK, SC_MARKNUM_FOLDERSUB, 0xa0a0a0);
    editor->SendEditor(SCI_MARKERSETBACK, SC_MARKNUM_FOLDERMIDTAIL, 0xa0a0a0);
    editor->SendEditor(SCI_MARKERSETBACK, SC_MARKNUM_FOLDERTAIL, 0xa0a0a0);
    editor->SendEditor(SCI_SETFOLDFLAGS, 16|4, 0);
	
	editor->SendEditor( SCI_SETPROPERTY, reinterpret_cast<sptr_t>("fold.compact"), reinterpret_cast<sptr_t>("0"));
	editor->SendEditor(SCI_SETPROPERTY, reinterpret_cast<sptr_t>("fold.by.indentation"), reinterpret_cast<sptr_t>("1"));


	editor->SendEditor(SCI_STYLESETFORE, SCE_LUA_COMMENT, 0x00008000);
    editor->SendEditor(SCI_STYLESETFORE, SCE_LUA_COMMENTLINE, 0x00008000);
    editor->SendEditor(SCI_STYLESETFORE, SCE_LUA_COMMENTDOC, 0x00008000);


    editor->SendEditor(SCI_SETCARETLINEVISIBLE, 1);
    editor->SendEditor(SCI_SETCARETLINEBACK, 0xb0ffff);
    editor->SendEditor(SCI_SETMARGINTYPEN,0,SC_MARGIN_SYMBOL);

	// word wrap
	editor->SendEditor(SCI_SETWRAPMODE,SC_WRAP_WORD,0);    
    // Enable word wrap
    editor->SendEditor(  SCI_SETWRAPVISUALFLAGS, SC_WRAPVISUALFLAG_END, 0);    
    // Optionally set wrap indent mode
    editor->SendEditor(  SCI_SETWRAPINDENTMODE, SC_WRAPINDENT_INDENT, 0);

    // editor->SendEditor(SCI_SETPROPERTY, (uptr_t)"dropped-urls", (sptr_t)"1");

	return Fl::run();
}