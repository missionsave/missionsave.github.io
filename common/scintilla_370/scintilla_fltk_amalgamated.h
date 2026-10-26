// Scintilla FLTK amalgamated port — generated from the four supplied files.
// Define SCINTILLA_FLTK_IMPLEMENTATION in exactly one .cpp before including this header.
#ifndef SCINTILLA_FLTK_AMALGAMATED_H
#define SCINTILLA_FLTK_AMALGAMATED_H

// ---- PlatFLTK.h ----
// Scintilla source code edit control
/** @file PlatFLTK.h
 ** Implementation of platform facilities on FLTK.
 **/
// Copyright 1998-2011 by Neil Hodgson <neilh@scintilla.org>
// Copyright 2015-2016 by cyantree <cyantree.guo@gmail.com>
// Emscripten fixes 2026

#ifndef PLATFLTK_H
#define PLATFLTK_H

#include "FL/Fl_Group.H"
#include "FL/Fl_Double_Window.H"
#include "FL/fl_draw.H"

// #include "CallTip.h"
#include "Scintilla.h"

#include <stdlib.h>
#include <string.h>

// iconv is optional; under Emscripten we prefer a pure UTF-8 path
#if !defined(__EMSCRIPTEN__)
#include "iconv.h"
#endif

typedef struct {
	unsigned char type; // 0-Fl_ScintillaBase, 1-listbox, 2-calltip
	void *wid;

	struct {
		int x, y, w, h;
	} rc_client;

	struct {
		int x, y;
	} main_client;
} SCIWinType;

class ICONVConverter
{
private:
#if !defined(__EMSCRIPTEN__)
	iconv_t cd;
#endif
	char *out_;
	int outlen_;
public:
	ICONVConverter()
	{
#if !defined(__EMSCRIPTEN__)
		cd = (iconv_t)-1;
#endif
		out_ = NULL;
		outlen_ = 0;
	}
	~ICONVConverter()
	{
		Close();
		if (out_ != NULL) free(out_);
	}

	unsigned char Open4UTF8(int codepage)
	{
#ifdef __EMSCRIPTEN__
		(void)codepage;
		return 1; // always "UTF-8"
#else
		if (codepage == 936)   return Open("GB18030", "UTF-8");
		if (codepage == 950)   return Open("BIG5", "UTF-8");
		if (codepage == 949)   return Open("HANGUL", "UTF-8");
		if (codepage == 1361)  return Open("JOHAB", "UTF-8");
		if (codepage == 28605) return Open("ISO-8859-15", "UTF-8");
		return Open("", "UTF-8");
#endif
	}

	unsigned char Open4MB(int codepage)
	{
#ifdef __EMSCRIPTEN__
		(void)codepage;
		return 1;
#else
		if (codepage == 936)   return Open("UTF-8", "GB18030");
		if (codepage == 950)   return Open("UTF-8", "BIG5");
		if (codepage == 949)   return Open("UTF-8", "HANGUL");
		if (codepage == 1361)  return Open("UTF-8", "JOHAB");
		if (codepage == 28605) return Open("UTF-8", "ISO-8859-15");
		return Open("UTF-8", "");
#endif
	}

	unsigned char OpenFromCodepage(int codepage, char *to)
	{
#ifdef __EMSCRIPTEN__
		(void)codepage; (void)to;
		return 1;
#else
		if (codepage == 936)   return Open("GB18030", to);
		if (codepage == 950)   return Open("BIG5", to);
		if (codepage == 949)   return Open("HANGUL", to);
		if (codepage == 1361)  return Open("JOHAB", to);
		if (codepage == 28605) return Open("ISO-8859-15", to);
		return Open("", to);
#endif
	}

	unsigned char OpenToCodepage(char *from, int codepage)
	{
#ifdef __EMSCRIPTEN__
		(void)from; (void)codepage;
		return 1;
#else
		if (codepage == 936)   return Open(from, "GB18030");
		if (codepage == 950)   return Open(from, "BIG5");
		if (codepage == 949)   return Open(from, "HANGUL");
		if (codepage == 1361)  return Open(from, "JOHAB");
		if (codepage == 28605) return Open(from, "ISO-8859-15");
		return Open(from, "");
#endif
	}

	unsigned char Open(const char *from_charset, const char *to_charset)
	{
#ifdef __EMSCRIPTEN__
		(void)from_charset; (void)to_charset;
		return 1;
#else
		Close();
		cd = iconv_open(to_charset, from_charset);
		if (cd != (iconv_t)-1) return 1;
		cd = (iconv_t)-1;
		return 0;
#endif
	}

	void Close()
	{
#if !defined(__EMSCRIPTEN__)
		if (cd != (iconv_t)-1) {
			iconv_close(cd);
			cd = (iconv_t)-1;
		}
#endif
	}

	int GetLength(char *src, int len)
	{
		if (len < 1) return 0;
#ifdef __EMSCRIPTEN__
		// UTF-8 identity
		return len;
#else
		if (cd == (iconv_t)-1) return 0;
		int outlen = len * 8;
		void *p;
		if (outlen > outlen_) {
			p = realloc(out_, outlen);
			if (p == NULL) return 0;
			out_ = (char *)p;
			outlen_ = outlen;
		}
		return convert(src, len, out_, outlen);
#endif
	}

	// FIXED: removed the early "return 0;" that made every conversion fail
	int convert(char *inbuf, int inlen, char *outbuf, int outlen)
	{
		if (!inbuf || inlen < 1 || !outbuf || outlen < 1) return 0;

#ifdef __EMSCRIPTEN__
		// Identity copy (document is always UTF-8)
		int n = inlen < outlen ? inlen : outlen;
		memcpy(outbuf, inbuf, n);
		return n;
#else
		if (cd == (iconv_t)-1) return 0;

		char *pin = inbuf;
		char *pout = outbuf;
		size_t inleft = (size_t)inlen;
		size_t outleft = (size_t)outlen;

		size_t r = iconv(cd, &pin, &inleft, &pout, &outleft);
		if (r == (size_t)-1) return 0;
		return (int)(outlen - (int)outleft);
#endif
	}
};

extern void Platform_Initialise();
extern void Platform_Finalise();

#endif

// ---- Fl_Scintilla.h ----
// Copyright 2015-2016 by cyantree <cyantree.guo@gmail.com>

#ifndef FL_SCINTILLA_H
#define FL_SCINTILLA_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <time.h>

#include <cmath>
#include <stdexcept>
#include <new>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

#undef PLAT_WIN
#include "Platform.h"

#include "ILexer.h"
#include "Scintilla.h"

#ifdef SCI_LEXER
#include "SciLexer.h"
#endif
#include "StringCopy.h"
#ifdef SCI_LEXER
#include "LexerModule.h"
#endif
#include "Position.h"
#include "SplitVector.h"
#include "Partitioning.h"
#include "RunStyles.h"
#include "ContractionState.h"
#include "CellBuffer.h"
#include "CallTip.h"
#include "KeyMap.h"
#include "Indicator.h"
#include "XPM.h"
#include "LineMarker.h"
#include "Style.h"
#include "ViewStyle.h"
#include "CharClassify.h"
#include "Decoration.h"
#include "CaseFolder.h"
#include "Document.h"
#include "CaseConvert.h"
#include "UniConversion.h"
#include "Selection.h"
#include "PositionCache.h"
#include "EditModel.h"
#include "MarginView.h"
#include "EditView.h"
#include "Editor.h"

#include "AutoComplete.h"
#include "ScintillaBase.h"

#ifdef SCI_LEXER
#include "ExternalLexer.h"
#endif

#include "FL/Fl.H"
#include "FL/fl_draw.H"
#include "FL/Fl_Group.H"
#include "FL/Fl_Widget.H"
#include "FL/Fl_Scrollbar.H"
#include "FL/Fl_Window.H"
#include "FL/Fl_Menu_Button.H"
#include "FL/Fl_Double_Window.H"
#include "FL/Fl_Box.H"

class Fl_Scintilla : public Fl_Group, public Scintilla::ScintillaBase	
{
public:
	explicit Fl_Scintilla(int X, int Y, int W, int H, const char* l = 0);
	virtual ~Fl_Scintilla();

	void SetNotify(void OnNotify(Scintilla::SCNotification *scn, void *data), void *data) 
	{
		cb_notify_.data = data;
		cb_notify_.callback = OnNotify;
	}

	sptr_t SendEditor(unsigned int iMessage, uptr_t wParam=0, sptr_t lParam=0)
	{
		return ScintillaBase::WndProc(iMessage, wParam, lParam);
	}

protected:
	struct {
		void (*callback)(Scintilla::SCNotification *scn, void *data);
		void *data;
	} cb_notify_;

	void draw();
	int handle(int e);
	void resize(int X, int Y, int W, int H);

// protected:
public:
	void patchfix();

	ICONVConverter *ic_;
	char *ic_str_;
	int ic_str_len_;

	Scintilla::Surface *sw_;
	
	SCIWinType swt_;
	Fl_Scrollbar* mHScrollBar;
	Fl_Scrollbar* mVScrollBar;
	int scrollbar_width_;
	unsigned char scrollpush_; // 0-not click, 1-vscroll, 2-hscroll

	static void h_scrollbar_cb(Fl_Scrollbar* w, Fl_Scintilla* d);
	static void v_scrollbar_cb( Fl_Scrollbar* w, Fl_Scintilla* d);
	void update_v_scrollbar();
	void update_h_scrollbar();
	int scrollbar_width() const { return scrollbar_width_; }
	void scrollbar_width(int W) { scrollbar_width_ = W; }
// protected://
	int handle_key(int event);

	bool lastKeyDownConsumed;
	bool capturedMouse;
	bool LButtonDown;

	unsigned int linesPerScroll;	///< Intellimouse support
	int wheelDelta; ///< Wheel delta from roll	

	virtual void Initialise();
	virtual void Finalise();

	int CodePageOfDocument() const;
	virtual bool ValidCodePage(int codePage) const;

	virtual void ScrollText(int linesToMove);
	virtual void SetVerticalScrollPos();
	virtual void SetHorizontalScrollPos();
	virtual bool ModifyScrollBars(int nMax, int nPage);

	virtual void ClaimSelection() {}
	virtual void NotifyChange();
	virtual void NotifyFocus(bool focus);
	virtual void NotifyParent(Scintilla::SCNotification scn);
	//virtual void NotifyDoubleClick(Point pt, int modifiers); // do not need
	virtual bool FineTickerAvailable() { return true; }
	virtual bool FineTickerRunning(Scintilla::Editor::TickReason reason);
	virtual void FineTickerStart(Scintilla::Editor::TickReason reason, int millis, int tolerance);
	virtual void FineTickerCancel(Scintilla::Editor::TickReason reason);
	virtual bool SetIdle(bool on);
	virtual void SetMouseCapture(bool on);
	virtual bool HaveMouseCapture();
	virtual void SetTrackMouseLeaveEvent(bool on);
	virtual bool PaintContains(Scintilla::PRectangle rc);
	void FullPaint();

	// do not need
	//virtual void SetCtrlID(int identifier);
	//virtual int GetCtrlID();

	virtual void Copy();
	virtual void CopyAllowLine();
	virtual bool CanPaste();
	virtual void CopyToClipboard(const Scintilla::SelectionText &selectedText);
	virtual void Paste();
	void DoPaste(const char *s);

	char *drag_str_;
	int drag_str_size_;
	virtual bool DragThreshold(Scintilla::Point ptStart, Scintilla::Point ptNow);
	virtual void StartDrag();
	void DragEnter();
	int DragOver();
	void DragLeave();
	void Drop();
	void SaveDragData(const Scintilla::SelectionText &selectedText);

	virtual Scintilla::CaseFolder *CaseFolderForEncoding();
	virtual std::string CaseMapString(const std::string &s, int caseMapping);

	Fl_Window *callwin_;
	virtual void CreateCallTipWindow(Scintilla::PRectangle rc);

	static void cb_popmenu(Fl_Widget *widget, void *data)
	{
		Fl_Scintilla *sci = (Fl_Scintilla *)(widget->user_data());
#if __APPLE__
		int cmd = (uintptr_t)data;
#else
		int cmd = (int)(intptr_t)data;
		// int cmd = (int)data;
#endif
		sci->Command(cmd);
	}
	virtual void AddToPopUp(const char *label, int cmd = 0, bool enabled = true)
	{
		Fl_Menu_Button *m = (Fl_Menu_Button*)(popup.GetID());
		m->user_data(this);
		if (!label[0]) {
			//m->add("_");
		} else if (enabled) {
			if ( strcmp(label, "Redo") == 0 ) m->add(label, 0, cb_popmenu, (void*)cmd, FL_MENU_DIVIDER); 
			else if ( strcmp(label, "Delete") == 0 ) m->add(label, 0, cb_popmenu, (void*)cmd, FL_MENU_DIVIDER);
			else m->add(label, 0, cb_popmenu, (void*)cmd);
			//m->add(label, 0, cb_popmenu, (void*)cmd);
		} else {
			if ( strcmp(label, "Redo") == 0 ) m->add(label, 0, cb_popmenu, (void*)cmd, FL_MENU_INACTIVE|FL_MENU_DIVIDER); 
			else if ( strcmp(label, "Delete") == 0 ) m->add(label, 0, cb_popmenu, (void*)cmd, FL_MENU_INACTIVE|FL_MENU_DIVIDER);
			else m->add(label, 0, cb_popmenu, (void*)cmd, FL_MENU_INACTIVE);
			//m->add(label, 0, 0, 0, FL_MENU_INACTIVE);
		}
	}

	virtual sptr_t DefWndProc(unsigned int iMessage, uptr_t wParam, sptr_t lParam) { return 0; } // ok

	friend class ScintillaEditBase;

private:
	unsigned char timeractive_[5];
	double timetick_[5];
	static void static_time_fun_1(void *x) { Fl_Scintilla *sci = (Fl_Scintilla*)x; sci->time_fun(0); }
	static void static_time_fun_2(void *x) { Fl_Scintilla *sci = (Fl_Scintilla*)x; sci->time_fun(1); }
	static void static_time_fun_3(void *x) { Fl_Scintilla *sci = (Fl_Scintilla*)x; sci->time_fun(2); }
	static void static_time_fun_4(void *x) { Fl_Scintilla *sci = (Fl_Scintilla*)x; sci->time_fun(3); }
	static void static_time_fun_5(void *x) { Fl_Scintilla *sci = (Fl_Scintilla*)x; sci->time_fun(4); }
	void time_fun(int index);

	unsigned int idle_dwstart_;
	static void static_time_idle(void *x) { Fl_Scintilla *sci = (Fl_Scintilla*)x; sci->time_fun_idle(); }
	void time_fun_idle();

	// For use in creating a system caret
	bool HasCaretSizeChanged() const;
	bool CreateSystemCaret();
	bool DestroySystemCaret();
	int sysCaretWidth;
	int sysCaretHeight;

	void RefreshIME();
	void MoveImeCarets(int offset);
	virtual void UpdateSystemCaret();

public:
	void CallTip_DoPush();
};

#endif

// Define SCINTILLA_FLTK_IMPLEMENTATION in exactly one .cpp before including this header.
#ifdef SCINTILLA_FLTK_IMPLEMENTATION
// ---- PlatFLTK.cxx ----
// Scintilla source code edit control

/** @file PlatFLTK.cxx

 ** Implementation of platform facilities on FLTK.

 **/

// Copyright 1998-2003 by Neil Hodgson <neilh@scintilla.org>

// The License.txt file describes the conditions under which this software may be distributed.

// Copyright 2015-2016 by cyantree <cyantree.guo@gmail.com>

// Emscripten / FLTK-wasm fixes 2026

#include <stdlib.h>

#include <string.h>

#include <stdio.h>

#include <stdarg.h>

#include <time.h>

#include <math.h>

#include <ctype.h>

#include <limits.h>

#include <vector>

#include <map>

#include "Platform.h"

#include "StringCopy.h"

#include "XPM.h"

#include "UniConversion.h"

#include "FontQuality.h"

#include "FL/Fl.H"

#include "FL/Fl_Widget.H"

#include "FL/Fl_Group.H"

#include "FL/fl_ask.H"

#include "FL/Fl_Window.H"

#include "FL/Fl_Double_Window.H"

#include "FL/Fl_Hold_Browser.H"

#include "FL/Fl_RGB_Image.H"

#include "FL/fl_draw.H"

#include "FL/Fl_Menu_Button.H"

#include "FL/Fl_Copy_Surface.H"

#include "FL/Fl_Image_Surface.H"

#include "FL/Fl_Menu_Window.H"

// FL/x.H is X11-only; do not pull it in under Emscripten or pure Win32 builds

#if !defined(__EMSCRIPTEN__) && !defined(_WIN32)

#include "FL/x.H"

#endif

// ====================== Platform::Point ===================================

Scintilla::Point Scintilla::Point::FromLong(long lpoint) 

{

    return Point(Platform::LowShortFromLong(lpoint), Platform::HighShortFromLong(lpoint));

}

// ====================== Platform::Font ===================================

class FontData {

public:

    int size_;

    char *name_;

    Fl_Font ff_;

    FontData(Fl_Font ff, char *name, int size)

    {

        size_ = size;

        name_ = NULL;

        if ( name != NULL ) name_ = (char*)malloc(strlen(name)+1);

        if (name_) strcpy(name_, name);

        ff_ = ff;

    }

    ~FontData()

    {

        if ( name_ != NULL ) {

            free(name_);

            name_ = NULL;

        }

    }

    // FIXED: was "strcmp && size" which is always wrong

    unsigned char SameAs(const char *name, int size)

    {

        if ( name_ && name && !strcmp(name, name_) && size == size_ ) return 1;

        return 0;

    }

};

class FontCached : Scintilla::Font {

    FontCached *next;

    int usage;

    FontData *fd;

    explicit FontCached(Fl_Font ff, char *name, int size);

    ~FontCached() {}

    virtual void Release();

    static FontCached *first;

public:

    FontData *GetFD() { return fd; }

    static Scintilla::FontID FindOrCreate(const Scintilla::FontParameters &fp);

    static void ReleaseId(Scintilla::FontID fid_);

};

FontCached *FontCached::first = 0;

FontCached::FontCached(Fl_Font ff, char *name, int size) : next(0), usage(0), fd(0)

{

    fd = new FontData(ff, name, size);

    fid = (void*)fd;

    usage = 1;

}

void FontCached::Release() 

{

    if ( fd != NULL ) {

        delete fd;

        fd = NULL;

    }

    fid = 0;

}

Scintilla::FontID FontCached::FindOrCreate(const Scintilla::FontParameters &fp)

