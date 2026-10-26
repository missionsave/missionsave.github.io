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

#include "PlatFLTK.h"

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
		if ( name != NULL )	name_ = (char*)malloc(strlen(name)+1);
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

// void SurfaceFLTK::Release()
// {
// 	if ( offscreen_buffer != 0 ) {
// 		fl_delete_offscreen(offscreen_buffer);
// 		offscreen_buffer = 0;
// 	}

// 	scibase_ = 0;
// 	win_ = 0;
// 	drawtype_ = 0;
// 	swt_ = 0;
// }

bool SurfaceFLTK::Initialised()
{
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
	if (width > 0 && height > 0)
		offscreen_buffer = fl_create_offscreen(width, height);

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
	int x = Scintilla::RoundXYPosition(rc.left);
	int y = Scintilla::RoundXYPosition(rc.top);
	int w = Scintilla::RoundXYPosition(rc.right-rc.left);
	int h = Scintilla::RoundXYPosition(rc.bottom-rc.top);
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
			// fallback solid red so we can see the problem
			FillRectangle(rc, Scintilla::ColourDesired(0xff, 0, 0));
			return;
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
		// Something is wrong so display in red
		FillRectangle(rc, Scintilla::ColourDesired(0xff, 0, 0));
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
// 	int x = Scintilla::RoundXYPosition(rc.left);
// 	int y = Scintilla::RoundXYPosition(rc.top);
// 	int w = Scintilla::RoundXYPosition(rc.right-rc.left);
// 	int h = Scintilla::RoundXYPosition(rc.bottom-rc.top);

// 	// Simple outline (alpha ignored on pure FLTK canvas)
// 	if ( drawtype_ == 0 ) {
// 		if (!offscreen_buffer) return;
// 		fl_begin_offscreen(offscreen_buffer);
// 		fl_color(outline.GetRed(), outline.GetGreen(), outline.GetBlue());
// 		fl_rect(x, y, w, h);
// 		fl_end_offscreen();
// 	} else {
// 		fl_color(outline.GetRed(), outline.GetGreen(), outline.GetBlue());
// 		fl_rect(x + ox(), y + oy(), w, h);
// 	}
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
		fl_font(fd->ff_, fd->size_);
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
	fl_font(fd->ff_, fd->size_);
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

void SurfaceFLTK::DrawTextClipped(
    Scintilla::PRectangle rc,
    Scintilla::Font &font_,
    Scintilla::XYPOSITION ybase,
    const char *s,
    int len,
    Scintilla::ColourDesired fore,
    Scintilla::ColourDesired back)
{
    SetClip(rc);
    DrawTextNoClip(rc, font_, ybase, s, len, fore, back);

    if (clipped_) {
        fl_pop_clip();
        clipped_ = false;
    }
}

void SurfaceFLTK::DrawTextTransparent(Scintilla::PRectangle rc, Scintilla::Font &font_, Scintilla::XYPOSITION ybase, const char *s, int len, Scintilla::ColourDesired fore)
{
	DrawTextNoClip(rc, font_, ybase, s, len, fore, Scintilla::ColourDesired(0,0,0));
}

Scintilla::XYPOSITION SurfaceFLTK::WidthText(Scintilla::Font &font_, const char *s, int len)
{
	FontData* fd = (FontData*)(font_.GetID());
	if (!fd) return 0;
	fl_font(fd->ff_, fd->size_);
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
	fl_font(fd->ff_, fd->size_);

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
	fl_font(fd->ff_, fd->size_);
	return (Scintilla::XYPOSITION)fl_width(ch);
}

// Scintilla::XYPOSITION SurfaceFLTK::Ascent(Scintilla::Font &font_)
// {
// 	FontData* fd = (FontData*)(font_.GetID());
// 	if (!fd) return 0;
// 	fl_font(fd->ff_, fd->size_);
// 	return (Scintilla::XYPOSITION)(fl_height() - fl_descent());
// }

// Scintilla::XYPOSITION SurfaceFLTK::Descent(Scintilla::Font &font_)
// {
// 	FontData* fd = (FontData*)(font_.GetID());
// 	if (!fd) return 0;
// 	fl_font(fd->ff_, fd->size_);
// 	return (Scintilla::XYPOSITION)fl_descent();
// }

// helper near the top of PlatFLTK.cxx
#ifdef __EMSCRIPTEN__
static inline int WasmDescent(int size) { return (size * 28 + 50) / 100; }   // ~0.28em -> 3 at 12
static inline int WasmHeight(int size)  { return (size * 140 + 50) / 100; }  // ~1.4em  -> 17 at 12, same as X11
#endif

Scintilla::XYPOSITION SurfaceFLTK::Ascent(Scintilla::Font &font_)
{
	FontData* fd = (FontData*)(font_.GetID());
	if (!fd) return 0;
#ifdef __EMSCRIPTEN__
	return (Scintilla::XYPOSITION)(WasmHeight(fd->size_) - WasmDescent(fd->size_));
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
	return (Scintilla::XYPOSITION)WasmDescent(fd->size_);
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
	return (Scintilla::XYPOSITION)WasmHeight(fd->size_);
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

// Scintilla::XYPOSITION SurfaceFLTK::Height(Scintilla::Font &font_)
// {
// 	FontData* fd = (FontData*)(font_.GetID());
// 	if (!fd) return 0;
// 	return (Scintilla::XYPOSITION)fl_height(fd->ff_, fd->size_);
// }

Scintilla::XYPOSITION SurfaceFLTK::AverageCharWidth(Scintilla::Font &font_)
{
	FontData* fd = (FontData*)(font_.GetID());
	if (!fd) return 0;
	fl_font(fd->ff_, fd->size_);
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
}






// void SurfaceFLTK::SetClip(Scintilla::PRectangle rc)
// {
// 	// The Fl_Scintilla::draw() already pushes a clip for the client area.
// 	// Additional clipping is not required (and fl_push_clip is nested carefully
// 	// by FLTK itself). Leaving this empty is correct for all backends including
// 	// Emscripten.
// }

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
	win->labelsize(fd->size_);
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

// bool Scintilla::Window::HasFocus()
// {
// 	SCIWinType *swt = (SCIWinType *)wid;
// 	if ( swt->type == 0 ) {
// 		Fl_Group *w = (Fl_Group *)swt->wid;
// 		if ( w->visible_focus() ) {
// 			return true;
// 		}
// 		return false;
// 	}

// 	Fl_Window *win = (Fl_Window *)swt->wid;
// 	if ( win->visible_focus() ) return true;
// 	return false;
// }
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
		sci->labelsize(fd->size_);
	} else {
		Fl_Window *win = (Fl_Window *)swt->wid;
		win->labelfont(fd->ff_);
		win->labelsize(fd->size_);
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
