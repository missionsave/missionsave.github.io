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

#include "CallTip.h"
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