{

#ifdef __EMSCRIPTEN__

    // No fontconfig / no system fonts – use FLTK built-ins only

    Fl_Font ff = FL_HELVETICA;

    const char *name = fp.faceName ? fp.faceName : "";

    if (strstr(name, "Courier") || strstr(name, "Mono") || strstr(name, "Consolas"))

        ff = FL_COURIER;

    else if (strstr(name, "Times"))

        ff = FL_TIMES;

    else if (strstr(name, "Bold"))

        ff = FL_HELVETICA_BOLD;

    int useSize = (int)fp.size;

    if (useSize < 8) useSize = 12;

    // reuse cache if same face+size already exists

    for (FontCached *cur = first; cur; cur = cur->next) {

        if (cur->GetFD()->ff_ == ff && cur->GetFD()->size_ == useSize) {

            cur->usage++;

            return cur->fid;

        }

    }

    FontCached *fc = new FontCached(ff, (char*)(fp.faceName ? fp.faceName : "Helvetica"), useSize);

    fc->next = first;

    first = fc;

    return fc->fid;

#else 

    int n = Fl::set_fonts("-*");

    Scintilla::FontID ret = 0;

    for (FontCached *cur=first; cur; cur=cur->next) {

        if ( cur->GetFD()->SameAs(fp.faceName, (int)fp.size) ) {

            cur->usage++;

            ret = cur->fid;

            return ret;   // found – return immediately

        }

    }

    if (ret == 0) {

        int i;

        Fl_Font ff = FL_HELVETICA;   // safe default under Emscripten

        for (i=0; i<n; i++) {

            int t;

            const char *name = Fl::get_font_name((Fl_Font)i, &t);

            if ( name && strcmp(name, fp.faceName) == 0 ) {

                ff = (Fl_Font)i;

                break;

            }

        }

        int useSize = (int)fp.size;

#if defined(_WIN32) && !defined(__EMSCRIPTEN__)

        HDC hdc = GetDC(NULL);

        if (hdc) {

            useSize = MulDiv(useSize, GetDeviceCaps(hdc, LOGPIXELSY), 72);

            ReleaseDC(NULL, hdc);

        }

#endif

        // Under Emscripten / high-DPI FLTK the point size is already correct

        if (useSize < 1) useSize = 10;

        FontCached *fc = new FontCached(ff, (char*)fp.faceName, useSize);

        fc->next = first;

        first = fc;

        ret = fc->fid;

    }

    return ret;

#endif

}

void FontCached::ReleaseId(Scintilla::FontID fid_) 

{

    FontCached **pcur=&first;

    for (FontCached *cur=first; cur; cur=cur->next) {

        if (cur->fid == fid_) {

            cur->usage--;

            if (cur->usage == 0) {

                *pcur = cur->next;

                cur->Release();

                cur->next = 0;

                delete cur;

            }

            break;

        }

        pcur=&cur->next;

    }

}

Scintilla::Font::Font() 

{

    fid = 0;

}

Scintilla::Font::~Font() 

{

}

#define FONTS_CACHED

void Scintilla::Font::Create(const FontParameters &fp) 

{

    Release();

    if (fp.faceName) {

        fid = FontCached::FindOrCreate(fp);

    }

}

void Scintilla::Font::Release() 

{

    if (fid) FontCached::ReleaseId(fid);

    fid = 0;

}

// ====================== Platform::Surface ===================================

#ifdef __EMSCRIPTEN__

static inline int WasmPxSize(int pointSize)
{
    float h, v;
    Fl::screen_dpi(h, v, 0);
    if (v < 1.0f) v = 96.0f;
    int px = (int)((pointSize * v + 36.0f) / 72.0f);
    if (px < 1) px = 1;
    return px;
}

static inline int WasmDescent(int pxSize) { return (pxSize * 28 + 50) / 100; }
static inline int WasmHeight(int pxSize)  { return (pxSize * 140 + 50) / 100; }
#endif

static inline int GetFlSize(int size) {
#ifdef __EMSCRIPTEN__
    return WasmPxSize(size);
#else
    return size;
#endif
}

class SurfaceFLTK : public Scintilla::Surface

{

private:

    ICONVConverter *ic_;

    int ic_codepage_;

    char *ic_src_;

    int ic_src_size_, ic_srclen_;

    char *ic_dst_;

    int ic_dstsize_, ic_dstlen_;

    char *dst_;

    int dstsize_, dstlen_;

private:

    SCIWinType *swt_;

    Fl_Group *scibase_;

    Fl_Window *win_;

    unsigned char drawtype_; // 0-memory dc, 1-scibase, 2-win

    bool unicodeMode;

    int codePage;

    int px_, py_;

    Fl_Color forecolor_;

    bool clipped_;

    // NEW: marks a pixmap surface as "exists" even when we cannot allocate an
    // offscreen buffer (Emscripten). Scintilla asserts Initialised() on
    // margin pixmaps, so we must return true here.
    bool pixmap_init_;

    // Private so SurfaceGDI objects can not be copied

    SurfaceFLTK(const SurfaceFLTK &);

    SurfaceFLTK &operator=(const SurfaceFLTK &);

    // Helper: origin offset when drawing into the live Scintilla widget

    int ox() const { return (drawtype_ == 1 && swt_) ? swt_->rc_client.x : 0; }

    int oy() const { return (drawtype_ == 1 && swt_) ? swt_->rc_client.y : 0; }

public:

    Fl_Offscreen offscreen_buffer;

    int off_width, off_height;

    SurfaceFLTK();

    virtual ~SurfaceFLTK();

    virtual void Init(Scintilla::WindowID wid);

    virtual void Init(Scintilla::SurfaceID sid, Scintilla::WindowID wid);

    virtual void InitPixMap(int width, int height, Scintilla::Surface *surface_, Scintilla::WindowID wid);

    virtual void Release();

    virtual bool Initialised();

    virtual void PenColour(Scintilla::ColourDesired fore);

    virtual int LogPixelsY();

    virtual int DeviceHeightFont(int points);

    virtual void MoveTo(int x_, int y_);

    virtual void LineTo(int x_, int y_);

    virtual void Polygon(Scintilla::Point *pts, int npts, Scintilla::ColourDesired fore, Scintilla::ColourDesired back);

    virtual void RectangleDraw(Scintilla::PRectangle rc, Scintilla::ColourDesired fore, Scintilla::ColourDesired back);

    virtual void FillRectangle(Scintilla::PRectangle rc, Scintilla::ColourDesired back);

    virtual void FillRectangle(Scintilla::PRectangle rc, Scintilla::Surface &surfacePattern);

    virtual void RoundedRectangle(Scintilla::PRectangle rc, Scintilla::ColourDesired fore, Scintilla::ColourDesired back);

    virtual void AlphaRectangle(Scintilla::PRectangle rc, int cornerSize, Scintilla::ColourDesired fill, int alphaFill, Scintilla::ColourDesired outline, int alphaOutline, int flags);

    virtual void DrawRGBAImage(Scintilla::PRectangle rc, int width, int height, const unsigned char *pixelsImage);

    virtual void Ellipse(Scintilla::PRectangle rc, Scintilla::ColourDesired fore, Scintilla::ColourDesired back);

    virtual void Copy(Scintilla::PRectangle rc, Scintilla::Point from, Scintilla::Surface &surfaceSource);

    virtual void DrawTextNoClip(Scintilla::PRectangle rc, Scintilla::Font &font_, Scintilla::XYPOSITION ybase, const char *s, int len, Scintilla::ColourDesired fore, Scintilla::ColourDesired back);

    virtual void DrawTextClipped(Scintilla::PRectangle rc, Scintilla::Font &font_, Scintilla::XYPOSITION ybase, const char *s, int len, Scintilla::ColourDesired fore, Scintilla::ColourDesired back);

    virtual void DrawTextTransparent(Scintilla::PRectangle rc, Scintilla::Font &font_, Scintilla::XYPOSITION ybase, const char *s, int len, Scintilla::ColourDesired fore);

    unsigned char Ansi2Utf(const char *s, int len);

    unsigned char Copy2Buffer(const char *s, int len);

    virtual void MeasureWidths(Scintilla::Font &font_, const char *s, int len, Scintilla::XYPOSITION *positions);

    virtual Scintilla::XYPOSITION WidthText(Scintilla::Font &font_, const char *s, int len);

    virtual Scintilla::XYPOSITION WidthChar(Scintilla::Font &font_, char ch);

    virtual Scintilla::XYPOSITION Ascent(Scintilla::Font &font_);

    virtual Scintilla::XYPOSITION Descent(Scintilla::Font &font_);

    virtual Scintilla::XYPOSITION InternalLeading(Scintilla::Font &font_);

    virtual Scintilla::XYPOSITION ExternalLeading(Scintilla::Font &font_);

    virtual Scintilla::XYPOSITION Height(Scintilla::Font &font_);

    virtual Scintilla::XYPOSITION AverageCharWidth(Scintilla::Font &font_);

    virtual void SetClip(Scintilla::PRectangle rc);

    virtual void FlushCachedState();

    virtual void SetUnicodeMode(bool unicodeMode_);

    virtual void SetDBCSMode(int codePage_);

};

SurfaceFLTK::SurfaceFLTK() 

{

    scibase_ = 0;

    win_ = 0;

    drawtype_ = 0;

    clipped_ = false;

    offscreen_buffer = 0;

    off_width = 0;

    off_height = 0;

    unicodeMode = false;

    codePage = 0;

    px_ = py_ = 0;

    forecolor_ = FL_BLACK;

    swt_ = 0;

    pixmap_init_ = false;

    //

    ic_ = new ICONVConverter();

    ic_codepage_ = 0;

    ic_src_ = NULL;

    ic_src_size_ = 0;

    ic_srclen_ = 0;

    ic_dst_ = NULL;

    ic_dstsize_ = 0;

    ic_dstlen_ = 0;

    dst_ = NULL;

    dstsize_ = 0;

    dstlen_ = 0;

}

SurfaceFLTK::~SurfaceFLTK()

{

    Release();

    //

    delete ic_;

    if ( ic_src_ != NULL ) free(ic_src_);

    if ( ic_dst_ != NULL ) free(ic_dst_);

    if ( dst_ != NULL ) free(dst_);

}

bool SurfaceFLTK::Initialised()

{

    // A pixmap surface is considered valid even if the offscreen buffer could
    // not be allocated (Emscripten). Scintilla asserts Initialised() on the
    // margin pixmaps; the draw methods are safe no-ops in that case.
    if ( pixmap_init_ ) return true;

    if ( scibase_ == 0 && win_ == 0 && offscreen_buffer == 0 ) return false;

    return true;

}

void SurfaceFLTK::Init(Scintilla::WindowID wid)

{

    Release();

    swt_ = (SCIWinType *)wid;

    scibase_ = (Fl_Group *)swt_->wid;

    drawtype_ = 1;

}

void SurfaceFLTK::Init(Scintilla::SurfaceID sid, Scintilla::WindowID wid)

{

    Release();

    swt_ = (SCIWinType *)wid;

    if ( sid == (void*)1 ) {

        scibase_ = (Fl_Group *)swt_->wid;

        drawtype_ = 1;

    } else if ( sid == (void*)2 ) {

        win_ = (Fl_Window *)swt_->wid;

        drawtype_ = 2;

    }   

}

void SurfaceFLTK::InitPixMap(int width, int height, Scintilla::Surface *surface_, Scintilla::WindowID wid)

{

    Release();

    SurfaceFLTK *psurfOther = (SurfaceFLTK *)surface_;

    SetUnicodeMode(psurfOther->unicodeMode);

    SetDBCSMode(psurfOther->codePage);

    off_width = width; off_height = height;

    // Mark this surface as a valid pixmap even if we cannot allocate the
    // backing store (see comment on pixmap_init_).
    pixmap_init_ = (width > 0 && height > 0);

#ifdef __EMSCRIPTEN__
    // fl_create_offscreen() switches FLTK's Emscripten graphics driver onto an
    // OffscreenCanvas (Fl_Emscripten_Image_Surface_Driver::set_current) and
    // fl_end_offscreen() never restores the window context. Every later
    // fl_rectf/fl_draw — including fl_copy_offscreen of the line buffer —
    // lands on that hidden canvas, so the editor is blank. X11 restores the
    // GC in pop_current(), which is why the same path works there.
    // Pixmaps stay uninitialised; callers fall back to direct drawing.
    offscreen_buffer = 0;
    (void)wid;
#else
    if (width > 0 && height > 0)

        offscreen_buffer = fl_create_offscreen(width, height);
#endif

    drawtype_ = 0;

}

void SurfaceFLTK::PenColour(Scintilla::ColourDesired fore)

{

    forecolor_ = fl_rgb_color(fore.GetRed(), fore.GetGreen(), fore.GetBlue());

}

int SurfaceFLTK::LogPixelsY() // ok

{

    float h, v;

    Fl::screen_dpi(h, v, 0);

    if (v < 1.0f) v = 96.0f;   // sensible default for Emscripten

    return (int)v;

}

int SurfaceFLTK::DeviceHeightFont(int points) // ok

{

    int logPix = LogPixelsY();

    return (points * logPix + logPix / 2) / 72;

}

void SurfaceFLTK::MoveTo(int x_, int y_)

{

    px_ = x_;

    py_ = y_;

}

void SurfaceFLTK::LineTo(int x_, int y_)

{

    if ( drawtype_ == 0 ) {

        if (!offscreen_buffer) { px_ = x_; py_ = y_; return; }

        fl_begin_offscreen(offscreen_buffer);

        fl_color(forecolor_);

        fl_line(px_, py_, x_, y_);

        fl_end_offscreen();

    } else {

        fl_color(forecolor_);

        fl_line(px_ + ox(), py_ + oy(), x_ + ox(), y_ + oy());

    }

    px_ = x_;

    py_ = y_;

}

void SurfaceFLTK::Polygon(Scintilla::Point *pts, int npts, Scintilla::ColourDesired fore, Scintilla::ColourDesired back)

{

    if ( drawtype_ == 0 ) {

        if (!offscreen_buffer) return;

        fl_begin_offscreen(offscreen_buffer);

        fl_color(fore.GetRed(), fore.GetGreen(), fore.GetBlue());

        fl_begin_polygon();

        for (int i=0; i<npts; i++) {

            fl_vertex(pts[i].x, pts[i].y);

        }

        fl_end_polygon();

        fl_end_offscreen();

    } else {

        fl_color(fore.GetRed(), fore.GetGreen(), fore.GetBlue());

        fl_begin_polygon();

        for (int i=0; i<npts; i++) {

            fl_vertex(pts[i].x + ox(), pts[i].y + oy());

        }

        fl_end_polygon();

    }

}

void SurfaceFLTK::RectangleDraw(Scintilla::PRectangle rc, Scintilla::ColourDesired fore, Scintilla::ColourDesired back)

{

    int x = Scintilla::RoundXYPosition(rc.left);

    int y = Scintilla::RoundXYPosition(rc.top);

    int w = Scintilla::RoundXYPosition(rc.right-rc.left);

    int h = Scintilla::RoundXYPosition(rc.bottom-rc.top);

    if ( drawtype_ == 0 ) {

        if (!offscreen_buffer) return;

        fl_begin_offscreen(offscreen_buffer);

        fl_rect(x, y, w, h, fl_rgb_color(fore.GetRed(), fore.GetGreen(), fore.GetBlue()));

        fl_end_offscreen();

    } else {

        fl_rect(x + ox(), y + oy(), w, h, fl_rgb_color(fore.GetRed(), fore.GetGreen(), fore.GetBlue()));

    }

}

void SurfaceFLTK::FillRectangle(Scintilla::PRectangle rc, Scintilla::ColourDesired back) // ok

{

    // Use floor/ceil to prevent 1px gaps between adjacent line backgrounds
    int x = (int)std::floor(rc.left);

    int y = (int)std::floor(rc.top);

    int w = (int)std::ceil(rc.right) - x;

    int h = (int)std::ceil(rc.bottom) - y;

    if (w <= 0 || h <= 0) return;

    if ( drawtype_ == 0 ) {

        if (!offscreen_buffer) return;

        fl_begin_offscreen(offscreen_buffer);

        fl_color(back.GetRed(), back.GetGreen(), back.GetBlue());

        fl_rectf(x, y, w, h);

        fl_end_offscreen();

    } else {

        fl_color(back.GetRed(), back.GetGreen(), back.GetBlue());

        fl_rectf(x + ox(), y + oy(), w, h);

    }

}

static void GetOffScreenImg(SurfaceFLTK *sf, int w, int h, unsigned char **img)

{

    if (!sf->offscreen_buffer) { *img = NULL; return; }

    fl_begin_offscreen(sf->offscreen_buffer);

    *img = fl_read_image(NULL, 0, 0, w, h, 0);

    fl_end_offscreen();

}

void SurfaceFLTK::FillRectangle(Scintilla::PRectangle rc, Scintilla::Surface &surfacePattern)

{

    int x = Scintilla::RoundXYPosition(rc.left);

    int y = Scintilla::RoundXYPosition(rc.top);

    int w = Scintilla::RoundXYPosition(rc.right-rc.left);

    int h = Scintilla::RoundXYPosition(rc.bottom-rc.top);

    if (w <= 0 || h <= 0) return;

    SurfaceFLTK *sf = (SurfaceFLTK*)&surfacePattern;

    if ( sf->offscreen_buffer != 0 ) {

        int ow = sf->off_width, oh = sf->off_height;

        if (ow <= 0 || oh <= 0) return;

        int i, j, m, n;

        unsigned char *img = NULL;

        GetOffScreenImg(sf, ow, oh, &img);

        if (!img) {

#ifdef __EMSCRIPTEN__
            return;
#else
            // fallback solid red so we can see the problem

            FillRectangle(rc, Scintilla::ColourDesired(0xff, 0, 0));

            return;
#endif

        }

        m = w / ow; if ( w % ow > 0 ) m++;

        n = h / oh; if ( h % oh > 0 ) n++;

        if ( drawtype_ == 0 ) {

            if (!offscreen_buffer) { delete[] img; return; }

            fl_begin_offscreen(offscreen_buffer);

            for (j=0; j<n; j++) {

                for (i=0; i<m; i++) fl_draw_image(img, x+i*ow, y+j*oh, ow, oh, 3, 0);

            }

            fl_end_offscreen();

        } else {

            // FIXED: apply client origin

            for (j=0; j<n; j++) {

                for (i=0; i<m; i++)

                    fl_draw_image(img, x + i*ow + ox(), y + j*oh + oy(), ow, oh, 3, 0);

            }

        }

        delete[] img;

    } else {

#ifdef __EMSCRIPTEN__
        // Pattern pixmaps are offscreens, which this backend cannot host.
        // The line background was already painted; don't cover it in red.
        return;
#else
        // Something is wrong so display in red

        FillRectangle(rc, Scintilla::ColourDesired(0xff, 0, 0));
#endif

    }

}

void SurfaceFLTK::RoundedRectangle(Scintilla::PRectangle rc, Scintilla::ColourDesired fore, Scintilla::ColourDesired back)

