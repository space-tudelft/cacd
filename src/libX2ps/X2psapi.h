/*
 * ISC License
 *
 * Copyright (C) 1994-2018 by
 *	P.C. van der Wekken
 * Delft University of Technology
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _X2psapi_h
#define _X2psapi_h

#include <stdio.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

#if defined(__STDC__) || defined(__cplusplus)
# define _FARGS_(s) s
#else
# define _FARGS_(s) ()
#endif

#define XPSTRUCTID    "XpStructId"

typedef struct {
    int format;
    Boolean orientation;
    float paperwidth;
    float paperwidthoffset;
    float paperheight;
    float paperheightoffset;
    int dpi;
}XpPSStruct;

XpFontmap* XpFontmapCreate(
#if NeedFunctionPrototypes
#endif
);

void XpFontmapShow(
#if NeedFunctionPrototypes
    XpFontmap*        /* xpfontmap */
#endif
);

void XpFontmapFree(
#if NeedFunctionPrototypes
    XpFontmap*        /* xpfontmap */
#endif
);

XpFontmap* XpFontmapDup(
#if NeedFunctionPrototypes
    XpFontmap*        /* xpfontmap */
#endif
);

void XpStructShow(
#if NeedFunctionPrototypes
    XpStruct*        /* xpinfo */
#endif
);

XpStruct* XpStructCreate(
#if NeedFunctionPrototypes
#endif
);

void XpStructFree(
#if NeedFunctionPrototypes
    XpStruct*        /* xpinfo */
#endif
);

Boolean XpIsXpStruct(
#if NeedFunctionPrototypes
    XpStruct*        /* xpinfo */
#endif
);

XpStruct* XpStructDup(
#if NeedFunctionPrototypes
    XpStruct*        /* xpinfo */
#endif
);

void XpSetDisplay(
#if NeedFunctionPrototypes
    XpStruct*        /* xpinfo */,
    Display*        /* display */
#endif
);

void XpSetOutputMode(
#if NeedFunctionPrototypes
    XpStruct*        /* xpinfo */,
    int                /* outputmode */
#endif
);

void XpSetDimension(
#if NeedFunctionPrototypes
    XpStruct*        /* xpinfo */,
    unsigned int    /* width */,
    unsigned int    /* height */
#endif
);

void XpSetFontmap(
#if NeedFunctionPrototypes
    XpStruct*        /* xpinfo */,
    char*            /* fontmapfile */
#endif
);

void XpSetColormap(
#if NeedFunctionPrototypes
    XpStruct*        /* xpinfo */,
    Colormap        /* colormap */
#endif
);

void XpSetColorMode(
#if NeedFunctionPrototypes
    XpStruct*        /* xpinfo */,
    int /* Boolean            colormode */
#endif
);

void XpSetBackground(
#if NeedFunctionPrototypes
    XpStruct*        /* xpinfo */,
    Pixel            /* background */
#endif
);

void XpSetDrawBackground(
#if NeedFunctionPrototypes
    XpStruct*        /* xpinfo */,
    int  /*Boolean            drawbackground */
#endif
);

FILE* XpOpenOutput(
#if NeedFunctionPrototypes
    XpStruct*        /* xpinfo */,
    char*            /* outputfile */
#endif
);

void XpCloseOutput(
#if NeedFunctionPrototypes
    XpStruct*        /* xpinfo */
#endif
);

void XpPrintHeader(
#if NeedFunctionPrototypes
    XpStruct*        /* xpinfo */,
    XpPSStruct*        /* psinfo */
#endif
);

void XpPrintFooter(
#if NeedFunctionPrototypes
    XpStruct*        /* xpinfo */,
    XpPSStruct*      /* psinfo */
#endif
);

#endif /* _X2psapi_h */
