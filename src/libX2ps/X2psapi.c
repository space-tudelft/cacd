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
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <malloc.h>
#include <memory.h>
#include <math.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include "src/libX2ps/X2pslib.h"
#include "src/libX2ps/X2psapi.h"

extern Boolean XpRGBCompare _FARGS_((XColor xcolor1, XColor xcolor2));

/* uncomment next line for debug information */
/*#define DEBUGME*/

#ifdef DEBUGME
#define IFDEBUG(A) A
#else
#define IFDEBUG(A)
#endif

XpFontmap* XpFontmapCreate()
{
    XpFontmap* xpfontmap = (XpFontmap*)calloc(sizeof(XpFontmap), 1);
    IFDEBUG(printf("XpFontmapCreate()\n"));

    xpfontmap->XLFD = NULL;
    xpfontmap->PSfont = NULL;

    return xpfontmap;
}

void XpFontmapShow(xpfontmap)
    XpFontmap* xpfontmap;
{
    if(xpfontmap)
        printf("XpFontmap => XLFD: %s, PSfont: %s\n", xpfontmap->XLFD,
               xpfontmap->PSfont);
    else
        printf("XpFontmap: NULL\n");
}

void XpFontmapFree(xpfontmap)
    XpFontmap* xpfontmap;
{
    IFDEBUG(printf("XpFontmapFree()\n"));

    if(xpfontmap) {
        if(xpfontmap->XLFD)
            free(xpfontmap->XLFD);
        if(xpfontmap->PSfont)
            free(xpfontmap->PSfont);
        free((char*)xpfontmap);
    }
}

XpFontmap* XpFontmapDup(xpfontmap)
    XpFontmap* xpfontmap;
{
    XpFontmap* newxpfontmap = XpFontmapCreate();
    IFDEBUG(printf("XpFontmapDup()\n"));

    if(xpfontmap->XLFD)
        newxpfontmap->XLFD = strdup(xpfontmap->XLFD);
    if(xpfontmap->PSfont)
        newxpfontmap->PSfont = strdup(xpfontmap->PSfont);

    return newxpfontmap;
}

void XpStructShow(xpinfo)
    XpStruct* xpinfo;
{
    printf("XpStructShow() ");
    if(xpinfo) {
        printf("id: %s, outputmode: %d\n", xpinfo->id, xpinfo->flag);
        printf("outputfile: %s, file: %p\n",
               xpinfo->outputfile, xpinfo->PSfile);
        printf("bbox: %d %d %d %d\n",xpinfo->llx, xpinfo->lly,
               xpinfo->width, xpinfo->height);
        printf("fontmapfile: %s\n",xpinfo->fontmapfile);
        XpFontmapShow(xpinfo->xpfontmap);
    }
    else
        printf("NULL\n");
}

XpStruct* XpStructCreate()
{
    XpStruct* xpinfo;

    IFDEBUG(printf("XpStructCreate()\n"));
    xpinfo = (XpStruct*)calloc(sizeof(XpStruct), 1);
    xpinfo->id = XPSTRUCTID;
    xpinfo->flag = XpDISPLAY;
    xpinfo->outputfile = NULL;
    xpinfo->PSfile = NULL;
    xpinfo->llx = xpinfo->lly = xpinfo->width = xpinfo->height = 0;
    xpinfo->fontmapfile = NULL;
    xpinfo->xpfontmap = (XpFontmap*)NULL;
    xpinfo->colormap = (Colormap)NULL;
    xpinfo->drawbackground = False;
    xpinfo->background = 0;
    IFDEBUG(XpStructShow(xpinfo));
    return xpinfo;
}

void XpStructFree(xpinfo)
    XpStruct* xpinfo;
{
    IFDEBUG(printf("XpStructFree()\n"));
    if(xpinfo) {
        if(xpinfo->outputfile)
            free(xpinfo->outputfile);
        if(xpinfo->fontmapfile)
            free(xpinfo->fontmapfile);
        if(xpinfo->xpfontmap)
            XpFontmapFree(xpinfo->xpfontmap);
        free((char*)xpinfo);
    }
}

Boolean XpIsXpStruct(xpinfo)
    XpStruct* xpinfo;
{
    if(xpinfo->id) {
        return(!memcmp(xpinfo->id,XPSTRUCTID,strlen(XPSTRUCTID)));
    }
    else
        return False;
}