{

    int x = Scintilla::RoundXYPosition(rc.left);

    int y = Scintilla::RoundXYPosition(rc.top);

    int w = Scintilla::RoundXYPosition(rc.right-rc.left);

    int h = Scintilla::RoundXYPosition(rc.bottom-rc.top);

    if ( drawtype_ == 0 ) {

        if (!offscreen_buffer) return;

        fl_begin_offscreen(offscreen_buffer);

        fl_color(back.GetRed(), back.GetGreen(), back.GetBlue());

        fl_rectf(x, y, w, h);

        fl_color(fore.GetRed(), fore.GetGreen(), fore.GetBlue());

        fl_rect(x, y, w, h);

        fl_end_offscreen();

    } else {

        fl_color(back.GetRed(), back.GetGreen(), back.GetBlue());

        fl_rectf(x + ox(), y + oy(), w, h);

        fl_color(fore.GetRed(), fore.GetGreen(), fore.GetBlue());

        fl_rect(x + ox(), y + oy(), w, h);

    }

}

// void SurfaceFLTK_AlphaRectangle(Scintilla::PRectangle rc, int cornerSize, Scintilla::ColourDesired fill, int alphaFill, Scintilla::ColourDesired outline, int alphaOutline, int /* flags*/ )

// {

//  int x = Scintilla::RoundXYPosition(rc.left);

//  int y = Scintilla::RoundXYPosition(rc.top);

//  int w = Scintilla::RoundXYPosition(rc.right-rc.left);

//  int h = Scintilla::RoundXYPosition(rc.bottom-rc.top);

//  // Simple outline (alpha ignored on pure FLTK canvas)

//  if ( drawtype_ == 0 ) {

//      if (!offscreen_buffer) return;

//      fl_begin_offscreen(offscreen_buffer);

//      fl_color(outline.GetRed(), outline.GetGreen(), outline.GetBlue());

//      fl_rect(x, y, w, h);

//      fl_end_offscreen();

//  } else {

//      fl_color(outline.GetRed(), outline.GetGreen(), outline.GetBlue());

//      fl_rect(x + ox(), y + oy(), w, h);

//  }

// }

void SurfaceFLTK::AlphaRectangle(Scintilla::PRectangle rc, int cornerSize,

    Scintilla::ColourDesired fill, int alphaFill,

    Scintilla::ColourDesired outline, int alphaOutline, int)

{

    int x = Scintilla::RoundXYPosition(rc.left);

    int y = Scintilla::RoundXYPosition(rc.top);

    int w = Scintilla::RoundXYPosition(rc.right - rc.left);

    int h = Scintilla::RoundXYPosition(rc.bottom - rc.top);

    if (w <= 0 || h <= 0) return;

    // Blend against white (your editor background). Good enough without readback.

    Fl_Color bg = FL_WHITE;

    Fl_Color cf = fl_color_average(

        fl_rgb_color(fill.GetRed(), fill.GetGreen(), fill.GetBlue()),

        bg, alphaFill / 255.0f);

    Fl_Color co = fl_color_average(

        fl_rgb_color(outline.GetRed(), outline.GetGreen(), outline.GetBlue()),

        bg, alphaOutline / 255.0f);

    auto draw = [&](int dx, int dy) {

        fl_color(cf);

        fl_rectf(x + dx, y + dy, w, h);

        if (alphaOutline > 0) {

            fl_color(co);

            fl_rect(x + dx, y + dy, w, h);

        }

    };

    if (drawtype_ == 0) {

        if (!offscreen_buffer) return;

        fl_begin_offscreen(offscreen_buffer);

        draw(0, 0);

        fl_end_offscreen();

    } else {

        draw(ox(), oy());

    }

}

void SurfaceFLTK::DrawRGBAImage(Scintilla::PRectangle rc, int width, int height, const unsigned char *pixelsImage)

{

    if (!pixelsImage || width <= 0 || height <= 0) return;

    // NEW: no offscreen backing on Emscripten – do not paint onto the window.
    if (drawtype_ == 0 && !offscreen_buffer) return;

    int x = Scintilla::RoundXYPosition(rc.left);

    int y = Scintilla::RoundXYPosition(rc.top);

    Fl_RGB_Image img(pixelsImage, width, height, 4);

    img.draw(x + ox(), y + oy());

}

void SurfaceFLTK::Ellipse(Scintilla::PRectangle rc, Scintilla::ColourDesired fore, Scintilla::ColourDesired back)

{

    double x = Scintilla::RoundXYPosition(rc.left);

    double y = Scintilla::RoundXYPosition(rc.top);

    double w = Scintilla::RoundXYPosition(rc.right-rc.left);

    double h = Scintilla::RoundXYPosition(rc.bottom-rc.top);

    if ( drawtype_ == 0 ) {

        if (!offscreen_buffer) return;

        fl_begin_offscreen(offscreen_buffer);

        fl_push_matrix();

        fl_color(back.GetRed(), back.GetGreen(), back.GetBlue());

        fl_begin_complex_polygon();

        fl_circle(x+w/2.0, y+h/2.0, w/2.0);

        fl_end_complex_polygon();

        fl_color(fore.GetRed(), fore.GetGreen(), fore.GetBlue());

        fl_begin_line();

        fl_circle(x+w/2.0, y+h/2.0, w/2.0);

        fl_end_line();

        fl_pop_matrix();

        fl_end_offscreen();

    } else {

        fl_push_matrix();

        fl_color(back.GetRed(), back.GetGreen(), back.GetBlue());

        fl_begin_complex_polygon();

        fl_circle(x+w/2.0 + ox(), y+h/2.0 + oy(), w/2.0);

        fl_end_complex_polygon();

        fl_color(fore.GetRed(), fore.GetGreen(), fore.GetBlue());

        fl_begin_line();

        fl_circle(x+w/2.0 + ox(), y+h/2.0 + oy(), w/2.0);

        fl_end_line();

        fl_pop_matrix();

    }

}

void SurfaceFLTK::Copy(Scintilla::PRectangle rc, Scintilla::Point from, Scintilla::Surface &surfaceSource) // ok

{

    SurfaceFLTK *sf = (SurfaceFLTK*)&surfaceSource;

    if (!sf->offscreen_buffer) return;

    int x, y, w, h, srcx, srcy;

    x = Scintilla::RoundXYPosition(rc.left) + ox();

    y = Scintilla::RoundXYPosition(rc.top)  + oy();

    w = Scintilla::RoundXYPosition(rc.Width());

    h = Scintilla::RoundXYPosition(rc.Height());

    srcx = Scintilla::RoundXYPosition(from.x);

    srcy = Scintilla::RoundXYPosition(from.y);

    if (w <= 0 || h <= 0) return;

    fl_copy_offscreen(x, y, w, h, sf->offscreen_buffer, srcx, srcy);

}

// ---------------------------------------------------------------------------

// Text drawing – CRITICAL fix: apply client origin when drawtype_ == 1

// ---------------------------------------------------------------------------

void SurfaceFLTK::DrawTextNoClip(Scintilla::PRectangle rc, Scintilla::Font &font_, Scintilla::XYPOSITION ybase, const char *s, int len, Scintilla::ColourDesired fore, Scintilla::ColourDesired back)

{

    if (!s || len <= 0) return;

    FontData* fd = (FontData*)(font_.GetID());

    if (!fd) return;

    Fl_Color fc = fl_rgb_color(fore.GetRed(), fore.GetGreen(), fore.GetBlue());

    const int dx = ox();

    const int dy = oy();

    const int tx = Scintilla::RoundXYPosition(rc.left) + dx;

    const int ty = Scintilla::RoundXYPosition(ybase)   + dy;

    if ( drawtype_ == 0 ) {

        if (!offscreen_buffer) return;

        fl_begin_offscreen(offscreen_buffer);

        fl_font(fd->ff_, GetFlSize(fd->size_));

        fl_color(fc);

        if (unicodeMode) {

            fl_draw(s, len, Scintilla::RoundXYPosition(rc.left), Scintilla::RoundXYPosition(ybase));

            fl_end_offscreen();

            return;

        }

        // ansi path (rarely used under Emscripten)

        if ( ic_src_ != NULL && ic_srclen_ == len && ic_dst_ != NULL && ic_dstlen_ > 0 && ic_codepage_ == codePage ) {

            if ( memcmp(ic_src_, s, len) == 0 ) {

                fl_draw(ic_dst_, ic_dstlen_, Scintilla::RoundXYPosition(rc.left), Scintilla::RoundXYPosition(ybase));

                fl_end_offscreen();

                return;

            }

        }

        ic_->Open4UTF8(codePage);

        int cl = ic_->GetLength((char*)s, len);

        if ( cl <= 0 ) {

            fl_draw(s, len, Scintilla::RoundXYPosition(rc.left), Scintilla::RoundXYPosition(ybase));

            fl_end_offscreen();

            return;

        }

        if ( dstsize_ < cl ) {

            void *p = realloc(dst_, cl);

            if ( p == NULL ) {

                fl_draw(s, len, Scintilla::RoundXYPosition(rc.left), Scintilla::RoundXYPosition(ybase));

                fl_end_offscreen();

                return;

            }

            dst_ = (char *)p;

            dstsize_ = cl;

        }

        memset(dst_, 0, dstsize_);

        dstlen_ = ic_->convert((char*)s, len, dst_, dstsize_);

        if ( dstlen_ <= 0 ) {

            fl_draw(s, len, Scintilla::RoundXYPosition(rc.left), Scintilla::RoundXYPosition(ybase));

            fl_end_offscreen();

            return;

        }

        fl_draw(dst_, dstlen_, Scintilla::RoundXYPosition(rc.left), Scintilla::RoundXYPosition(ybase));

        fl_end_offscreen();

        return;

    }

    // ===== live widget (drawtype_ 1 or 2) – ALWAYS apply origin =====

    fl_font(fd->ff_, GetFlSize(fd->size_));

    fl_color(fc);

    if (unicodeMode) {

        fl_draw(s, len, tx, ty);

        return;

    }

    // ansi fallback (kept for native platforms)

    if ( ic_src_ != NULL && ic_srclen_ == len && ic_dst_ != NULL && ic_dstlen_ > 0 && ic_codepage_ == codePage ) {

        if ( memcmp(ic_src_, s, len) == 0 ) {

            fl_draw(ic_dst_, ic_dstlen_, tx, ty);

            return;

        }

    }

    ic_->Open4UTF8(codePage);

    int cl = ic_->GetLength((char*)s, len);

    if ( cl <= 0 ) {

        fl_draw(s, len, tx, ty);

        return;

    }

    if ( dstsize_ < cl ) {

        void *p = realloc(dst_, cl);

        if ( p == NULL ) {

            fl_draw(s, len, tx, ty);

            return;

        }

        dst_ = (char *)p;

        dstsize_ = cl;

    }

    memset(dst_, 0, dstsize_);

    dstlen_ = ic_->convert((char*)s, len, dst_, dstsize_);

    if ( dstlen_ <= 0 ) {

        fl_draw(s, len, tx, ty);

        return;

    }

    fl_draw(dst_, dstlen_, tx, ty);

}

void SurfaceFLTK::DrawTextClipped(Scintilla::PRectangle rc, Scintilla::Font &font_, Scintilla::XYPOSITION ybase, const char *s, int len, Scintilla::ColourDesired fore, Scintilla::ColourDesired back)

{
    // Scintilla's text clip is local to the editor client. FLTK's outer
    // widget clip is not enough: adjacent rows/margins must not be overdrawn.
    const int cx = Scintilla::RoundXYPosition(rc.left) + ox();
    const int cy = Scintilla::RoundXYPosition(rc.top) + oy();
    const int cw = Scintilla::RoundXYPosition(rc.right - rc.left);
    const int ch = Scintilla::RoundXYPosition(rc.bottom - rc.top);
    if (cw <= 0 || ch <= 0 || !s || len <= 0) return;
    if (drawtype_ != 1) {
        DrawTextNoClip(rc, font_, ybase, s, len, fore, back);
        return;
    }
    fl_push_clip(cx, cy, cw, ch);
    DrawTextNoClip(rc, font_, ybase, s, len, fore, back);
    fl_pop_clip();
}

void SurfaceFLTK::DrawTextTransparent(Scintilla::PRectangle rc, Scintilla::Font &font_, Scintilla::XYPOSITION ybase, const char *s, int len, Scintilla::ColourDesired fore)

{

    DrawTextNoClip(rc, font_, ybase, s, len, fore, Scintilla::ColourDesired(0,0,0));

}

Scintilla::XYPOSITION SurfaceFLTK::WidthText(Scintilla::Font &font_, const char *s, int len)

{

    FontData* fd = (FontData*)(font_.GetID());

    if (!fd) return 0;

    fl_font(fd->ff_, GetFlSize(fd->size_));

    return (Scintilla::XYPOSITION)fl_width(s, len);

}

unsigned char SurfaceFLTK::Ansi2Utf(const char *s, int len)

{

    int cl;

    void *p;

    ic_->Open4UTF8(codePage);

    cl = ic_->GetLength((char*)s, len);

    if ( cl <= 0 ) return 0;

    if ( ic_dstsize_ < cl ) {

        p = realloc(ic_dst_, cl);

        if ( p == NULL ) return 0;

        ic_dst_ = (char *)p;

        ic_dstsize_ = cl;

    }

    memset(ic_dst_, 0, ic_dstsize_);

    ic_dstlen_ = ic_->convert((char*)s, len, ic_dst_, ic_dstsize_);

    if ( ic_dstlen_ <= 0 ) return 0;

    if ( ic_src_size_ < len ) {

        p = realloc(ic_src_, len);

        if ( p == NULL ) return 0;

        ic_src_ = (char *)p;

        ic_src_size_ = len;

    }

    memcpy(ic_src_, s, len);

    ic_srclen_ = len;

    ic_codepage_ = codePage;

    return 1;

}

unsigned char SurfaceFLTK::Copy2Buffer(const char *s, int len)

{

    void *p;

    if ( ic_dstsize_ < len ) {

        p = realloc(ic_dst_, len);

        if ( p == NULL ) return 0;

        ic_dst_ = (char *)p;

        ic_dstsize_ = len;

    }

    memcpy(ic_dst_, s, len);

    ic_dstlen_ = len;

    if ( ic_src_size_ < len ) {

        p = realloc(ic_src_, len);

        if ( p == NULL ) return 0;

        ic_src_ = (char *)p;

        ic_src_size_ = len;

    }

    memcpy(ic_src_, s, len);

    ic_srclen_ = len;

    ic_codepage_ = codePage;

    return 1;

}

void SurfaceFLTK::MeasureWidths(Scintilla::Font &font_, const char *s, int len, Scintilla::XYPOSITION *positions)

{

    memset(positions, 0, len*sizeof(Scintilla::XYPOSITION));

    FontData* fd = (FontData*)(font_.GetID());

    if (!fd) return;

    fl_font(fd->ff_, GetFlSize(fd->size_));

    unsigned char isutf=1;

    if (!unicodeMode) {

        isutf = Ansi2Utf(s, len);

        if ( ! isutf ) {

            if ( ! Copy2Buffer(s, len) ) return;

        }

    }

    char *ss;

    int sslen;

    char c;

    int i, u1, n, j;

    double pos;

    char *ansi_s;

    int m, ansi_add;

    if ( (!unicodeMode) && isutf ) {

        ss = ic_dst_;

        sslen = ic_dstlen_;

        n = 0; pos = 0.0;

        ansi_s = (char*)s; m = 0;

        for (i=0; i<sslen;) {

            c = ss[i];

            u1 = fl_utf8len(c);

            if ( u1 > 0 ) {

                pos += fl_width(ss+i, u1);

                ansi_add = 1;

                if (codePage == 932 || codePage == 936 || codePage == 949 || codePage == 950 || codePage == 1361)

                    ansi_add = Scintilla::Platform::IsDBCSLeadByte(codePage, ansi_s[m]) ? 2 : 1;

                m += ansi_add;

                positions[n] = (float)pos;

                n++;

                if ( n >= len ) break;

                for (j=0; j<ansi_add-1; j++ ) {

                    positions[n] = (float)pos;

                    n++;

                    if ( n >= len ) break;

                }

                if ( n >= len ) break;

            }

            if (u1 < 1) i += 1;

            else i += u1;

        }

        const Scintilla::XYPOSITION lastPos = (n>0 && n<=len) ? positions[n - 1] : 0.0f;

        for (i=0; i<len-n;i++) positions[i+n] = lastPos;

        return;

    } 

    if ( unicodeMode ) {

        ss = (char*)s;

        sslen = len;

        n = 0; pos = 0.0;

        for (i=0; i<sslen;) {

            c = ss[i];

            u1 = fl_utf8len(c);

            if ( u1 > 0 ) {

                pos += fl_width(ss+i, u1);

                for (j=0; j<u1; j++ ) {

                    positions[n] = (float)pos;

                    n++;

                    if ( n >= len ) break;

                }

                if ( n >= len ) break;

            }

            if (u1 < 1) i += 1;

            else i += u1;

        }

        const Scintilla::XYPOSITION lastPos = (n>0 && n<=len) ? positions[n - 1] : 0.0f;

        for (i=0; i<len-n;i++) positions[i+n] = lastPos;

        return;

    }

    if ( (!unicodeMode) && (!isutf) ) {

        ss = ic_dst_;

        sslen = ic_dstlen_;

        n = 0; pos = 0.0;

        ansi_s = (char*)s; m = 0;

        for (i=0; i<sslen;) {

            c = ss[i];

            u1 = fl_utf8len(c);

            if ( u1 > 0 ) {

                pos += fl_width(ss+i, u1);

                ansi_add = 1;

                if (codePage == 932 || codePage == 936 || codePage == 949 || codePage == 950 || codePage == 1361)

                    ansi_add = Scintilla::Platform::IsDBCSLeadByte(codePage, ansi_s[m]) ? 2 : 1;

                m += ansi_add;

                positions[n] = (float)pos;

                n++;

                if ( n >= len ) break;

                for (j=0; j<ansi_add-1; j++ ) {

                    positions[n] = (float)pos;

                    n++;

                    if ( n >= len ) break;

                }

                if ( n >= len ) break;

            }

            if (u1 < 1) i += 1;

            else i += u1;

        }

        const Scintilla::XYPOSITION lastPos = (n>0 && n<=len) ? positions[n - 1] : 0.0f;

        for (i=0; i<len-n;i++) positions[i+n] = lastPos;

        return;

    }

}

Scintilla::XYPOSITION SurfaceFLTK::WidthChar(Scintilla::Font &font_, char ch)

{

    FontData* fd = (FontData*)(font_.GetID());

    if (!fd) return 0;

    fl_font(fd->ff_, GetFlSize(fd->size_));

    return (Scintilla::XYPOSITION)fl_width(ch);

}

// helper near the top of PlatFLTK.cxx

#ifdef __EMSCRIPTEN__

static inline int WasmAscent(int pxSize) { return WasmHeight(pxSize) - WasmDescent(pxSize); }

#endif

Scintilla::XYPOSITION SurfaceFLTK::Ascent(Scintilla::Font &font_)

{

    FontData* fd = (FontData*)(font_.GetID());

    if (!fd) return 0;

#ifdef __EMSCRIPTEN__

    return (Scintilla::XYPOSITION)WasmAscent(WasmPxSize(fd->size_));

#else

    fl_font(fd->ff_, fd->size_);

    return (Scintilla::XYPOSITION)(fl_height() - fl_descent());

#endif

}

Scintilla::XYPOSITION SurfaceFLTK::Descent(Scintilla::Font &font_)

{

    FontData* fd = (FontData*)(font_.GetID());

    if (!fd) return 0;

#ifdef __EMSCRIPTEN__

    return (Scintilla::XYPOSITION)WasmDescent(WasmPxSize(fd->size_));

#else

    fl_font(fd->ff_, fd->size_);

    return (Scintilla::XYPOSITION)fl_descent();

#endif

}

Scintilla::XYPOSITION SurfaceFLTK::Height(Scintilla::Font &font_)

{

    FontData* fd = (FontData*)(font_.GetID());

    if (!fd) return 0;

#ifdef __EMSCRIPTEN__

    return (Scintilla::XYPOSITION)WasmHeight(WasmPxSize(fd->size_));

#else

    return (Scintilla::XYPOSITION)fl_height(fd->ff_, fd->size_);

#endif

}

Scintilla::XYPOSITION SurfaceFLTK::InternalLeading(Scintilla::Font &font_)

{

    return 0;

}

Scintilla::XYPOSITION SurfaceFLTK::ExternalLeading(Scintilla::Font &font_)

{

    return 0;

}

Scintilla::XYPOSITION SurfaceFLTK::AverageCharWidth(Scintilla::Font &font_)

{

    FontData* fd = (FontData*)(font_.GetID());

    if (!fd) return 0;

    fl_font(fd->ff_, GetFlSize(fd->size_));

    return (Scintilla::XYPOSITION)fl_width('n');

}

void SurfaceFLTK::SetClip(Scintilla::PRectangle rc)

{

    if (drawtype_ == 0) return;

    if (clipped_) { fl_pop_clip(); clipped_ = false; }

    int w = Scintilla::RoundXYPosition(rc.right - rc.left);

    int h = Scintilla::RoundXYPosition(rc.bottom - rc.top);

    if (w <= 0 || h <= 0) return;

    fl_push_clip(Scintilla::RoundXYPosition(rc.left) + ox(),

                 Scintilla::RoundXYPosition(rc.top) + oy(), w, h);

    clipped_ = true;

}

void SurfaceFLTK::Release()

{

    if (clipped_) { fl_pop_clip(); clipped_ = false; }

    if (offscreen_buffer != 0) {

        fl_delete_offscreen(offscreen_buffer);

        offscreen_buffer = 0;

    }

    scibase_ = 0; win_ = 0; drawtype_ = 0; swt_ = 0;

    pixmap_init_ = false;

}

void SurfaceFLTK::FlushCachedState()

{

}

void SurfaceFLTK::SetUnicodeMode(bool unicodeMode_)

{

    unicodeMode = unicodeMode_;

}

void SurfaceFLTK::SetDBCSMode(int codePage_)

{

    codePage = codePage_;

}

Scintilla::Surface *Scintilla::Surface::Allocate(int technology)

{

    return new SurfaceFLTK();

}

// ====================== Platform::Menu ===================================

Scintilla::Menu::Menu() : mid(0) 

{

}

void Scintilla::Menu::CreatePopUp() 

{

    Destroy();

    Fl_Menu_Button *m;

    m = new Fl_Menu_Button(0, 0, 0, 0);

    m->type(Fl_Menu_Button::POPUP3);

    mid = m;

}

void Scintilla::Menu::Destroy() 

{

    if (mid) delete (Fl_Menu_Button*)mid;

    mid = 0;

}

void Scintilla::Menu::Show(Point pt, Scintilla::Window &w) 

{

    Fl_Menu_Button *m = (Fl_Menu_Button *)mid;

    if (m) m->popup();

    Destroy();

}

// ====================== Platform::ElapsedTime ===================================

Scintilla::ElapsedTime::ElapsedTime() 

{

    bigBit = clock();

}

double Scintilla::ElapsedTime::Duration(bool reset) 

{

    double result;

    long endBigBit;

    long endLittleBit;

    endBigBit = clock();

    endLittleBit = 0;

    double elapsed = endBigBit - bigBit;

    result = elapsed / CLOCKS_PER_SEC;

    if (reset) {

        bigBit = endBigBit;

        littleBit = endLittleBit;

    }

    return result;

}

// ====================== Platform::DynamicLibrary ok ===================================

#if defined(_WIN32) && !defined(__EMSCRIPTEN__)

#include <windows.h>

#else

#include <dlfcn.h>

#endif

class DynamicLibraryImpl : public Scintilla::DynamicLibrary {

protected:

#if defined(_WIN32) && !defined(__EMSCRIPTEN__)

    HMODULE h;

#else

    void * h;

#endif

public:

    explicit DynamicLibraryImpl(const char *modulePath) {

#if defined(_WIN32) && !defined(__EMSCRIPTEN__)

        h = ::LoadLibraryA(modulePath);

#else

        h = dlopen(modulePath, RTLD_LAZY);

#endif

    }

    virtual ~DynamicLibraryImpl() {

#if defined(_WIN32) && !defined(__EMSCRIPTEN__)

        if (h != NULL) ::FreeLibrary(h);

#else

        if ( h != NULL ) dlclose(h);

#endif

    }

    virtual Scintilla::Function FindFunction(const char *name) {

#if defined(_WIN32) && !defined(__EMSCRIPTEN__)

        if (h != NULL) {

            union {

                FARPROC fp;

                Scintilla::Function f;

            } fnConv;

            fnConv.fp = ::GetProcAddress(h, name);

            return fnConv.f;

        } else {

            return NULL;

        }

#else

        if (h != NULL) {

            union {

                void *fp;

                Scintilla::Function f;

            } fnConv;

            fnConv.fp = dlsym(h, name);

            return fnConv.f;

        }

        return NULL;

#endif      

    }

    virtual bool IsValid() {

        return h != NULL;

    }

};

Scintilla::DynamicLibrary *Scintilla::DynamicLibrary::Load(const char *modulePath) 

{

    return static_cast<DynamicLibrary *>(new DynamicLibraryImpl(modulePath));

}

// ====================== Platform::ListBox ===================================

struct ListItemData {

    const char *text;

    int pixId;

};

class LineToItem {

    std::vector<char> words;

    std::vector<ListItemData> data;

public:

    LineToItem() {

    }

    ~LineToItem() {

        Clear();

    }

    void Clear() {

        words.clear();

        data.clear();

    }

    ListItemData Get(int index) const {

        if (index >= 0 && index < static_cast<int>(data.size())) {

            return data[index];

        } else {

            ListItemData missing = {"", -1};

            return missing;

        }

    }

    int Count() const {

        return static_cast<int>(data.size());

    }

    void AllocItem(const char *text, int pixId) {

        ListItemData lid = { text, pixId };

        data.push_back(lid);

    }

    char *SetWords(const char *s) {

        words = std::vector<char>(s, s+strlen(s)+1);

        return &words[0];

    }

};

class ListboxWindow : public Fl_Double_Window {

public:

    ListboxWindow(int X, int Y, int W, int H, const char* l = 0) : Fl_Double_Window(X, Y, W, H, l)

    {

        browser_ = new Fl_Hold_Browser(2, 2, W-4, H-4, "");

        browser_->box(FL_FLAT_BOX);

        browser_->clear_visible_focus();

        resizable(browser_);

        end();

        m_borderesize_status = 0;

    }

    Fl_Hold_Browser *GetBrowser()

    {

        return browser_;

    }

protected:

    unsigned int m_borderesize_status;

    Fl_Hold_Browser *browser_;

    int handle(int e)

    {

        if ( FL_DRAG == e ) {

            handle_drag();

            return 1;

        }

        if ( FL_MOVE == e ) {

            handle_move();

            return 1;

        }

        if ( FL_PUSH == e ) {

            handle_push(e);

            return 1;

        }

        if ( FL_RELEASE == e ) {

            handle_release(e);

            return 1;

        }

        return Fl_Double_Window::handle(e);

    }

    void handle_push(int e)

    {

        int len=20, split=2;

        if ( Fl::event_inside(0, len, split, h()-len*2) ) {

        } else if ( Fl::event_inside(w()-split, len, split, h()-len*2) ) {

        } else if ( Fl::event_inside(len, 0, w()-len*2, split) ) {

        } else if ( Fl::event_inside(len, h()-split, w()-len*2, split) ) {

        } else if ( Fl::event_inside(0, 0, len, split) || Fl::event_inside(0, 0, split, len) ) {

        } else if ( Fl::event_inside(0, h()-split, len, split) || Fl::event_inside(0, h()-len, split, len) ) {

        } else if ( Fl::event_inside(w()-len, 0, len, split) || Fl::event_inside(w()-split, 0, split, len) ) {

        } else if ( Fl::event_inside(w()-len, h()-split, len, split) || Fl::event_inside(w()-split, h()-len, split, len) ) {

        } else {

            browser_->handle(e);

        }

    }

    void handle_release(int e)

    {

        int len=20, split=2;

        if ( Fl::event_inside(0, len, split, h()-len*2) ) {

        } else if ( Fl::event_inside(w()-split, len, split, h()-len*2) ) {

        } else if ( Fl::event_inside(len, 0, w()-len*2, split) ) {

        } else if ( Fl::event_inside(len, h()-split, w()-len*2, split) ) {

        } else if ( Fl::event_inside(0, 0, len, split) || Fl::event_inside(0, 0, split, len) ) {

        } else if ( Fl::event_inside(0, h()-split, len, split) || Fl::event_inside(0, h()-len, split, len) ) {

        } else if ( Fl::event_inside(w()-len, 0, len, split) || Fl::event_inside(w()-split, 0, split, len) ) {

        } else if ( Fl::event_inside(w()-len, h()-split, len, split) || Fl::event_inside(w()-split, h()-len, split, len) ) {

        } else {

            browser_->handle(e);

        }

    }

    void handle_move()

    {

        int len=20, split=2;

        m_borderesize_status = 0;

        if ( Fl::event_inside(0, len, split, h()-len*2) ) {

            fl_cursor(FL_CURSOR_WE);

            m_borderesize_status = 1;

        } else if ( Fl::event_inside(w()-split, len, split, h()-len*2) ) {

            fl_cursor(FL_CURSOR_WE);

            m_borderesize_status = 2;

        } else if ( Fl::event_inside(len, 0, w()-len*2, split) ) {

            fl_cursor(FL_CURSOR_NS);

            m_borderesize_status = 3;

        } else if ( Fl::event_inside(len, h()-split, w()-len*2, split) ) {

            fl_cursor(FL_CURSOR_NS);

            m_borderesize_status = 4;

        } else if ( Fl::event_inside(0, 0, len, split) || Fl::event_inside(0, 0, split, len) ) {

            fl_cursor(FL_CURSOR_NWSE);

            m_borderesize_status = 13;

        } else if ( Fl::event_inside(0, h()-split, len, split) || Fl::event_inside(0, h()-len, split, len) ) {

            fl_cursor(FL_CURSOR_NESW);

            m_borderesize_status = 14;

        } else if ( Fl::event_inside(w()-len, 0, len, split) || Fl::event_inside(w()-split, 0, split, len) ) {

            fl_cursor(FL_CURSOR_NESW);

            m_borderesize_status = 23;

        } else if ( Fl::event_inside(w()-len, h()-split, len, split) || Fl::event_inside(w()-split, h()-len, split, len) ) {

            fl_cursor(FL_CURSOR_NWSE);

            m_borderesize_status = 24;

        } else {

            fl_cursor(FL_CURSOR_DEFAULT);

        }

    }

    void handle_drag()

    {

        int x1, y1;

        int x = Fl::event_x();

        int y = Fl::event_y();

        if ( 1 == m_borderesize_status ) {

            if (w()-x>80) this->resize(this->x()+x, this->y(), this->w()- x, this->h());

        } else if ( 2 == m_borderesize_status ) {

            if (x>80) this->resize(this->x(), this->y(), x, this->h());

        } else if ( 3 == m_borderesize_status ) {

            if (this->h()-y>40) this->resize(this->x(), this->y()+y, this->w(), this->h()-y);

        } else if ( 4 == m_borderesize_status ) {

            if (y>40) this->resize(this->x(), this->y(), this->w(), y);

        } else if ( 13 == m_borderesize_status ) {

            x1=x; if (w()-x<=80) x1=0;

            y1=y; if (this->h()-y<=40) y1=0;

            this->resize(this->x()+x1, this->y()+y1, this->w()-x1, this->h()-y1);

        } else if ( 14 == m_borderesize_status ) {

            x1=x; if (w()-x<=80) x1=0;

            y1=y; if (y<=40) y1=h();

            this->resize(this->x()+x1, this->y(), this->w()-x1, y1);

        } else if ( 23 == m_borderesize_status ) {

            x1=x; if (x<=80) x1=w();

            y1=y; if (this->h()-y<=40) y1=0;

            this->resize(this->x(), this->y()+y1, x1,this->h()-y1);

        } else if ( 24 == m_borderesize_status ) {

            x1=x; if (x<=80) x1=w();

            y1=y; if (y<=40) y1=h();

            this->resize(this->x(), this->y(), x1,y1);

        }

    }

};

Scintilla::ListBox::ListBox() {

}

Scintilla::ListBox::~ListBox() {

}

class ListBoxX : public Scintilla::ListBox {

    Fl_Hold_Browser *brw_;

    SCIWinType swt_;

    int lineHeight;

    Scintilla::RGBAImageSet images;

    LineToItem lti;

    bool unicodeMode;

    int desiredVisibleRows;

    unsigned int maxItemCharacters;

    unsigned int aveCharWidth;

    Scintilla::Window *parent;

    int ctrlID;

    Scintilla::CallBackAction doubleClickAction;

    void *doubleClickActionData;

    int widestItemindex;

    unsigned int maxCharWidth;

    int resizeHit;

    Scintilla::PRectangle rcPreSize;

    Scintilla::Point dragOffset;

    Scintilla::Point location;

    int wheelDelta;

    void AppendListItem(const char *text, const char *numword);

    static void AdjustWindowRect(Scintilla::PRectangle *rc);

    int ItemHeight() const;

    int MinClientWidth() const;

    int TextOffset() const;

    void OnDoubleClick();

    static const Scintilla::Point ItemInset;

    static const Scintilla::Point TextInset;

    static const Scintilla::Point ImageInset;

public:

    ListBoxX() : lineHeight(10), unicodeMode(false),

        desiredVisibleRows(5), maxItemCharacters(0), aveCharWidth(8),

        parent(NULL), ctrlID(0), doubleClickAction(NULL), doubleClickActionData(NULL),

        widestItemindex(0), maxCharWidth(1), resizeHit(0), wheelDelta(0) 

    {

    }

    virtual ~ListBoxX() 

    {

    }

    virtual void SetFont(Scintilla::Font &font);

    virtual void Create(Scintilla::Window &parent_, int ctrlID_, Scintilla::Point location_, int lineHeight_, bool unicodeMode_, int technology_);

    virtual void SetAverageCharWidth(int width);

    virtual void SetVisibleRows(int rows);

    virtual int GetVisibleRows() const;

    virtual Scintilla::PRectangle GetDesiredRect();

    virtual int CaretFromEdge();

    virtual void Clear();

    virtual void Append(char *s, int type = -1);

    virtual int Length();

    virtual void Select(int n);

    virtual int GetSelection();

    virtual int Find(const char *prefix);

    virtual void GetValue(int n, char *value, int len);

    virtual void RegisterImage(int type, const char *xpm_data);

    virtual void RegisterRGBAImage(int type, int width, int height, const unsigned char *pixelsImage);

    virtual void ClearRegisteredImages();

    virtual void SetDoubleClickAction(Scintilla::CallBackAction action, void *data) {

        doubleClickAction = action;

        doubleClickActionData = data;

    }

    virtual void SetList(const char *list, char separator, char typesep);

protected:

    static void cb_brw(Fl_Widget *w, void *x)

    {

        ListBoxX *lsb = (ListBoxX *)x;

        if ( Fl::event_clicks() == 1 ) lsb->OnDoubleClick();

    }

};

const Scintilla::Point ListBoxX::ItemInset(0, 0);

const Scintilla::Point ListBoxX::TextInset(2, 0);

const Scintilla::Point ListBoxX::ImageInset(1, 0);

Scintilla::ListBox *Scintilla::ListBox::Allocate() 

{

    ListBoxX *lb = new ListBoxX();

    return lb;

}

void ListBoxX::Create(Scintilla::Window &parent_, int ctrlID_, Scintilla::Point location_, int lineHeight_, bool unicodeMode_, int technology_) 

{

    parent = &parent_;

    ctrlID = ctrlID_;

    location = location_;

    lineHeight = lineHeight_;

    unicodeMode = unicodeMode_;

    int wx=Scintilla::RoundXYPosition(location_.x), wy=Scintilla::RoundXYPosition(location_.y) + lineHeight_;

    SCIWinType *swt = (SCIWinType *)parent->GetID();

    Fl_Group *wd = (Fl_Group *)swt->wid;

    wx += wd->x(); wy += wd->y();

    Fl_Window* w = NULL;

    for (w = wd->window(); w; w = w->window()) {

        wx += w->x();

        wy += w->y();

    }

    Fl_Group::current(0);

    ListboxWindow *win = new ListboxWindow(0, 0, 150, 80);

    win->clear_border();

    win->set_tooltip_window();

    brw_ = win->GetBrowser();

    brw_->callback(cb_brw, this);

    swt_.type = 1;

    swt_.wid = win;

    swt_.main_client.x = wx;

    swt_.main_client.y = wy;

    wid = &swt_;

}

void ListBoxX::SetFont(Scintilla::Font &font) 

{

    FontData* fd = (FontData*)(font.GetID());

    if (!fd) return;

    Fl_Window *win = (Fl_Window*)swt_.wid;

    win->labelfont(fd->ff_);

    win->labelsize(GetFlSize(fd->size_));

}

void ListBoxX::SetAverageCharWidth(int width) 

{

    aveCharWidth = width;

}

void ListBoxX::SetVisibleRows(int rows) 

{

    desiredVisibleRows = rows;

}

int ListBoxX::GetVisibleRows() const 

{

    return desiredVisibleRows;

}

Scintilla::PRectangle ListBoxX::GetDesiredRect() 

{

    Scintilla::PRectangle rcDesired = GetPosition();

    int rows = Length();

    if ((rows == 0) || (rows > desiredVisibleRows)) rows = desiredVisibleRows;

    rcDesired.bottom = rcDesired.top + ItemHeight() * rows;

    int width = MinClientWidth();

    int len = 0;

    const char *s;

    if (widestItemindex > 0) {

        s = brw_->text(widestItemindex);

        len = (int)(strlen(s));

        width = (int)fl_width(s);

    }

    int widthDesired = Scintilla::Platform::Maximum(width, (len + 1) * 10);

    if (width < widthDesired) width = widthDesired;

    rcDesired.right = rcDesired.left + TextOffset() + width + (TextInset.x * 2);

    if (Length() > rows) rcDesired.right += brw_->scrollbar_size();

    AdjustWindowRect(&rcDesired);

    return rcDesired;

}