XpStruct* XpStructDup(xpinfo)
    XpStruct* xpinfo;
{
    XpStruct* newxpinfo = XpStructCreate();

    IFDEBUG(printf("XpStructDup()\n");
            XpStructShow(xpinfo));

    newxpinfo->display = xpinfo->display;
    newxpinfo->flag = xpinfo->flag;
    if(xpinfo->outputfile)
        newxpinfo->outputfile = strdup(xpinfo->outputfile);
    newxpinfo->PSfile = xpinfo->PSfile;
    newxpinfo->llx = xpinfo->llx;
    newxpinfo->lly = xpinfo->lly;
    newxpinfo->width = xpinfo->width;
    newxpinfo->height = xpinfo->height;
    if(xpinfo->fontmapfile)
        newxpinfo->fontmapfile = strdup(xpinfo->fontmapfile);
    if(xpinfo->xpfontmap)
        newxpinfo->xpfontmap = XpFontmapDup(xpinfo->xpfontmap);
    newxpinfo->colormap = xpinfo->colormap;
    newxpinfo->colormode = xpinfo->colormode;
    newxpinfo->drawbackground = xpinfo->drawbackground;
    IFDEBUG(XpStructShow(newxpinfo));
    return newxpinfo;
}

void XpSetDisplay(xpinfo, display)
    XpStruct* xpinfo;
    Display* display;
{
    xpinfo->display = display;
}

void XpSetOutputMode(xpinfo, outputmode)
    XpStruct* xpinfo;
    int outputmode;
{
    xpinfo->flag = outputmode;
}

void XpSetDimension(xpinfo, width, height)
    XpStruct* xpinfo;
    unsigned int width, height;
{
    xpinfo->width = width;
    xpinfo->height = height;
}

void XpSetFontmap(xpinfo, fontmapfile)
    XpStruct* xpinfo;
    char* fontmapfile;
{
    if(fontmapfile) {
        if(xpinfo->fontmapfile)
            free(xpinfo->fontmapfile);
        xpinfo->fontmapfile = strdup(fontmapfile);
    }
}

void XpSetColormap(xpinfo, colormap)
    XpStruct* xpinfo;
    Colormap colormap;
{
    xpinfo->colormap = colormap;
}

void XpSetColorMode(xpinfo, colormode)
    XpStruct* xpinfo;
    Boolean colormode;
{
    xpinfo->colormode = colormode;
}

void XpSetBackground(xpinfo, background)
    XpStruct* xpinfo;
    Pixel background;
{
    xpinfo->background = background;
}

void XpSetDrawBackground(xpinfo, drawbackground)
    XpStruct* xpinfo;
    Boolean drawbackground;
{
    xpinfo->drawbackground = drawbackground;
}

FILE* XpOpenOutput(xpinfo, outputfile)
    XpStruct* xpinfo;
    char* outputfile;
{
    if(outputfile) {
        if(xpinfo->outputfile)
            free(xpinfo->outputfile);
        xpinfo->outputfile = strdup(outputfile);
    }

    if((xpinfo->PSfile == NULL) && (xpinfo->outputfile)) {
        xpinfo->PSfile = fopen(xpinfo->outputfile,"w");
        return xpinfo->PSfile;
    }
    else
    {
        return NULL;
    }
}

void XpCloseOutput(xpinfo)
    XpStruct* xpinfo;
{
    if(xpinfo->PSfile) {
        fclose(xpinfo->PSfile);
        xpinfo->PSfile = NULL;
    }
}