int ListBoxX::TextOffset() const 

{

    int pixWidth = images.GetWidth();

    return static_cast<int>(pixWidth == 0 ? ItemInset.x : ItemInset.x + pixWidth + (ImageInset.x * 2));

}

int ListBoxX::CaretFromEdge() 

{

    Scintilla::PRectangle rc;

    AdjustWindowRect(&rc);

    return TextOffset() + static_cast<int>(TextInset.x + (0 - rc.left) - 1);

}

void ListBoxX::Clear() 

{

    maxItemCharacters = 0;

    widestItemindex = 0;

    lti.Clear();

    int i, n=brw_->size();

    for (i=0; i<n; i++) {

        Fl_Image *img = brw_->icon(i+1);

        brw_->icon(i+1, 0);

        if ( img ) delete img;

    }

    brw_->clear();

}

void ListBoxX::Append(char *, int) 

{

    PLATFORM_ASSERT(false);

}

int ListBoxX::Length() 

{

    return lti.Count();

}

void ListBoxX::Select(int n) 

{

    brw_->value(n+1);

}

int ListBoxX::GetSelection() 

{

    return brw_->value()-1;

}

int ListBoxX::Find(const char *) 

{

    return -1;

}

void ListBoxX::GetValue(int n, char *value, int len) 

{

    ListItemData item = lti.Get(n);

    strncpy(value, item.text, len);

    value[len-1] = '\0';

}

void ListBoxX::RegisterImage(int type, const char *xpm_data) 

{

    Scintilla::XPM xpmImage(xpm_data);

    images.Add(type, new Scintilla::RGBAImage(xpmImage));

}

void ListBoxX::RegisterRGBAImage(int type, int width, int height, const unsigned char *pixelsImage) 

{

    images.Add(type, new Scintilla::RGBAImage(width, height, 1.0, pixelsImage));

}

void ListBoxX::ClearRegisteredImages() 

{

    images.Clear();

}

void ListBoxX::AppendListItem(const char *text, const char *numword) {

    int pixId = -1;

    if (numword) {

        pixId = 0;

        char ch;

        while ((ch = *++numword) != '\0') {

            pixId = 10 * pixId + (ch - '0');

        }

    }

    lti.AllocItem(text, pixId);

    unsigned int len = static_cast<unsigned int>(strlen(text));

    if (maxItemCharacters < len) {

        maxItemCharacters = len;

        widestItemindex = lti.Count();

    }

}

void ListBoxX::SetList(const char *list, char separator, char typesep) 

{

    Clear();

    size_t size = strlen(list);

    char *words = lti.SetWords(list);

    char *startword = words;

    char *numword = NULL;

    Fl_RGB_Image *fti;

    for (size_t i=0; i < size; i++) {

        if (words[i] == separator) {

            words[i] = '\0';

            if (numword)

                *numword = '\0';

            AppendListItem(startword, numword);

            brw_->add(startword);

            ListItemData item = lti.Get(brw_->size()-1);

            int pixId = item.pixId;

            Scintilla::RGBAImage *pimage = images.Get(pixId);

            if (pimage) {

                fti = new Fl_RGB_Image(pimage->Pixels(), pimage->GetWidth(), pimage->GetHeight(), 4);

                brw_->icon(brw_->size(), fti);

            }

            startword = words + i + 1;

            numword = NULL;

        } else if (words[i] == typesep) {

            numword = words + i;

        }

    }

    if (startword) {

        if (numword) *numword = '\0';

        AppendListItem(startword, numword);

        brw_->add(startword);

        ListItemData item = lti.Get(brw_->size()-1);

        int pixId = item.pixId;

        Scintilla::RGBAImage *pimage = images.Get(pixId);

        if (pimage) {

            fti = new Fl_RGB_Image(pimage->Pixels(), pimage->GetWidth(), pimage->GetHeight(), 4);

            brw_->icon(brw_->size(), fti);

        }

    }

    if ( brw_->size() > 0 ) brw_->select(1);

}

void ListBoxX::AdjustWindowRect(Scintilla::PRectangle *rc) 

{

    Scintilla::XYPOSITION left = rc->left;

    Scintilla::XYPOSITION right = rc->right;

    Scintilla::XYPOSITION top = rc->top;

    Scintilla::XYPOSITION bottom = rc->bottom;

    *rc = Scintilla::PRectangle(left-2, top-2, right+2, bottom+2);

}

int ListBoxX::ItemHeight() const 

{

    int itemHeight = lineHeight + (static_cast<int>(TextInset.y) * 2);

    int pixHeight = images.GetHeight() + (static_cast<int>(ImageInset.y) * 2);

    if (itemHeight < pixHeight) {

        itemHeight = pixHeight;

    }

    return itemHeight;

}

int ListBoxX::MinClientWidth() const 

{

    return 12 * (aveCharWidth+aveCharWidth/3);

}

void ListBoxX::OnDoubleClick() {

    if (doubleClickAction != NULL) {

        doubleClickAction(doubleClickActionData);

    }

}

// ====================== Platform::Window ===================================

Scintilla::Window::~Window() {}

void Scintilla::Window::Destroy() 

{   

    if ( wid ) {        

        SCIWinType *swt = (SCIWinType *)wid;

        if ( swt->type == 0 ) {

        } else if ( swt->type == 1 ) {

            Fl_Window *w = (Fl_Window*)swt->wid;

            w->hide();

        } else if ( swt->type == 2 ) {

            Fl_Window *w = (Fl_Window*)swt->wid;

            w->hide();

        }       

    }

    wid = 0; 

}

bool Scintilla::Window::HasFocus()

{

    SCIWinType *swt = (SCIWinType *)wid;

    if (!swt) return false;

    return Fl::focus() == (Fl_Widget *)swt->wid;

}

Scintilla::PRectangle Scintilla::Window::GetPosition()

{

    SCIWinType *swt = (SCIWinType *)wid;

    Fl_Widget *w = (Fl_Widget *)swt->wid;

    return PRectangle::FromInts(w->x(), w->y(), w->x() + w->w(), w->y() + w->h());

}

void Scintilla::Window::SetPosition(PRectangle rc)

{   

    if ( ! wid ) return;

    int x = RoundXYPosition(rc.left);

    int y = RoundXYPosition(rc.top);

    int w = RoundXYPosition(rc.Width());

    int h = RoundXYPosition(rc.Height());

    SCIWinType *swt = (SCIWinType *)wid;

    if ( swt->type == 0 ) {

        Fl_Group *g = (Fl_Group*)swt->wid;

        g->resize(x, y, w, h);

    } else {

        Fl_Window *win = (Fl_Window *)swt->wid;

        win->resize(x, y, w, h);

    }

}

void Scintilla::Window::SetPositionRelative(PRectangle rc, Scintilla::Window win)

{

    if ( ! wid ) return;

    int x = RoundXYPosition(rc.left);

    int y = RoundXYPosition(rc.top);

    int w = RoundXYPosition(rc.Width());

    int h = RoundXYPosition(rc.Height());

    SCIWinType *swt = (SCIWinType *)wid;

    if ( swt->type == 0 ) {

        x += swt->rc_client.x;

        y += swt->rc_client.y;

        SetPosition(PRectangle::FromInts(x, y, x+w, y+h));

    } else if ( swt->type == 1 ) {

        x = swt->main_client.x;

        y = swt->main_client.y;

        SetPosition(PRectangle::FromInts(x, y, x+w, y+h));

    } else {

        x += swt->main_client.x;

        y += swt->main_client.y;

        SetPosition(PRectangle::FromInts(x, y, x+w, y+h));

    }

}

Scintilla::PRectangle Scintilla::Window::GetClientPosition()

{

    if ( wid ) {

        SCIWinType *swt = (SCIWinType *)wid;

        if ( swt->type == 0 ) {

            return Scintilla::PRectangle::FromInts(0, 0, swt->rc_client.w, swt->rc_client.h);

        }

        Fl_Window *win = (Fl_Window *)swt->wid;

        return Scintilla::PRectangle::FromInts(0, 0, win->w(), win->h());

    } else {

        return Scintilla::PRectangle::FromInts(0, 0, 0, 0);

    }

}

void Scintilla::Window::Show(bool show)

{

    if ( ! wid ) return;

    SCIWinType *swt = (SCIWinType *)wid;

    if ( swt->type == 0 ) {

        Fl_Group *w = (Fl_Group *)swt->wid;

        if (show ) {

            w->show();

        } else {

            w->hide();

        }

    } else {

        Fl_Window *win = (Fl_Window *)swt->wid;

        if (show ) {

            win->show();

        } else {

            win->hide();

        }

    }

}

void Scintilla::Window::InvalidateAll()

{

    if ( ! wid ) return;

    SCIWinType *swt = (SCIWinType *)wid;

    if ( swt->type == 0 ) {

        Fl_Group *w = (Fl_Group *)swt->wid;

        w->damage(FL_DAMAGE_ALL, swt->rc_client.x, swt->rc_client.y, swt->rc_client.w, swt->rc_client.h);

    } else {

        Fl_Window *win = (Fl_Window *)swt->wid;

        win->damage(FL_DAMAGE_ALL, 0, 0, win->w(), win->h());

    }

}

void Scintilla::Window::InvalidateRectangle(PRectangle rc)

{

    if ( ! wid ) return;

    int x, y, w, h;

    SCIWinType *swt = (SCIWinType *)wid;

    if ( swt->type == 0 ) {

        Fl_Group *sci = (Fl_Group *)swt->wid;

        x = RoundXYPosition(rc.left) + swt->rc_client.x;

        y = RoundXYPosition(rc.top) + swt->rc_client.y;

        w = RoundXYPosition(rc.Width());

        h = RoundXYPosition(rc.Height());

        sci->damage(FL_DAMAGE_ALL, x, y, w, h);

    } else {

        Fl_Window *win = (Fl_Window *)swt->wid;

        x = RoundXYPosition(rc.left);

        y = RoundXYPosition(rc.top);

        w = RoundXYPosition(rc.Width());

        h = RoundXYPosition(rc.Height());

        win->damage(FL_DAMAGE_ALL, x, y, w, h);

    }

}

void Scintilla::Window::SetFont(Font &font)

{

    if ( ! wid ) return;

    FontData* fd = (FontData*)(font.GetID());

    if (!fd) return;

    SCIWinType *swt = (SCIWinType *)wid;

    if ( swt->type == 0 ) {

        Fl_Group *sci = (Fl_Group *)swt->wid;

        sci->labelfont(fd->ff_);

        sci->labelsize(GetFlSize(fd->size_));

    } else {

        Fl_Window *win = (Fl_Window *)swt->wid;

        win->labelfont(fd->ff_);

        win->labelsize(GetFlSize(fd->size_));

    }

}

void Scintilla::Window::SetCursor(Cursor curs)

{

    if ( ! wid ) return;

    Fl_Cursor cur = FL_CURSOR_ARROW;

    switch (curs) {

    case cursorText:

        cur = FL_CURSOR_INSERT;

        break;

    case cursorUp:

        cur = FL_CURSOR_N;

        break;

    case cursorWait:

        cur = FL_CURSOR_WAIT;

        break;

    case cursorHoriz:

        cur = FL_CURSOR_WE;

        break;

    case cursorVert:

        cur = FL_CURSOR_NS;

        break;

    case cursorHand:

        cur = FL_CURSOR_HAND;

        break;

    case cursorReverseArrow:

        cur = FL_CURSOR_ARROW;

        break;

    case cursorArrow:

    case cursorInvalid:

        cur = FL_CURSOR_ARROW;

        break;

    }

    SCIWinType *swt = (SCIWinType *)wid;

    if ( swt->type == 0 ) {

        Fl_Group *sci = (Fl_Group *)swt->wid;

        if (sci->window()) sci->window()->cursor(cur);

    } else {

        Fl_Window *win = (Fl_Window *)swt->wid;

        win->cursor(cur);

    }

}

void Scintilla::Window::SetTitle(const char *s)

{

    SCIWinType *swt = (SCIWinType *)wid;

    Fl_Group *w = (Fl_Group *)swt->wid;

    w->copy_label(s);   

}

Scintilla::PRectangle Scintilla::Window::GetMonitorRect(Scintilla::Point pt)

{

    return Scintilla::PRectangle();

}

// ====================== Platform ===================================

Scintilla::ColourDesired Scintilla::Platform::Chrome() // ok

{

    unsigned char r,g,b;

    Fl_Color fc = Fl::get_color(FL_BACKGROUND_COLOR);

    Fl::get_color(fc, r, g, b);

    return ColourDesired(r, g, b);

}

Scintilla::ColourDesired Scintilla::Platform::ChromeHighlight() // ok

{

    unsigned char r,g,b;

    Fl_Color fc = Fl::get_color(FL_BACKGROUND2_COLOR);

    Fl::get_color(fc, r, g, b);

    return ColourDesired(r, g, b);

}

const char *Scintilla::Platform::DefaultFont() // ok

{

    return Fl::get_font_name(FL_HELVETICA);

}

int Scintilla::Platform::DefaultFontSize() // ok

{

    return 10;

}

unsigned int Scintilla::Platform::DoubleClickTime() // ok

{

    return 500;

}

bool Scintilla::Platform::MouseButtonBounce() // ok

{

    return false;

}

void Scintilla::Platform::DebugDisplay(const char *s)

{

    printf("%s\n", s);

}

bool Scintilla::Platform::IsKeyDown(int key)

{

    if ( Fl::event_state(key) == 0 ) return false;

    return true;

}

long Scintilla::Platform::SendScintilla(WindowID w, unsigned int msg, unsigned long wParam, long lParam)

{

    return 0;

}

long Scintilla::Platform::SendScintillaPointer(WindowID w, unsigned int msg, unsigned long wParam, void *lParam)

{

    return 0;

}

bool Scintilla::Platform::IsDBCSLeadByte(int codePage, char ch)

{

    unsigned char uch = static_cast<unsigned char>(ch);

    switch (codePage) {

    case 932:

        return ((uch >= 0x81) && (uch <= 0x9F)) || ((uch >= 0xE0) && (uch <= 0xEF));

    case 936:

        return (uch >= 0x81) && (uch <= 0xFE);

    case 949:

        return (uch >= 0x81) && (uch <= 0xFE);

    case 950:

        return (uch >= 0x81) && (uch <= 0xFE);

    case 1361:

        return

        ((uch >= 0x84) && (uch <= 0xD3)) ||

        ((uch >= 0xD8) && (uch <= 0xDE)) ||

        ((uch >= 0xE0) && (uch <= 0xF9));

    }

    return false;

}

int Scintilla::Platform::DBCSCharLength(int codePage, const char *s)

{

    if (codePage == 932 || codePage == 936 || codePage == 949 || codePage == 950 || codePage == 1361) {

        return Platform::IsDBCSLeadByte(codePage, s[0]) ? 2 : 1;

    } else {

        return 1;

    }

}

int Scintilla::Platform::DBCSCharMaxLength()

{

    return 2;

}

int Scintilla::Platform::Minimum(int a, int b)

{

    if (a < b) return a;

    else return b;

}

int Scintilla::Platform::Maximum(int a, int b)

{

    if (a > b) return a;

    else return b;

}

#ifdef TRACE

void Scintilla::Platform::DebugPrintf(const char *format, ...)

{

    char buffer[2000];

    va_list pArguments;

    va_start(pArguments, format);

    vsprintf(buffer,format,pArguments);

    va_end(pArguments);

    Scintilla::Platform::DebugDisplay(buffer);

}

#else

void Scintilla::Platform::DebugPrintf(const char *, ...)

{

}

#endif

static bool assertionPopUps = true;

bool Scintilla::Platform::ShowAssertionPopUps(bool assertionPopUps_)

{

    bool ret = assertionPopUps;

    assertionPopUps = assertionPopUps_;

    return ret;

}

void Scintilla::Platform::Assert(const char *c, const char *file, int line)

{

    char buffer[2000];

    sprintf(buffer, "Assertion [%s] failed at %s %d%s", c, file, line, assertionPopUps ? "" : "\r\n");

    if (assertionPopUps) {

        int hotspot = fl_message_hotspot();

        fl_message_hotspot(0);

        fl_message_title("Assertion failure");

        int rep = fl_choice(buffer, "Retry", "Ignore", "Quit");

        fl_message_hotspot(hotspot);

        if (rep==0) {

        } else if (rep==1) {

        } else {

            abort();

        }

    } else {

        Scintilla::Platform::DebugDisplay(buffer);

        abort();

    }

}

int Scintilla::Platform::Clamp(int val, int minVal, int maxVal)

{

    if (val > maxVal) val = maxVal;

    if (val < minVal) val = minVal;

    return val;

}

// ====================== Interface ===================================

void Platform_Initialise()

{

}

void Platform_Finalise()

{

}

// ---- Fl_Scintilla.cxx ----
// Copyright 2015-2016 by cyantree <cyantree.guo@gmail.com>

/*

bug:

ok 1.mac pinyin type

ok 2.scroll

ok 3.timer, not test

ok 4.check virtual function lost

ok 5.get string and move mouse to overside

6.iconv cannot compile 64bit

7.win10 wm_touch

8.high dpi

*/

Fl_Scintilla::Fl_Scintilla(int X, int Y, int W, int H, const char* l) : Fl_Group(X, Y, W, H, l)

{

    color(FL_BACKGROUND2_COLOR, FL_SELECTION_COLOR);

    box(FL_DOWN_FRAME);

    swt_.rc_client.x = X+Fl::box_dx(box());

    swt_.rc_client.y = Y+Fl::box_dy(box());

    swt_.rc_client.w = W-Fl::box_dw(box());

    swt_.rc_client.h = H-Fl::box_dh(box());

    swt_.type = 0;

    swt_.wid = this;

    mVScrollBar = new Fl_Scrollbar(0,0,1,1);

    mVScrollBar->linesize(1);

    mVScrollBar->callback((Fl_Callback*)v_scrollbar_cb, this);

    mHScrollBar = new Fl_Scrollbar(0,0,1,1);

    mHScrollBar->linesize(20);

    mHScrollBar->callback((Fl_Callback*)h_scrollbar_cb, this);

    mHScrollBar->type(FL_HORIZONTAL);

    scrollpush_ = 0;

    end();

    scrollbar_width(Fl::scrollbar_size());

    //

    cb_notify_.data = NULL;

    cb_notify_.callback = NULL;

    //

    Platform_Initialise();

#ifdef SCI_LEXER 

    Scintilla_LinkLexers();

#endif

    lastKeyDownConsumed = false;

    capturedMouse = false;

    LButtonDown = false;

    wMain = &swt_;

    // printf("fl_scintilla: %x\n", this);

    linesPerScroll = 3;

    wheelDelta = 0;   // Wheel delta from roll

    sysCaretWidth = 0;

    sysCaretHeight = 0;

    caret.period = 530;

    if (caret.period < 0) caret.period = 0;

    Initialise();

    sw_ = Scintilla::Surface::Allocate(0);

    ic_ = new ICONVConverter();

    ic_str_ = NULL;

    ic_str_len_ = 0;

    drag_str_ = NULL;

    drag_str_size_ = 0;

    // Emscripten is always UTF-8; force the document into Unicode mode.

    SendEditor(SCI_SETCODEPAGE, SC_CP_UTF8, 0);

#ifdef __EMSCRIPTEN__
    // Buffered drawing renders each line into an Fl_Offscreen and blits it.
    // On this backend that offscreen permanently replaces the window canvas
    // (see SurfaceFLTK::InitPixMap), so the blit never hits the screen.
    SendEditor(SCI_SETBUFFEREDDRAW, 0, 0);
#endif

}

Fl_Scintilla::~Fl_Scintilla()

{

    sw_->Release();

    delete sw_;

    delete ic_;

    if ( ic_str_len_ > 0 ) free(ic_str_);

    if ( drag_str_ != NULL ) free(drag_str_);

    Platform_Finalise();

    Finalise();

}

#define SC_INDICATOR_INPUT INDIC_IME

#define SC_INDICATOR_TARGET INDIC_IME+1

#define SC_INDICATOR_CONVERTED INDIC_IME+2

#define SC_INDICATOR_UNKNOWN INDIC_IME_MAX

void Fl_Scintilla::Initialise()

{

    for (TickReason tr = tickCaret; tr <= tickDwell; tr = static_cast<TickReason>(tr + 1)) {

        timeractive_[tr] = 0;

    }

    vs.indicators[SC_INDICATOR_UNKNOWN] = Scintilla::Indicator(INDIC_HIDDEN, Scintilla::ColourDesired(0, 0, 0xff));

    vs.indicators[SC_INDICATOR_INPUT] = Scintilla::Indicator(INDIC_DOTS, Scintilla::ColourDesired(0, 0, 0xff));

    vs.indicators[SC_INDICATOR_CONVERTED] = Scintilla::Indicator(INDIC_COMPOSITIONTHICK, Scintilla::ColourDesired(0, 0, 0xff));

    vs.indicators[SC_INDICATOR_TARGET] = Scintilla::Indicator(INDIC_STRAIGHTBOX, Scintilla::ColourDesired(0, 0, 0xff));

}

void Fl_Scintilla::Finalise()

{

    ScintillaBase::Finalise();

    for (TickReason tr = tickCaret; tr <= tickDwell; tr = static_cast<TickReason>(tr + 1)) {

        FineTickerCancel(tr);

    }

    SetIdle(false);

}

void Fl_Scintilla::NotifyParent(Scintilla::SCNotification scn)

{

    if ( cb_notify_.callback == NULL ) return;

    cb_notify_.callback(&scn, cb_notify_.data);

}

void Fl_Scintilla::resize(int X, int Y, int W, int H)

{

    //printf("resize, v visiable=%d, h visiable=%d\n", mVScrollBar->visible(), mHScrollBar->visible());

    swt_.rc_client.x = X+Fl::box_dx(box());

    swt_.rc_client.y = Y+Fl::box_dy(box());

    swt_.rc_client.w = W-Fl::box_dw(box());

    swt_.rc_client.h = H-Fl::box_dh(box());

    Fl_Group::resize(X,Y,W,H);

    ChangeSize();

}

/** Map the key codes to their equivalent SCK_ form. */

static int KeyTranslate(int keyIn)

{

    switch (keyIn) {

    case FL_Down:

        return SCK_DOWN;

    case FL_Up:

        return SCK_UP;

    case FL_Left:

        return SCK_LEFT;

    case FL_Right:

        return SCK_RIGHT;

    case FL_Home:

        return SCK_HOME;

    case FL_End:

        return SCK_END;

    case FL_Page_Up:

        return SCK_PRIOR;

    case FL_Page_Down:

        return SCK_NEXT;

    case FL_Delete:

        return SCK_DELETE;

    case FL_Insert:

        return SCK_INSERT;

    case FL_Escape:

        return SCK_ESCAPE;

    case FL_BackSpace:

        return SCK_BACK;

    case FL_Tab:

        return SCK_TAB;

    case FL_Enter:

        return SCK_RETURN;

    case FL_KP+'+':

        return SCK_ADD;

    case FL_KP+'-':

        return SCK_SUBTRACT;

    case FL_KP+'/':

        return SCK_DIVIDE;

    case FL_Meta_L:

        return SCK_WIN;

    case FL_Meta_R:

        return SCK_RWIN;

    case FL_Menu:

        return SCK_MENU;

    default:

        return keyIn;

    }

}

void Fl_Scintilla::patchfix(){

    ScrollTo(mVScrollBar->value()+1);

    ScrollTo(mVScrollBar->value()-1); 

}

int Fl_Scintilla::handle(int event)

{

    int button;

    unsigned int clockt;

    //patch fix

    if (event == FL_KEYDOWN &&

        (Fl::event_key() == FL_BackSpace || Fl::event_key() == FL_Delete)) {

        patchfix();

    }

    if ( event == FL_HIDE ) {

        fl_reset_spot();

        if ( ct.wCallTip.Created() ) SendEditor(SCI_CALLTIPCANCEL);

        if ( ac.lb->Created() ) ac.lb->Destroy();

        return 1;

    }

    if ( event == FL_FOCUS ) {

        fl_set_spot(labelfont(), labelsize(), x(), y(), w(), h(), window());

        SetFocusState(true);

        DestroySystemCaret();

        CreateSystemCaret();

        return 1;

    }

    if ( event == FL_UNFOCUS ) {

        if ( ct.wCallTip.Created() ) SendEditor(SCI_CALLTIPCANCEL);

        if ( ac.lb->Created() ) ac.lb->Destroy();

        fl_reset_spot();

        SetFocusState(false);

        DestroySystemCaret();

        return 1;

    }

    // return 0;

    if ( event == FL_ENTER ) {

        Scintilla::Point pt = Scintilla::Point::FromInts(Fl::event_x()-swt_.rc_client.x, Fl::event_y()-swt_.rc_client.y);

        if (PointInSelMargin(pt)) {

            DisplayCursor(GetMarginCursor(pt));

        } else if (PointInSelection(pt) && !SelectionEmpty()) {

            DisplayCursor(Scintilla::Window::cursorArrow);

        } else if (PointIsHotspot(pt)) {

            DisplayCursor(Scintilla::Window::cursorHand);

        } else {

            if (Fl::event_inside(swt_.rc_client.x, swt_.rc_client.y, swt_.rc_client.w, swt_.rc_client.h)) {

                DisplayCursor(Scintilla::Window::cursorText);

            } else {

                DisplayCursor(Scintilla::Window::cursorArrow);

            }

        }

        return 1;

    }

    if ( event == FL_LEAVE ) {

        DisplayCursor(Scintilla::Window::cursorArrow);

        SetTrackMouseLeaveEvent(false);

        MouseLeave();

        return 1;

    }

    if ( event == FL_KEYDOWN ) {

        return handle_key(event);

    }

    if ( event == FL_MOVE ) {

        if (!Fl::event_inside(swt_.rc_client.x, swt_.rc_client.y, swt_.rc_client.w, swt_.rc_client.h)) {

            DisplayCursor(Scintilla::Window::cursorArrow);

            return 0;

        }

        const Scintilla::Point pt = Scintilla::Point::FromInts(Fl::event_x()-swt_.rc_client.x, Fl::event_y()-swt_.rc_client.y);

        // Windows might send WM_MOUSEMOVE even though the mouse has not been moved:

        // http\://blogs.msdn.com/b/oldnewthing/archive/2003/10/01/55108.aspx

        if (ptMouseLast.x != pt.x || ptMouseLast.y != pt.y) {

            SetTrackMouseLeaveEvent(true);

            ButtonMoveWithModifiers(pt,

                                    (Fl::event_state(FL_SHIFT) != 0 ? SCI_SHIFT : 0) |

                                    (Fl::event_state(FL_CTRL) != 0 ? SCI_CTRL : 0) |

                                    (Scintilla::Platform::IsKeyDown(FL_ALT) ? SCI_ALT : 0));

        }

        return 1;

    }

    if ( event == FL_PASTE ) {

        if (!Fl::event_text()) return 1;

        DoPaste(Fl::event_text());

        return 1;

    }

    if ( event == FL_DRAG ) {

        if ( scrollpush_ == 1 ) return mVScrollBar->handle(event);

        if ( scrollpush_ == 2 ) return mHScrollBar->handle(event);

        const Scintilla::Point pt = Scintilla::Point::FromInts(Fl::event_x()-swt_.rc_client.x, Fl::event_y()-swt_.rc_client.y);

        // Windows might send WM_MOUSEMOVE even though the mouse has not been moved:

        // http\://blogs.msdn.com/b/oldnewthing/archive/2003/10/01/55108.aspx

        if (ptMouseLast.x != pt.x || ptMouseLast.y != pt.y) {

            SetTrackMouseLeaveEvent(true);

            ButtonMoveWithModifiers(pt, 

                (Fl::event_state(FL_SHIFT) != 0 ? SCI_SHIFT : 0) | 

                (Fl::event_state(FL_CTRL) != 0 ? SCI_CTRL : 0) | 

                (Scintilla::Platform::IsKeyDown(FL_ALT) ? SCI_ALT : 0));

        }

        return 0;

    }

    if ( event == FL_DND_ENTER ) {

        DragEnter();

        return 1;

    }

    if ( event == FL_DND_DRAG ) {

        DragOver();

        return 1;

    }

    if ( event == FL_DND_LEAVE ) {

        DragLeave();

        return 1;

    }

    if ( event == FL_DND_RELEASE ) {

        Drop();

        return 1;

    }

    if ( event == FL_PUSH ) {

        //printf("fl_push\n");

        scrollpush_ = 0;

        if ( Fl::event_inside(mVScrollBar->x(), mVScrollBar->y(), mVScrollBar->w(), mVScrollBar->h()) && mVScrollBar->visible()) {

            scrollpush_ = 1;

            return mVScrollBar->handle(event);

        }

        if ( Fl::event_inside(mHScrollBar->x(), mHScrollBar->y(), mHScrollBar->w(), mHScrollBar->h()) && mHScrollBar->visible()) {

            scrollpush_ = 2;

            return mHScrollBar->handle(event);

        }

        //printf("push\n");

        if (!Fl::event_inside(swt_.rc_client.x, swt_.rc_client.y, swt_.rc_client.w, swt_.rc_client.h)) return 0;

        if (Fl::visible_focus() && handle(FL_FOCUS)) Fl::focus(this);

        button = Fl::event_button();

        if ( button == 1 ) {

            clockt = (clock() * 1000) / CLOCKS_PER_SEC;

            ButtonDown(Scintilla::Point::FromInts(Fl::event_x()-swt_.rc_client.x, Fl::event_y()-swt_.rc_client.y), clockt,

                       Fl::event_state(FL_SHIFT) != 0,

                       Fl::event_state(FL_CTRL) != 0,

                       Scintilla::Platform::IsKeyDown(FL_ALT));

            RefreshIME();

            LButtonDown = true;

            return 1;

        }

        if ( button == 3 ) {

            Scintilla::Point pt = Scintilla::Point::FromInts(Fl::event_x()-swt_.rc_client.x, Fl::event_y()-swt_.rc_client.y);

            if (!PointInSelection(pt)) {

                CancelModes();

                SetEmptySelection(PositionFromLocation(pt));

            }

            return 1;

        }

        return 0;

    }

    if ( event == FL_RELEASE ) {

        //printf("fl_release\n");

        if ( scrollpush_ == 1 ) return mVScrollBar->handle(event);

        if ( scrollpush_ == 2 ) return mHScrollBar->handle(event);

        scrollpush_ = 0;

        if ( !hasFocus ) return 0;      

        //if ( !Fl::visible_focus() ) return 0;

        button = Fl::event_button();

        if ( button == 1 ) {

            if (!LButtonDown ) {

                clockt = (clock() * 1000) / CLOCKS_PER_SEC;

                ButtonDown(Scintilla::Point::FromInts(Fl::event_x()-swt_.rc_client.x, Fl::event_y()-swt_.rc_client.y), clockt,

                           Fl::event_state(FL_SHIFT) != 0,

                           Fl::event_state(FL_CTRL) != 0,

                           Scintilla::Platform::IsKeyDown(FL_ALT));

            }

            clockt = (clock() * 1000) / CLOCKS_PER_SEC;

            ButtonUp(Scintilla::Point::FromInts(Fl::event_x()-swt_.rc_client.x, Fl::event_y()-swt_.rc_client.y), clockt, Fl::event_state(FL_CTRL) != 0);

            LButtonDown = false;

            return 1;

        }

        if ( button == 3 ) {

            if (displayPopupMenu) {

                Scintilla::Point pt = Scintilla::Point::FromInts(Fl::event_x()-swt_.rc_client.x, Fl::event_y()-swt_.rc_client.y);

                if (!PointInSelection(pt)) {

                    CancelModes();

                    SetEmptySelection(PositionFromLocation(pt));

#ifndef __EMSCRIPTEN__

                    Fl::wait(0);

#endif

                }

                if ((pt.x == -1) && (pt.y == -1)) {

                    // Caused by keyboard so display menu near caret

                    pt = PointMainCaret();

                    pt = Scintilla::Point::FromInts(pt.x, pt.y);

                }

                ContextMenu(pt);

            }

            return 1;

        }

        return 0;

    }

    if ( event == FL_MOUSEWHEEL ) {

        // if autocomplete list active then send mousewheel message to it

        if (ac.Active()) {

            SCIWinType *swt = (SCIWinType *)(ac.lb->GetID());

            Fl_Window *win = (Fl_Window *)swt->wid;

            win->handle(event);

            return 1;

        }

        if ( !hasFocus ) return 1;

        // if ( !hasFocus ) return 0;

        // Don't handle datazoom.

        // (A good idea for datazoom would be to "fold" or "unfold" details.

        // i.e. if datazoomed out only class structures are visible, when datazooming in the control

        // structures appear, then eventually the individual statements...)

        if ( Fl::event_state(FL_SHIFT) ) {

            return 0;

        }

        // Either SCROLL or ZOOM. We handle the wheel steppings calculation

        wheelDelta = Fl::event_dy();

        if ( wheelDelta != 0 && linesPerScroll > 0) {

            int linesToScroll = linesPerScroll;

            if (linesPerScroll == 0xffffffff)

                linesToScroll = LinesOnScreen() - 1;

            if (linesToScroll == 0) {

                linesToScroll = 1;

            }

            linesToScroll *= (wheelDelta);

            if (Fl::event_state()&FL_CTRL) {

                // Zoom! We play with the font sizes in the styles.

                // Number of steps/line is ignored, we just care if sizing up or down

                if (linesToScroll < 0) {

                    KeyCommand(SCI_ZOOMIN);

                } else {

                    KeyCommand(SCI_ZOOMOUT);

                }

            } else {

                // Scroll

                ScrollTo(topLine + linesToScroll+0);

                redraw();

                //Redraw();

            }

        }

        return 1;

    }

    return 0;//Fl_Group::handle(event);

}

int Fl_Scintilla::handle_key(int event)

{

    // printf("keydown:%d\n", Fl::event_key());

    char *s;

    void *p;

    int len, n, del=0;

    // caret

    RefreshIME();

    // Check for dead keys or active composition
    bool composed = Fl::compose(del);

    // If we are in a dead-key state, we should not insert anything yet.
    if (!composed && Fl::event_length() == 0) {
        // Pass to KeyDown so shortcuts still work, but do not insert text
        lastKeyDownConsumed = false;
        int ret = KeyDown(KeyTranslate(Fl::event_key()),
                          Scintilla::Platform::IsKeyDown(FL_SHIFT),
                          Scintilla::Platform::IsKeyDown(FL_CTRL),
                          Scintilla::Platform::IsKeyDown(FL_ALT),
                          &lastKeyDownConsumed);
        return 1; // Consume dead key to prevent "Dead" from being inserted
    }

    // shortcut
    if ( ! composed ) {
        if ((Fl::event_state()&(FL_CTRL|FL_COMMAND)) && Fl::event_key()=='z') {

            ScintillaBase::WndProc(SCI_UNDO,  0, 0);patchfix(); 

            return 1;

        }

        if ((Fl::event_state()&(FL_CTRL|FL_COMMAND)) && Fl::event_key()=='y') {

            ScintillaBase::WndProc(SCI_REDO,  0, 0);patchfix(); 

            return 1;

        }

        if ((Fl::event_state()&(FL_CTRL|FL_COMMAND)) && Fl::event_key()=='x') {

            ScintillaBase::WndProc(SCI_CUT,   0, 0);

            patchfix(); 

            return 1;

        }

        if ((Fl::event_state()&(FL_CTRL|FL_COMMAND)) && Fl::event_key()=='c') {

            ScintillaBase::WndProc(SCI_COPY,  0, 0);

            return 1;

        }

        if ((Fl::event_state()&(FL_CTRL|FL_COMMAND)) && Fl::event_key()=='v') {

            ScintillaBase::WndProc(SCI_PASTE, 0, 0);

            patchfix(); 

            return 1;

        }

        if ((Fl::event_state()&(FL_CTRL|FL_COMMAND)) && Fl::event_key()=='a') {

            ScintillaBase::WndProc(SCI_SELECTALL, 0, 0);

            return 1;

        }

        lastKeyDownConsumed = false;

        int ret = KeyDown(KeyTranslate(Fl::event_key()),

                          Scintilla::Platform::IsKeyDown(FL_SHIFT),

                          Scintilla::Platform::IsKeyDown(FL_CTRL),

                          Scintilla::Platform::IsKeyDown(FL_ALT),

                          &lastKeyDownConsumed);

        if ( ret || lastKeyDownConsumed ) return 1;

        return 0;

    }

    if (del) {

        int pos = CurrentPosition();

        SetSelection(pos-del, pos);

        ClearSelection();

    }

    // text

    if (!Fl::event_text()) return 0;
    unsigned char c = Fl::event_text()[0];
    if (((Fl::event_key() >= 128) || !iscntrl(c)) || !lastKeyDownConsumed) {
        s = (char *)Fl::event_text();
        len = strlen(s);
        if ( len < 1 ) return 0;

        // Filter out dead key literal string from FLTK IME
        if (len >= 4 && strncmp(s, "Dead", 4) == 0) return 1;

#ifdef __EMSCRIPTEN__

        // Emscripten is always UTF-8 – never go through iconv

        AddCharUTF(s, len);

#else

        if ( ! IsUnicodeMode() ) {

            printf("no unicode\n");

            if ( len*8 > ic_str_len_ ) {

                p = realloc(ic_str_, len*8);

                if ( p == NULL ) return 1;

                ic_str_ = (char *)p;

                ic_str_len_ = len*8; 

            }

            ic_->Open4MB(CodePageOfDocument());

            n = ic_->convert(s, len, ic_str_, len*8);

            ic_str_[n] = 0;

            s = ic_str_;

            len = n;

        }

        // printf("keydown:%d  %d %s \n", Fl::event_key(),len,s); 

        AddCharUTF(s, len);

#endif

        // patchfix();

        return 1;

    }

    return 0;

}