void XpPrintProlog(xpinfo)
    XpStruct* xpinfo;
{
    if(xpinfo->PSfile) {
        fprintf(xpinfo->PSfile,"%%%%BeginProlog\n");
        fprintf(xpinfo->PSfile,"/$XpDict 200 dict def\n");
        fprintf(xpinfo->PSfile,"$XpDict begin\n");
        fprintf(xpinfo->PSfile,"$XpDict /mtrx matrix put\n");
        fprintf(xpinfo->PSfile,"/savematrix mtrx currentmatrix def\n");

        fprintf(xpinfo->PSfile,"/DrawArc {\n");
        fprintf(xpinfo->PSfile,"    /deltaangle exch def\n");
        fprintf(xpinfo->PSfile,"    /startangle exch def\n");
        fprintf(xpinfo->PSfile,"    /height exch def\n");
        fprintf(xpinfo->PSfile,"    /width exch def\n");
        fprintf(xpinfo->PSfile,"    /y exch def\n");
        fprintf(xpinfo->PSfile,"    /x exch def\n");
        fprintf(xpinfo->PSfile,"    /checkangles deltaangle 0 lt {/startangle startangle deltaangle add def /deltaangle deltaangle neg def} if pop\n");
        fprintf(xpinfo->PSfile,"    /endangle deltaangle startangle add def\n");
        fprintf(xpinfo->PSfile,"    /height height 2 div def\n");
        fprintf(xpinfo->PSfile,"    /width width 2 div def\n");
        fprintf(xpinfo->PSfile,"    /y y height add def\n");
        fprintf(xpinfo->PSfile,"    /x x width add def\n");
        fprintf(xpinfo->PSfile,"    /savematrix mtrx currentmatrix def\n");
        fprintf(xpinfo->PSfile,"    newpath x y translate width height scale 0 0 1 startangle endangle arc\n");
        fprintf(xpinfo->PSfile,"    savematrix setmatrix\n");
        fprintf(xpinfo->PSfile,"    } def\n");

        fprintf(xpinfo->PSfile,"/DrawLine {\n");
        fprintf(xpinfo->PSfile,"    /y2 exch def\n");
        fprintf(xpinfo->PSfile,"    /x2 exch def\n");
        fprintf(xpinfo->PSfile,"    /y1 exch def\n");
        fprintf(xpinfo->PSfile,"    /x1 exch def\n");
        fprintf(xpinfo->PSfile,"    newpath x1 y1 moveto x2 y2 lineto\n");
        fprintf(xpinfo->PSfile,"    } def\n");

        fprintf(xpinfo->PSfile,"/DrawRectangle {\n");
        fprintf(xpinfo->PSfile,"    /height exch def\n");
        fprintf(xpinfo->PSfile,"    /width exch def\n");
        fprintf(xpinfo->PSfile,"    /y exch def\n");
        fprintf(xpinfo->PSfile,"    /x exch def\n");
        fprintf(xpinfo->PSfile,"    newpath x y moveto width 0 rlineto 0 height ");
        fprintf(xpinfo->PSfile,"rlineto 0 width sub 0 rlineto closepath\n");
        fprintf(xpinfo->PSfile,"    } def\n");

        fprintf(xpinfo->PSfile,"end\n");
        fprintf(xpinfo->PSfile,"/$XpBegin {$XpDict begin /$XpEnteredState save def} def\n/");
        fprintf(xpinfo->PSfile,"$XpEnd {$XpEnteredState restore end} def\n");
        fprintf(xpinfo->PSfile,"%%%%EndProlog\n");
    }
}