static bool BoundsContains(Scintilla::PRectangle rcBounds, Scintilla::PRectangle rcCheck)

{

    bool contains = true;

    if (!rcCheck.Empty()) {

        if (!rcBounds.Contains(rcCheck)) {

            contains = false;

        }

    }

    return contains;

}

void Fl_Scintilla::draw()

{

    fl_push_clip(x(), y(), w(), h());

#ifdef __EMSCRIPTEN__

    const bool full = true;   // backend may clear the canvas: always repaint everything

#else

    const bool full = (damage() & FL_DAMAGE_ALL) != 0;

#endif

    Fl_Color bgcolor = active_r() ? color() : fl_inactive(color());

    if (full) {

        draw_box(box(), x(), y(), w(), h(), bgcolor);

        if (mVScrollBar->visible() && mHScrollBar->visible()) {

            fl_rectf(mVScrollBar->x(), mHScrollBar->y(),

                     mVScrollBar->w(), mHScrollBar->h(), FL_GRAY);

        }

        mVScrollBar->damage(FL_DAMAGE_ALL);

        mHScrollBar->damage(FL_DAMAGE_ALL);

    }

    update_child(*mVScrollBar);

    update_child(*mHScrollBar);

    if (full) {

        int X, Y, W, H;

        if (!fl_clip_box(swt_.rc_client.x, swt_.rc_client.y,
                         swt_.rc_client.w, swt_.rc_client.h, X, Y, W, H)) {
            X = swt_.rc_client.x;
            Y = swt_.rc_client.y;
            W = swt_.rc_client.w;
            H = swt_.rc_client.h;
        }
#ifdef __EMSCRIPTEN__
        // Browser canvas damage can be incomplete after keyboard/input events.
        // Since `full` is true on WASM, repaint the complete client every time.
        X = swt_.rc_client.x;
        Y = swt_.rc_client.y;
        W = swt_.rc_client.w;
        H = swt_.rc_client.h;
#endif

        if (W > 0 && H > 0) {

            bool assertsPopup = Scintilla::Platform::ShowAssertionPopUps(false);

            fl_push_clip(X, Y, W, H);

            paintState = painting;

            rcPaint = Scintilla::PRectangle::FromInts(

                X - swt_.rc_client.x, Y - swt_.rc_client.y,

                X - swt_.rc_client.x + W, Y - swt_.rc_client.y + H);

            paintingAllText = BoundsContains(rcPaint, GetClientRectangle());

            if (sw_) {

                sw_->Init((void*)1, &swt_);

                Paint(sw_, rcPaint);

                sw_->Release();

            }

            if (paintState == paintAbandoned) FullPaint();

            paintState = notPainting;

            fl_pop_clip();

            Scintilla::Platform::ShowAssertionPopUps(assertsPopup);

        }

    }

    fl_pop_clip();

}

bool Fl_Scintilla::PaintContains(Scintilla::PRectangle rc)

{

    if (paintState == painting) {

        return BoundsContains(rcPaint, rc);

    }

    return true;

}

/**

 * Redraw all of text area.

 * This paint will not be abandoned.

 */

void Fl_Scintilla::FullPaint()

{

    paintState = painting;

    rcPaint = GetClientRectangle();

    paintingAllText = true;

    // Under Emscripten we must already be inside an FLTK draw() callback;

    // the Surface implementation must honour sid==1 as “current FLTK GC”.

    if (sw_) {

        sw_->Init((void*)1, &swt_);

        Paint(sw_, rcPaint);

        sw_->Release();

    }

    paintState = notPainting;

}

static int CodePageFromCharSet(int characterSet, int documentCodePage)

{

    if (documentCodePage == SC_CP_UTF8) return SC_CP_UTF8;

    switch (characterSet) {

    case SC_CHARSET_ANSI:

        return 1252;

    case SC_CHARSET_DEFAULT:

        return documentCodePage;

    case SC_CHARSET_BALTIC:

        return 1257;

    case SC_CHARSET_CHINESEBIG5:

        return 950;

    case SC_CHARSET_EASTEUROPE:

        return 1250;

    case SC_CHARSET_GB2312:

        return 936;

    case SC_CHARSET_GREEK:

        return 1253;

    case SC_CHARSET_HANGUL:

        return 949;

    case SC_CHARSET_MAC:

        return 10000;

    case SC_CHARSET_OEM:

        return 437;

    case SC_CHARSET_RUSSIAN:

        return 1251;

    case SC_CHARSET_SHIFTJIS:

        return 932;

    case SC_CHARSET_TURKISH:

        return 1254;

    case SC_CHARSET_JOHAB:

        return 1361;

    case SC_CHARSET_HEBREW:

        return 1255;

    case SC_CHARSET_ARABIC:

        return 1256;

    case SC_CHARSET_VIETNAMESE:

        return 1258;

    case SC_CHARSET_THAI:

        return 874;

    case SC_CHARSET_8859_15:

        return 28605;

        // Not supported

    case SC_CHARSET_CYRILLIC:

        return documentCodePage;

    case SC_CHARSET_SYMBOL:

        return documentCodePage;

    }

    return documentCodePage;

}

int Fl_Scintilla::CodePageOfDocument() const

{

    return CodePageFromCharSet(vs.styles[STYLE_DEFAULT].characterSet, pdoc->dbcsCodePage);

}

bool Fl_Scintilla::ValidCodePage(int codePage) const

{

    return codePage == 0 || codePage == SC_CP_UTF8 || codePage == 932 || codePage == 936 || codePage == 949 || codePage == 950 || codePage == 1361;

}

void Fl_Scintilla::SetTrackMouseLeaveEvent(bool on)

{

    // Do nothing

}

void Fl_Scintilla::SetMouseCapture(bool on)

{

    capturedMouse = on;

}

bool Fl_Scintilla::HaveMouseCapture()

{

    // Cannot just see if GetCapture is this window as the scroll bar also sets capture for the window

    return capturedMouse;

}

// =========== Copy & Paste =======================================

void Fl_Scintilla::CopyToClipboard(const Scintilla::SelectionText &selectedText)

{

    const char* copy = selectedText.Data();

    int len = static_cast<int>(selectedText.LengthWithTerminator());

    if ( (copy==NULL) || (len<1) ) return;

#ifdef __EMSCRIPTEN__

    // Always UTF-8 on Emscripten

    Fl::copy(copy, len-1, 1);

#else

    if (IsUnicodeMode()) {

        Fl::copy(copy, len-1, 1);

    } else {

        char *dst;

        int dstsize=len*8;

        dst = (char *)malloc(dstsize);

        ic_->Open4UTF8(CodePageOfDocument());

        int n = ic_->convert((char*)copy, len, dst, dstsize);

        if ( n <= 0 ) {

            free(dst);

            return;

        }

        // n includes the null terminator, so the real length is n - 1

        int realLen = n - 1;

        Fl::copy(dst, realLen, 1);

        // Fl::copy(dst, n, 1);

        free(dst);

    }

#endif

}

void Fl_Scintilla::Copy()

{

    if (!sel.Empty()) {

        Scintilla::SelectionText selectedText;

        CopySelectionRange(&selectedText);

        CopyToClipboard(selectedText);

    }

}

void Fl_Scintilla::CopyAllowLine()

{

    Scintilla::SelectionText selectedText;

    CopySelectionRange(&selectedText, true);

    CopyToClipboard(selectedText);

}

bool Fl_Scintilla::CanPaste()

{

    if (!Editor::CanPaste())

        return false;

    if ( Fl::event_text() != NULL ) return true;

    return false;

}

void Fl_Scintilla::Paste()

{

    Fl::paste(*this, 1);

}

void Fl_Scintilla::DoPaste(const char *s)

{

    const bool isLine = SelectionEmpty();

    ClearSelection(multiPasteMode == SC_MULTIPASTE_EACH);

    unsigned int len = strlen(s);

#ifdef __EMSCRIPTEN__

    // Always UTF-8

    InsertPasteShape(s, len, pasteStream);

#else

    if (IsUnicodeMode()) {

        InsertPasteShape(s, len, pasteStream);

    } else {

        char *dst;

        int dstsize=len*8;

        dst = (char *)malloc(dstsize);

        ic_->Open4MB(CodePageOfDocument());

        int n = ic_->convert((char*)s, len, dst, dstsize);

        if ( n <= 0 ) {

            free(dst);

            return;

        }

        InsertPasteShape(dst, n, pasteStream);

        free(dst);

    }

#endif

}

// =========== Dnd =======================================

bool Fl_Scintilla::DragThreshold(Scintilla::Point ptStart, Scintilla::Point ptNow)

{

    int xMove = static_cast<int>(std::abs(ptStart.x - ptNow.x));

    int yMove = static_cast<int>(std::abs(ptStart.y - ptNow.y));

    return (xMove > 4) || (yMove > 4);

}

void Fl_Scintilla::SaveDragData(const Scintilla::SelectionText &selectedText)

{

    drag_str_size_ = 0;

    const char* copy = selectedText.Data();

    int len = static_cast<int>(selectedText.LengthWithTerminator());

    if ( (copy==NULL) || (len<1) ) return;

    void *p;

#ifdef __EMSCRIPTEN__

    // Always UTF-8

    p = realloc(drag_str_, len+1);

    if ( p == NULL ) return;

    drag_str_ = (char *)p;

    memcpy(drag_str_, copy, len);

    drag_str_[len] = 0;

    drag_str_size_ = len+1;

#else

    if (IsUnicodeMode()) {

        p = realloc(drag_str_, len+1);

        if ( p == NULL ) return;

        drag_str_ = (char *)p;

        memcpy(drag_str_, copy, len);

        drag_str_[len] = 0;

        drag_str_size_ = len+1;

        //Fl::copy(copy, len, 1);

    } else {

        char *dst;

        int dstsize=len*8;

        dst = (char *)malloc(dstsize);

        ic_->Open4UTF8(CodePageOfDocument());

        int n = ic_->convert((char*)copy, len, dst, dstsize);

        if ( n <= 0 ) {

            free(dst);

            return;

        }

        p = realloc(drag_str_, len+1);

        if ( p == NULL ) {

            free(dst);

            return;

        }

        drag_str_ = (char *)p;

        memcpy(drag_str_, dst, n+1);

        drag_str_[n] = 0;

        drag_str_size_ = n+1;

        // Fl::copy(dst, n, 1);

        free(dst);

    }

#endif

}

void Fl_Scintilla::StartDrag()

{

    Scintilla::Editor::inDragDrop = ddDragging;

    dropWentOutside = true;

    if (!sel.Empty()) {

        Scintilla::SelectionText selectedText;

        CopySelectionRange(&selectedText);

        SaveDragData(selectedText);

        Fl::copy(drag_str_, drag_str_size_, 0);

    }

    Fl::dnd();

    Scintilla::Editor::inDragDrop = ddNone;

    SetDragPosition(Scintilla::SelectionPosition(Scintilla::invalidPosition));

}

void Fl_Scintilla::DragEnter()

{

    // do nothing

}

int Fl_Scintilla::DragOver()

{

    try {

        if ( pdoc->IsReadOnly()) {

            return 0;

        }

        // Update the cursor.

        Scintilla::Point pt = Scintilla::Point::FromInts(Fl::event_x()-swt_.rc_client.x, Fl::event_y()-swt_.rc_client.y);

        SetDragPosition(SPositionFromLocation(pt, false, false, UserVirtualSpace()));

#if defined(WIN32) && !defined(__EMSCRIPTEN__)

        Fl::wait(0);

#endif

        return 1;

    } catch (...) {

        errorStatus = SC_STATUS_FAILURE;

    }

    return 0;

}

void Fl_Scintilla::DragLeave()

{

    try {

        SetDragPosition(Scintilla::SelectionPosition(Scintilla::invalidPosition));

        return;

    } catch (...) {

        errorStatus = SC_STATUS_FAILURE;

    }

}

void Fl_Scintilla::Drop()

{

    try {

        SetDragPosition(Scintilla::SelectionPosition(Scintilla::invalidPosition));

        Scintilla::Point pt = Scintilla::Point::FromInts(Fl::event_x()-swt_.rc_client.x, Fl::event_y()-swt_.rc_client.y);

        Scintilla::SelectionPosition movePos = SPositionFromLocation(pt, false, false, UserVirtualSpace());

        char *data = "";

        int len = 0;

        if ( Fl::event_state(FL_CTRL) != 0 ) DropAt(movePos, data, len, 0, 1);

        else DropAt(movePos, data, len, 1, 1);

        return;

    } catch (...) {

        errorStatus = SC_STATUS_FAILURE;

    }

}

// =========== Case =======================================

class CaseFolderDBCS : public Scintilla::CaseFolderTable

{

    // Allocate the expandable storage here so that it does not need to be reallocated

    // for each call to Fold.

    std::vector<wchar_t> utf16Mixed;

    std::vector<wchar_t> utf16Folded;

    int cp;

    ICONVConverter ic_;

public:

    explicit CaseFolderDBCS(int cp_) : cp(cp_) {

        StandardASCII();

    }

    virtual size_t Fold(char *folded, size_t sizeFolded, const char *mixed, size_t lenMixed) {

        if ((lenMixed == 1) && (sizeFolded > 0)) {

            folded[0] = mapping[static_cast<unsigned char>(mixed[0])];

            return 1;

        } else {

            if (lenMixed > utf16Mixed.size()) {

                utf16Mixed.resize(lenMixed + 8);

            }

            ic_.OpenFromCodepage(cp, "WCHAR_T");

            size_t nUtf16Mixed = ic_.convert((char*)mixed,

                                             static_cast<int>(lenMixed),

                                             (char*)&utf16Mixed[0],

                                             static_cast<int>(utf16Mixed.size()));

            /*

            size_t nUtf16Mixed = ::MultiByteToWideChar(cp, 0, mixed,

                static_cast<int>(lenMixed),

                &utf16Mixed[0],

                static_cast<int>(utf16Mixed.size()));

            */

            if (nUtf16Mixed < 1) {

                // Failed to convert -> bad input

                folded[0] = '\0';

                return 1;

            }

            unsigned int lenFlat = 0;

            for (size_t mixIndex=0; mixIndex < nUtf16Mixed; mixIndex++) {

                if ((lenFlat + 20) > utf16Folded.size()) utf16Folded.resize(lenFlat + 60);

                const char *foldedUTF8 = CaseConvert(utf16Mixed[mixIndex], Scintilla::CaseConversionFold);

                if (foldedUTF8) {

                    // Maximum length of a case conversion is 6 bytes, 3 characters

                    wchar_t wFolded[20];

                    size_t charsConverted = Scintilla::UTF16FromUTF8(foldedUTF8,

                                            strlen(foldedUTF8),

                                            wFolded, ELEMENTS(wFolded));

                    for (size_t j=0; j<charsConverted; j++)

                        utf16Folded[lenFlat++] = wFolded[j];

                } else {

                    utf16Folded[lenFlat++] = utf16Mixed[mixIndex];

                }

            }

            ic_.OpenToCodepage("WCHAR_T", cp);

            size_t lenOut = ic_.GetLength((char*)&utf16Folded[0], lenFlat);

            /*

            size_t lenOut = ::WideCharToMultiByte(cp, 0,

                &utf16Folded[0], lenFlat,

                NULL, 0, NULL, 0);

            */

            if (lenOut < sizeFolded) {

                ic_.convert((char*)&utf16Folded[0], lenFlat, folded, static_cast<int>(lenOut));

                /*

                ::WideCharToMultiByte(cp, 0,

                    &utf16Folded[0], lenFlat,

                    folded, static_cast<int>(lenOut), NULL, 0);

                */

                return lenOut;

            } else {

                return 0;

            }

        }

    }

};

Scintilla::CaseFolder *Fl_Scintilla::CaseFolderForEncoding()

{

#ifdef __EMSCRIPTEN__

    // Always UTF-8

    return new Scintilla::CaseFolderUnicode();

#else

    ICONVConverter *ic;

    int cpDest = CodePageOfDocument();

    if (cpDest == SC_CP_UTF8) {

        return new Scintilla::CaseFolderUnicode();

    } else {

        if (pdoc->dbcsCodePage == 0) {

            Scintilla::CaseFolderTable *pcf = new Scintilla::CaseFolderTable();

            pcf->StandardASCII();

            // Only for single byte encodings

            int cpDoc = CodePageOfDocument();

            for (int i=0x80; i<0x100; i++) {

                char sCharacter[2] = "A";

                sCharacter[0] = static_cast<char>(i);

                wchar_t wCharacter[20];

                ic = new ICONVConverter();

                ic->OpenFromCodepage(cpDoc, "WCHAR_T");

                unsigned int lengthUTF16 = ic->convert((char*)sCharacter, 1, (char*)wCharacter, ELEMENTS(wCharacter));

                //unsigned int lengthUTF16 = ::MultiByteToWideChar(cpDoc, 0, sCharacter, 1, wCharacter, ELEMENTS(wCharacter));

                if (lengthUTF16 == 1) {

                    const char *caseFolded = CaseConvert(wCharacter[0], Scintilla::CaseConversionFold);

                    if (caseFolded) {

                        wchar_t wLower[20];

                        size_t charsConverted = Scintilla::UTF16FromUTF8(caseFolded,

                                                strlen(caseFolded),

                                                wLower, ELEMENTS(wLower));

                        if (charsConverted == 1) {

                            char sCharacterLowered[20];

                            ic->OpenToCodepage("WCHAR_T", cpDoc);

                            unsigned int lengthConverted = ic->convert((char*)wLower, static_cast<int>(charsConverted), sCharacterLowered, ELEMENTS(sCharacterLowered));

                            //unsigned int lengthConverted = ::WideCharToMultiByte(cpDoc, 0, wLower, static_cast<int>(charsConverted), sCharacterLowered, ELEMENTS(sCharacterLowered), NULL, 0);

                            if ((lengthConverted == 1) && (sCharacter[0] != sCharacterLowered[0])) {

                                pcf->SetTranslation(sCharacter[0], sCharacterLowered[0]);

                            }

                        }

                    }

                }

                delete ic;

            }

            return pcf;

        } else {

            return new CaseFolderDBCS(cpDest);

        }

    }

#endif

}

std::string Fl_Scintilla::CaseMapString(const std::string &s, int caseMapping)

{

    if ((s.size() == 0) || (caseMapping == cmSame))

        return s;

    int cpDoc = CodePageOfDocument();

    if (cpDoc == SC_CP_UTF8) {

        std::string retMapped(s.length() * Scintilla::maxExpansionCaseConversion, 0);

        size_t lenMapped = CaseConvertString(&retMapped[0], retMapped.length(), s.c_str(), s.length(),

                                             (caseMapping == cmUpper) ? Scintilla::CaseConversionUpper : Scintilla::CaseConversionLower);

        retMapped.resize(lenMapped);

        return retMapped;

    }

    return s;

    /*

    ICONVConverter *ic;

    ic = new ICONVConverter();

    ic->OpenFromCodepage(cpDoc, "WCHAR_T");

    unsigned int lengthUTF16 = ic->GetLength((char*)s.c_str(), static_cast<int>(s.size()));

    //unsigned int lengthUTF16 = ::MultiByteToWideChar(cpDoc, 0, s.c_str(), static_cast<int>(s.size()), NULL, 0);

    if (lengthUTF16 == 0) { // Failed to convert

        delete ic;

        return s;

    }

    //DWORD mapFlags = LCMAP_LINGUISTIC_CASING | ((caseMapping == cmUpper) ? LCMAP_UPPERCASE : LCMAP_LOWERCASE);

    // Change text to UTF-16

    std::vector<wchar_t> vwcText(lengthUTF16);

    ic->convert((char*)s.c_str(), static_cast<int>(s.size()), (char*)&vwcText[0], lengthUTF16);

    //::MultiByteToWideChar(cpDoc, 0, s.c_str(), static_cast<int>(s.size()), &vwcText[0], lengthUTF16);

    // Change case

    int charsConverted = ::LCMapStringW(LOCALE_SYSTEM_DEFAULT, mapFlags,

        &vwcText[0], lengthUTF16, NULL, 0);

    std::vector<wchar_t> vwcConverted(charsConverted);

    ::LCMapStringW(LOCALE_SYSTEM_DEFAULT, mapFlags,

        &vwcText[0], lengthUTF16, &vwcConverted[0], charsConverted);

    // Change back to document encoding

    unsigned int lengthConverted = ::WideCharToMultiByte(cpDoc, 0,

        &vwcConverted[0], static_cast<int>(vwcConverted.size()),

        NULL, 0, NULL, 0);

    std::vector<char> vcConverted(lengthConverted);

    ::WideCharToMultiByte(cpDoc, 0,

        &vwcConverted[0], static_cast<int>(vwcConverted.size()),

        &vcConverted[0], static_cast<int>(vcConverted.size()), NULL, 0);

    delete ic;

    return std::string(&vcConverted[0], vcConverted.size());

    */

}

// ================================================================

void Fl_Scintilla::NotifyChange()

{

    ScintillaBase::Command(SCEN_CHANGE);

}

void Fl_Scintilla::ScrollText(int /* linesToMove */)

{

    Redraw();

    UpdateSystemCaret();

}

void Fl_Scintilla::SetVerticalScrollPos()

{

    if ( topLine != mVScrollBar->value() ) {

        DwellEnd(true);

        mVScrollBar->value(topLine);

    }

}

void Fl_Scintilla::SetHorizontalScrollPos()

{

    if ( mHScrollBar->value() != xOffset ) {

        DwellEnd(true);

        mHScrollBar->value(xOffset);

    }

}

bool Fl_Scintilla::ModifyScrollBars(int nMax, int nPage)

{

    unsigned char vshow=1, hshow=1;

    bool modified = false;

    int pos, windowSize, first, total;

    first = (int)mVScrollBar->minimum();

    total = (int)mVScrollBar->maximum()+0;

    windowSize = (int)mVScrollBar->slider_size();

    pos = (int)mVScrollBar->value();

    int vertEndPreferred = nMax+1; //fixed

    if (!verticalScrollBarVisible) nPage = vertEndPreferred + 1;

    if ( nPage > vertEndPreferred ) vshow = 0;

    if ( (first != 0) || (total != vertEndPreferred) || (windowSize != nPage) || (pos != 0) ) {

        mVScrollBar->value(0, nPage, 0, vertEndPreferred);

        mVScrollBar->value(pos+0);

        modified = true;

    }

    Scintilla::PRectangle rcText = GetTextRectangle();

    int horizEndPreferred = scrollWidth;

    if (horizEndPreferred < 0) horizEndPreferred = 0;

    unsigned int pageWidth = static_cast<unsigned int>(rcText.Width());

    if (!horizontalScrollBarVisible || Wrapping()) pageWidth = horizEndPreferred + 1;

    if ( (int)pageWidth > horizEndPreferred ) hshow = 0;

    first = (int)mHScrollBar->minimum();

    total = (int)mHScrollBar->maximum();

    windowSize = (int)mHScrollBar->slider_size();

    pos = (int)mHScrollBar->value();

    if ( (first != 0) || (total != horizEndPreferred) || (windowSize != pageWidth) || (pos != 0)) {

        mHScrollBar->value(0, pageWidth, 0, horizEndPreferred);

        mHScrollBar->value(pos);

        modified = true;

        if (scrollWidth < (int)(pageWidth)) {

            HorizontalScrollTo(0);

        }

    }

    //

    if ( vshow == 0 && hshow == 0 ) {

        swt_.rc_client.x = x()+Fl::box_dx(box());

        swt_.rc_client.y = y()+Fl::box_dy(box());

        swt_.rc_client.w = w()-Fl::box_dw(box());

        swt_.rc_client.h = h()-Fl::box_dh(box());

        mVScrollBar->clear_visible();

        mHScrollBar->clear_visible();

    } else if ( vshow == 1 && hshow == 0 ) {

        swt_.rc_client.x = x()+Fl::box_dx(box());

        swt_.rc_client.y = y()+Fl::box_dy(box());

        swt_.rc_client.w = w()-Fl::box_dw(box()) - scrollbar_width();

        swt_.rc_client.h = h()-Fl::box_dh(box());

        mVScrollBar->set_visible();

        mVScrollBar->resize(swt_.rc_client.x+swt_.rc_client.w, swt_.rc_client.y, scrollbar_width(), swt_.rc_client.h);

        mHScrollBar->clear_visible();

    } else if ( vshow == 0 && hshow == 1 ) {

        swt_.rc_client.x = x()+Fl::box_dx(box());

        swt_.rc_client.y = y()+Fl::box_dy(box());

        swt_.rc_client.w = w()-Fl::box_dw(box());

        swt_.rc_client.h = h()-Fl::box_dh(box()) - scrollbar_width();

        mVScrollBar->clear_visible(); //printf("clear v scroll......, %d %d %d %d\n", swt_.rc_client.x, swt_.rc_client.y, swt_.rc_client.w, swt_.rc_client.h);

        mHScrollBar->set_visible();

        mHScrollBar->resize(swt_.rc_client.x, swt_.rc_client.y+swt_.rc_client.h, swt_.rc_client.w, scrollbar_width());

    } else if ( vshow == 1 && hshow == 1 ) {

        swt_.rc_client.x = x()+Fl::box_dx(box());

        swt_.rc_client.y = y()+Fl::box_dy(box());

        swt_.rc_client.w = w()-Fl::box_dw(box()) - scrollbar_width();

        swt_.rc_client.h = h()-Fl::box_dh(box()) - scrollbar_width();

        mVScrollBar->set_visible();

        mVScrollBar->resize(swt_.rc_client.x+swt_.rc_client.w, swt_.rc_client.y, scrollbar_width(), swt_.rc_client.h);

        mHScrollBar->set_visible();

        mHScrollBar->resize(swt_.rc_client.x, swt_.rc_client.y+swt_.rc_client.h, swt_.rc_client.w, scrollbar_width());

    }

    if (paintState == notPainting) DropGraphics(false);

    //damage(FL_DAMAGE_ALL);

    //Redraw();

    damage(FL_DAMAGE_ALL, mVScrollBar->x(), mHScrollBar->y(), mVScrollBar->w(), mHScrollBar->h());

    //printf("%s, %d, %d, modify:%d, new rc:%d %d %d %d, v%d, h%d\n", __FUNCTION__, nMax, nPage, modified, swt_.rc_client.x, swt_.rc_client.y, swt_.rc_client.w, swt_.rc_client.h, mVScrollBar->visible(), mHScrollBar->visible());

    return modified;

}

void Fl_Scintilla::NotifyFocus(bool focus)

{

    Command(focus ? SCEN_SETFOCUS : SCEN_KILLFOCUS);

    Scintilla::Editor::NotifyFocus(focus);

}

void Fl_Scintilla::RefreshIME()

{

    Scintilla::Point pos = PointMainCaret();

    int x = Scintilla::RoundXYPosition(pos.x)+swt_.rc_client.x;

    int y = Scintilla::RoundXYPosition(pos.y)+swt_.rc_client.y;

    int height = fl_height(labelfont(), labelsize());

#if defined(__APPLE__)

    Fl::insertion_point_location(x, y+height, height);

#elif defined(__EMSCRIPTEN__)

    // No native IME caret on the canvas; keep the widget focused.

    // (Optional: expose a JS helper that moves a hidden <input>.)

#else

    fl_set_spot(labelfont(), labelsize(), x, y+height, w(), h(), window());

#endif

}

void Fl_Scintilla::UpdateSystemCaret()

{

    if (hasFocus) {

        if (HasCaretSizeChanged()) {

            DestroySystemCaret();

            CreateSystemCaret();

        }

    }

}

void Fl_Scintilla::MoveImeCarets(int offset)

{

    // Move carets relatively by bytes.

    for (size_t r=0; r<sel.Count(); r++) {

        int positionInsert = sel.Range(r).Start().Position();

        sel.Range(r).caret.SetPosition(positionInsert + offset);

        sel.Range(r).anchor.SetPosition(positionInsert + offset);

    }

}

bool Fl_Scintilla::HasCaretSizeChanged() const

{

    if (

            ( (0 != vs.caretWidth) && (sysCaretWidth != vs.caretWidth) )

            || ((0 != vs.lineHeight) && (sysCaretHeight != vs.lineHeight))

    ) {

        return true;

    }

    return false;

}

bool Fl_Scintilla::CreateSystemCaret()

{

    sysCaretWidth = vs.caretWidth;

    if (0 == sysCaretWidth) sysCaretWidth = 1;

    sysCaretHeight = vs.lineHeight;

    return 1;

}

bool Fl_Scintilla::DestroySystemCaret()

{

    return 1;

}

bool Fl_Scintilla::FineTickerRunning(TickReason reason)

{

    return timeractive_[reason-tickCaret] != 0;

}

void Fl_Scintilla::FineTickerStart(TickReason reason, int millis, int tolerance)

{

    FineTickerCancel(reason);

    double tick_timeout=millis/1000.0;

    if ( reason == tickCaret && timeractive_[0] == 0 ) {

        Fl::add_timeout(tick_timeout, static_time_fun_1, this);

        timeractive_[0] = 1;

        timetick_[0] = tick_timeout;

    }

    if ( reason == tickScroll && timeractive_[1] == 0 ) {

        Fl::add_timeout(tick_timeout, static_time_fun_2, this);

        timeractive_[1] = 1;

        timetick_[1] = tick_timeout;

    }

    if ( reason == tickWiden && timeractive_[2] == 0 ) {

        Fl::add_timeout(tick_timeout, static_time_fun_3, this);

        timeractive_[2] = 1;

        timetick_[2] = tick_timeout;

    }

    if ( reason == tickDwell && timeractive_[3] == 0 ) {

        Fl::add_timeout(tick_timeout, static_time_fun_4, this);

        timeractive_[3] = 1;

        timetick_[3] = tick_timeout;

    }

    if ( reason == tickPlatform && timeractive_[4] == 0 ) {

        Fl::add_timeout(tick_timeout, static_time_fun_5, this);

        timeractive_[4] = 1;

        timetick_[4] = tick_timeout;

    }

}

void Fl_Scintilla::FineTickerCancel(TickReason reason)

{

    if ( reason == tickCaret && timeractive_[0] == 1 ) {

        Fl::remove_timeout(static_time_fun_1, this);

        timeractive_[0] = 0;

    }

    if ( reason == tickScroll && timeractive_[1] == 1 ) {

        Fl::remove_timeout(static_time_fun_2, this);

        timeractive_[1] = 0;

    }

    if ( reason == tickWiden && timeractive_[2] == 1 ) {

        Fl::remove_timeout(static_time_fun_3, this);

        timeractive_[2] = 0;

    }

    if ( reason == tickDwell && timeractive_[3] == 1 ) {

        Fl::remove_timeout(static_time_fun_4, this);

        timeractive_[3] = 0;

    }

    if ( reason == tickPlatform && timeractive_[4] == 1 ) {

        Fl::remove_timeout(static_time_fun_5, this);

        timeractive_[4] = 0;

    }

}

bool Fl_Scintilla::SetIdle(bool on)

{

    // On Win32 the Idler is implemented as a Timer on the Scintilla window.  This

    // takes advantage of the fact that WM_TIMER messages are very low priority,

    // and are only posted when the message queue is empty, i.e. during idle time.

    if (idler.state != on) {

        if (on) {

            idle_dwstart_ = 0;          

            Fl::add_timeout(10.0/1000.0, static_time_idle, this);

            idler.idlerID = (void*)1; //::SetTimer(MainHWND(), idleTimerID, 10, NULL) ? reinterpret_cast<IdlerID>(idleTimerID) : 0;

        } else {

            Fl::remove_timeout(static_time_idle, this);

            //::KillTimer(MainHWND(), reinterpret_cast<uptr_t>(idler.idlerID));

            idler.idlerID = 0;

        }

        idler.state = idler.idlerID != 0;

    }

    return idler.state;

}

void Fl_Scintilla::time_fun_idle()

{

    if ( ! idler.state ) return;

    if (idler.state) {

        //if (lParam || (WAIT_TIMEOUT == MsgWaitForMultipleObjects(0, 0, 0, 0, QS_INPUT|QS_HOTKEY))) {

        if (Idle()) {

            // User input was given priority above, but all events do get a turn.  Other

            // messages, notifications, etc. will get interleaved with the idle messages.

            // However, some things like WM_PAINT are a lower priority, and will not fire

            // when there's a message posted.  So, several times a second, we stop and let

            // the low priority events have a turn (after which the timer will fire again).

            unsigned int dwCurrent = (unsigned int)clock();

            unsigned int dwStart = idle_dwstart_ ? idle_dwstart_ : dwCurrent;

            const unsigned int maxWorkTime = 50;

            if (dwCurrent >= dwStart && dwCurrent > maxWorkTime && dwCurrent - maxWorkTime < dwStart) {

                //PostMessage(MainHWND(), SC_WIN_IDLE, dwStart, 0);

                idle_dwstart_ = dwStart;

                Fl::repeat_timeout(10.0/1000.0, static_time_idle, this);

            }

        } else {

            SetIdle(false);

        }

    }

}

void Fl_Scintilla::time_fun(int index)

{

    if ( index == 0 ) TickFor(tickCaret);

    else if ( index == 1 ) TickFor(tickScroll);

    else if ( index == 2 ) TickFor(tickWiden);

    else if ( index == 3 ) TickFor(tickDwell);

    else if ( index == 4 ) TickFor(tickPlatform);

    if ( index == 0 ) Fl::repeat_timeout(timetick_[0], static_time_fun_1, this);

    if ( index == 1 ) Fl::repeat_timeout(timetick_[1], static_time_fun_2, this);

    if ( index == 2 ) Fl::repeat_timeout(timetick_[2], static_time_fun_3, this);

    if ( index == 3 ) Fl::repeat_timeout(timetick_[3], static_time_fun_4, this);

    if ( index == 4 ) Fl::repeat_timeout(timetick_[4], static_time_fun_5, this);

}

// ================================================================

void Fl_Scintilla::update_v_scrollbar()

{

    ScrollTo(mVScrollBar->value());

}

void Fl_Scintilla::update_h_scrollbar()

{

    int xPos = xOffset;

    Scintilla::PRectangle rcText = GetTextRectangle();

    int pageWidth = static_cast<int>(rcText.Width() * 2 / 3);

    //printf("%d, %d\n", mHScrollBar->value(), xOffset);

    HorizontalScrollTo(mHScrollBar->value());

}

void Fl_Scintilla::v_scrollbar_cb(Fl_Scrollbar* b, Fl_Scintilla* textD)

{

    textD->update_v_scrollbar();

}

void Fl_Scintilla::h_scrollbar_cb(Fl_Scrollbar* b, Fl_Scintilla* textD)

{

    textD->update_h_scrollbar();

}

// =========== call tip =======================================

class Fl_CallWindow : public Fl_Double_Window

{

public:

    Fl_CallWindow(Scintilla::CallTip *ct, void dopush(void *x), void *x, int X, int Y, int W, int H, const char *l=0) : Fl_Double_Window(X, Y, W, H, l) {

        ct_ = ct;

        cb_notify_.callback = dopush;

        cb_notify_.data = x;

        swt_.type = 2;

        swt_.wid = this;

    }

    SCIWinType swt_;

protected:

    Scintilla::CallTip *ct_;

    void draw() {

        // return;

        fl_push_clip(0, 0, w(), h());

        Scintilla::Surface *surfaceWindow = Scintilla::Surface::Allocate(0);

        if (surfaceWindow) {

            surfaceWindow->Init((void*)2, &swt_);

            surfaceWindow->SetUnicodeMode(SC_CP_UTF8 == ct_->codePage);

            surfaceWindow->SetDBCSMode(ct_->codePage);

            ct_->PaintCT(surfaceWindow);

            surfaceWindow->Release();

            delete surfaceWindow;

        }

        fl_pop_clip();

    }

    int handle(int e) {

        // if (e == FL_KEYDOWN && Fl::event_key() == FL_BackSpace)update_v_scrollbar();

        if ( e == FL_PUSH ) {

            int x = Fl::event_x();

            int y = Fl::event_y();

            ct_->MouseClick(Scintilla::Point::FromInts(x, y));

            cb_notify_.callback(cb_notify_.data);

            return 1;

        }

        return Fl_Double_Window::handle(e);

    }

    struct {

        void (*callback)(void *data);

        void *data;

    } cb_notify_;

};

void Fl_Scintilla::CallTip_DoPush()

{

    CallTipClick();

}

static void calltip_dopush(void *x)

{

    Fl_Scintilla *sci = (Fl_Scintilla *)x;

    sci->CallTip_DoPush();

}

void Fl_Scintilla::CreateCallTipWindow(Scintilla::PRectangle rc)

{

    if ( ct.wCallTip.Created()) return;

    int wx=x(), wy=y();

    for (Fl_Window* w = this->window(); w; w = w->window()) {

        wx += w->x();

        wy += w->y();

    }

    Fl_Group::current(0);

    callwin_ = new Fl_CallWindow(&ct, calltip_dopush, this, (int)rc.left+wx, (int)rc.top+wy, (int)rc.Width(), (int)rc.Height());

    callwin_->end();

    callwin_->clear_border();

    callwin_->set_tooltip_window();

    Fl_CallWindow* cw = (Fl_CallWindow*)callwin_;

    cw->swt_.main_client.x = wx;

    cw->swt_.main_client.y = wy;

    ct.wCallTip = &(cw->swt_);

    ct.wDraw = ct.wCallTip;

#ifdef __EMSCRIPTEN__

    // Make sure the floating DIV is shown and redrawn

    callwin_->show();

    callwin_->redraw();

#endif

}


#endif // SCINTILLA_FLTK_IMPLEMENTATION

#endif // SCINTILLA_FLTK_AMALGAMATED_H