void XpPrintHeader(xpinfo, psinfo)
    XpStruct* xpinfo;
    XpPSStruct* psinfo;
{
    float scale,xscale,yscale;
    int picturewidth, pictureheight;
    XColor exact_white,screen_white;
    XColor xcolor;
    float red,green,blue;

    if(psinfo->format == XpPS)
        fprintf(xpinfo->PSfile,"%%!PS-Adobe-3.0\n");
    else
        fprintf(xpinfo->PSfile,"%%!PS-Adobe-3.0 EPSF-3.0\n");

    if(psinfo->orientation == XpPORTRAIT)
        fprintf(xpinfo->PSfile,"%%%%Orientation: Portrait\n");
    else
        fprintf(xpinfo->PSfile,"%%%%Orientation: Landscape\n");

    fprintf(xpinfo->PSfile,"%%%%BoundingBox: %d %d %d %d\n",
            0, 0, xpinfo->width, xpinfo->height);

    fprintf(xpinfo->PSfile,"%%%%Title: %s\n",xpinfo->outputfile);

    fprintf(xpinfo->PSfile,"%%%%EndComments\n");

    XpPrintProlog(xpinfo);

/* next code makes picture fit papersizes */
    fprintf(xpinfo->PSfile,"%%%%BeginSetup\n");
    scale = 1;
    picturewidth = (int)((psinfo->paperwidth-2*psinfo->paperwidthoffset)*psinfo->dpi);
    pictureheight =    (int)((psinfo->paperheight-2*psinfo->paperheightoffset)*psinfo->dpi);

    if(psinfo->format == XpPS) {
        if(psinfo->orientation == XpPORTRAIT) {
            if((xpinfo->width > picturewidth) || (xpinfo->height > pictureheight)) {
                xscale = (float)picturewidth/xpinfo->width;
                yscale = (float)pictureheight/xpinfo->height;
                if(xscale < yscale)
                    scale = xscale;
                else
                    scale = yscale;
            }
        }
        else { /* XpLANDSCAPE */
            if((xpinfo->width > pictureheight) ||
               (xpinfo->height > picturewidth)) {
                xscale = (float)pictureheight/xpinfo->width;
                yscale = (float)picturewidth/xpinfo->height;
                if(xscale < yscale)
                    scale = xscale;
                else
                    scale = yscale;
            }
        }
    }

    if(psinfo->format == XpPS) {
        if(psinfo->orientation == XpPORTRAIT)
            fprintf(xpinfo->PSfile,"%d %d translate\n",
                    (picturewidth
                     - (int)((float)xpinfo->width*scale))/2
                    + (int)(psinfo->paperwidthoffset * psinfo->dpi),
                    (pictureheight
                     - (int)((float)xpinfo->height*scale))/2
                    + (int)(psinfo->paperheightoffset * psinfo->dpi));
        else { /* XpLANDSCAPE */
            fprintf(xpinfo->PSfile,"%d %d translate\n",
                    (picturewidth
                     - (int)((float)xpinfo->height*scale))/2
                    + (int)(psinfo->paperwidthoffset * psinfo->dpi),
                    (pictureheight
                     - (int)((float)xpinfo->width*scale))/2
                    + (int)(psinfo->paperheightoffset * psinfo->dpi));
            fprintf(xpinfo->PSfile,"90 rotate\n");
        }
    }

    if((psinfo->format == XpEPS) || (psinfo->orientation == XpPORTRAIT))
        fprintf(xpinfo->PSfile,"0 %d translate\n",
                (int)((float)xpinfo->height * scale));
    fprintf(xpinfo->PSfile,"%f -%f scale\n",scale,scale);

    fprintf(xpinfo->PSfile,"%%%%EndSetup\n");
/* picture begin */
    fprintf(xpinfo->PSfile,"$XpBegin\n");

    /* flag = 2 will generate the PostScript */
    XpSetOutputMode(xpinfo,XpPS);

    /* set clippath and clip */
    fprintf(xpinfo->PSfile,"%% Set clippath\n");
    fprintf(xpinfo->PSfile,"0 0 %d %d DrawRectangle clip\n",xpinfo->width,
            xpinfo->height);

    /* to fill the background if needed..... */
    if(xpinfo->drawbackground && xpinfo->display != NULL) {
        fprintf(xpinfo->PSfile,"%% FillBackground\n");
        fprintf(xpinfo->PSfile,"%d %d %u %u DrawRectangle ",
                0, 0, xpinfo->width, xpinfo->height);
        fprintf(xpinfo->PSfile,"gsave ");
        xcolor.pixel = xpinfo->background;
        XQueryColor(xpinfo->display,xpinfo->colormap,&xcolor);
        if(xpinfo->colormode == XpCOLOR) {
            red = ((float)xcolor.red)/65536;
            green = ((float)xcolor.green)/65536;
            blue = ((float)xcolor.blue)/65536;
        }
        else {
            XLookupColor(xpinfo->display,xpinfo->colormap,"white",&exact_white,
                         &screen_white);
            if(XpRGBCompare(xcolor,screen_white)) {
                /* color = white */
                red = green = blue = 1;
            }
            else {
                /* color = black */
                red = green = blue = 0;
            }
        }
        fprintf(xpinfo->PSfile,"%1.2f %1.2f %1.2f setrgbcolor ",
                red,green,blue);
        fprintf(xpinfo->PSfile,"fill stroke grestore\n");
    }
    fprintf(xpinfo->PSfile,"%%%%BeginDrawing\n");
}

void XpPrintFooter(xpinfo, psinfo)
    XpStruct* xpinfo;
    XpPSStruct* psinfo;
{
    fprintf(xpinfo->PSfile,"%%%%EndDrawing\n");
    fprintf(xpinfo->PSfile,"$XpEnd\n");

    if(psinfo->format == XpPS)
        fprintf(xpinfo->PSfile,"showpage\n");

    fprintf(xpinfo->PSfile,"%%%%EndDocument\n");
}
