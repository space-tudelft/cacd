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
#if (XtSpecificationRelease > 4) /* X11R5 or higher has other includes (PvP) */
#include <X11/Xlibint.h>
#endif
#include <X11/Xatom.h>

#include "src/libX2ps/X2pslib.h"
#include "src/libX2ps/X2psapi.h"

/* Xlib functions */

static void update_bbox(Xpinfo,x1,y1,x2,y2)
    XpStruct* Xpinfo;
    int x1,y1,x2,y2;
{
    if(x1<x2) {
        if(x1<Xpinfo->llx) Xpinfo->llx = x1;
        if(x2>Xpinfo->width) Xpinfo->width = x2;
    }
    else {
        if(x2<Xpinfo->llx) Xpinfo->llx = x2;
        if(x1>Xpinfo->width) Xpinfo->width = x1;
    }
    if(y1<y2) {
        if(y1<Xpinfo->lly) Xpinfo->lly = y1;
        if(y2>Xpinfo->height) Xpinfo->height = y2;
    }
    else {
        if(y2<Xpinfo->lly) Xpinfo->lly = y2;
        if(y1>Xpinfo->height) Xpinfo->height = y1;
    }
#ifdef X2PS_DEBUG
    printf("update_bbox() : %3d %3d %3d %3d > %3d %3d %3d %3d\n",
           x1,y1,x2,y2,Xpinfo->llx, Xpinfo->lly,
           Xpinfo->width, Xpinfo->height);
    XpStructShow(Xpinfo);
#endif /* X2PS_DEBUG */
}

void PSSetLineWidth(Xpinfo, gc)
    XpStruct* Xpinfo;
    GC gc;
{
    unsigned long valuemask;
    XGCValues xgcvalues;

    valuemask = GCLineWidth;
    XGetGCValues(Xpinfo->display,gc,valuemask,&xgcvalues);
    fprintf(Xpinfo->PSfile,"%d setlinewidth ",xgcvalues.line_width);
}

void PSSetDash(Xpinfo, gc)
    XpStruct* Xpinfo;
    GC gc;
{
    unsigned long valuemask;
    XGCValues xgcvalues;
    typedef struct _XGC* XGC;
    XGC xgc = (XGC)gc;

    valuemask = GCLineStyle | GCDashOffset;
    XGetGCValues(Xpinfo->display,gc,valuemask,&xgcvalues);
    switch(xgcvalues.line_style) {
        case LineSolid:
        break;
        case LineOnOffDash:
        fprintf(Xpinfo->PSfile,"[%d %d] %d setdash ",xgc->values.dashes,
                xgc->values.dashes,xgcvalues.dash_offset);
        break;
        case LineDoubleDash:
        fprintf(Xpinfo->PSfile,"[%d %d] %d setdash ",xgc->values.dashes,
                xgc->values.dashes,xgcvalues.dash_offset);
        break;
    }
}

Boolean XpRGBCompare(xcolor1,xcolor2)
    XColor xcolor1;
    XColor xcolor2;
{
    return (xcolor1.red == xcolor2.red &&
            xcolor1.green == xcolor2.green &&
            xcolor1.blue == xcolor2.blue);
}

void PSSetRGBColor(Xpinfo, gc)
    XpStruct* Xpinfo;
    GC gc;
{
    unsigned long valuemask;
    XGCValues xgcvalues;
    XColor xcolor;
    XColor bg_xcolor;
    XColor exact_white,screen_white;
    float red,green,blue;

    valuemask = GCForeground | GCBackground;
    XGetGCValues(Xpinfo->display,gc,valuemask,&xgcvalues);

    /* Get background color */
    bg_xcolor.pixel = Xpinfo->background;
    /*x2psp->DrawWidget->core.background_pixel;*/
    XQueryColor(Xpinfo->display,Xpinfo->colormap,&bg_xcolor);

    /* get foreground color */
    xcolor.pixel = xgcvalues.foreground;
    XQueryColor(Xpinfo->display,Xpinfo->colormap,&xcolor);

    /* Lookup white to compare with back- and foreground color */
    XLookupColor(Xpinfo->display,Xpinfo->colormap,"white",&exact_white,
                 &screen_white);

    if(Xpinfo->drawbackground == False) {
	/* if no background is drawn, check if the draw color is white */
        if(XpRGBCompare(xcolor,screen_white)) {
            /* if so get background color to draw with */
            xcolor.pixel = xgcvalues.background;
            xcolor.red = bg_xcolor.red;
            xcolor.green = bg_xcolor.green;
            xcolor.blue = bg_xcolor.blue;
        }
        else {
	    /* or check if the draw color is the background color */
            if(XpRGBCompare(xcolor,bg_xcolor)) {
                /* if so set white to draw with */
                xcolor.pixel = screen_white.pixel;
                xcolor.red = screen_white.red;
                xcolor.green = screen_white.green;
                xcolor.blue = screen_white.blue;
            }
        }
    }

    /* color values is scaled between 0 and 65536 */
    if(Xpinfo->colormode == XpCOLOR) {
        red = ((float)xcolor.red)/65536;
        green = ((float)xcolor.green)/65536;
        blue = ((float)xcolor.blue)/65536;
    }
    else
	red = green = blue = 0; /* suppres uninitialized warning */

    if(Xpinfo->colormode == XpBW) {
        if(Xpinfo->drawbackground == False) {
            if(XpRGBCompare(xcolor,screen_white)) {
                /* color = white */
                red = green = blue = 1;
            }
            else {
                /* color = black */
                red = green = blue = 0;
            }
        }
        else {
            if(XpRGBCompare(xcolor,bg_xcolor)) {
                red = green = blue = 0;
            }
            else {
                red = green = blue = 1;
            }
        }
    }
    fprintf(Xpinfo->PSfile,"%1.2f %1.2f %1.2f setrgbcolor ",
            red,green,blue);
}

#define BYTES_PER_OUTPUT_LINE    24
/* The number of hex characters to print per line */

/*
 *  Proc:       XImageMaskToBitString
 *  Callers:    PixmapToColorPostScript
 *  Purpose:    Converts an XImage to a string of characters.  Each byte of
 *              the string represents 8 bits in the image (a number 0 -> 255).
 *  Params:     image - Pointer to the XImage.
 *              bitStringWidth - <Return> The number of bytes per line of the
 *                               image.
 *              bitStringLen - <Return> The number of bytes in the string.
 *              clip_image - Pointer to the XImage of the clipmask.
 *  Globals:    None
 *  Returns:    The bit string.
 */
unsigned char* XImageMaskToBitString(image, bitStringWidth, bitStringLen,
				     pixel, clip_image)
    XImage*       image;
    int*          bitStringWidth;
    int*          bitStringLen;
    unsigned long pixel;
    XImage*       clip_image;
{
    int                       y;
    unsigned char             *bitString;
    register int              x;
    register unsigned char    currentByte, currentBit;
    register unsigned char    *ptr;

    if( !image ) return( NULL );
    if( !bitStringLen ) return( NULL );

    /*
     * The XImage structure tells us how many bytes there are in one
     * scan line of the image.  Multiply this number by the height
     * (number of scan lines) in the image to get the total number of
     * bytes we will need for storage.
     */
    /* asume the clip_mask has the same size */
    (*bitStringWidth) = image->bytes_per_line;
    (*bitStringLen) = (*bitStringWidth)* image->height;

    /*
     * Make room for the string - initialize the bytes to zero.
     */

/*    bitString = (unsigned char *) XtCalloc((Cardinal)(*bitStringLen),
                                           (Cardinal)sizeof(unsigned char) );
*/
    bitString = (unsigned char *) calloc((Cardinal)(*bitStringLen),
                                         (Cardinal)sizeof(unsigned char) );
    if (!bitString) return(0);

    currentByte = 0;
    currentBit = 128;
    ptr = bitString;

    /*
     * Algorithm:
     *
     * Start at the MSB position in the current byte (currentBit) and
     * set it to a 1.  Starting at the lower left corner of the image
     * get the first pixel.  If the pixel equals the requested pixel,
     * set that bit in the current byte (currentByte) to a 1 by
     * or'ing.  Working left to right/bottom to top, move the bit
     * position (currentBit) to the right (by shifting).  Only 1 bit
     * position in currentBit will be 1 - the bit we are looking at.
     * Keep shifting and or'ing until the entire image is processed.
     * Any bit positions that are not set will be 0 by default since we
     * used calloc for memory allocation.
     */

    for( y=image->height - 1; y >= 0; y-- )
    {
        for( x=0; x < image->width; )
        {
	    if(!clip_image) {
		if(XGetPixel(image, x, y) == pixel)
		    currentByte |= currentBit;
	    }
	    else {
		if((XGetPixel(image, x, y) == pixel) &&
		   XGetPixel(clip_image, x, y))
		    currentByte |= currentBit;
	    }
            ++x;
            currentBit >>= 1;

            if( !(x & 7) )
            {
                *(ptr++) = currentByte;
                currentByte = 0;
                currentBit = 128;
            }
        }

        if( (x & 7) )
        {
            *(ptr++) = currentByte;
            currentByte = 0;
            currentBit = 128;
        }
    }

    return( bitString );
}

/***********************************************************/

/*
 *  Proc:    PixmapToColorPostScript
 *  Callers:    XpCopyArea()
 *  Purpose:    Converts a Pixmap image into a PostScript image and outputs
 *        the PostScript code that would be required to print the image.
 *        The code is not printable - it does not have a 'showpage' - it
 *        is meant to be "inlined" in other PostScript code.
 *  Params:    display - A display to work from.
 *        destFile - The destination file to output to.
 *        pixmap - The pixmap to convert.
 *        bps - Bits per sample of the pixmap (for PostScript).
 *        pixmapWidth - The actual width of the pixmap.
 *        pixmapHeight - The actual height of the pixmap.
 *        boundingBoxWidth - The width of the total area the pixmap
                           should occupy.
 *        boundingBoxHeight - The height of the total area the pixmap
                            should occupy.
 *  Globals:    None
 *  Returns:    0 if the conversion was successful.
 */
int PixmapToColorPostScript(display,destFile,colormap,clip_pixmap, pixmap,
			    pixmapX,pixmapY,pixmapWidth,pixmapHeight)
    Display* display;
    FILE* destFile;
    Colormap colormap;
    Pixmap clip_pixmap;
    Pixmap pixmap;
    unsigned int pixmapX;
    unsigned int pixmapY;
    unsigned int pixmapWidth;
    unsigned int pixmapHeight;
{
    int        c;
    int        currentByte;
    int        bitStringLen;
    int        bitStringWidth;
    XImage        *image;
    XImage        *clip_image = (XImage *)NULL;
    unsigned char    *ptr;
    unsigned char    *bitString;
    int x,y;
    unsigned long* pixel;
    XColor xcolor;
    int maskmax;
    unsigned long masknum;

    if( !display ) return( 1 );
    if( !destFile ) return( 1 );

    /*
     * Convert the Pixmap to an XImage, then convert the XImage into a
     * string of hexadecimal characters that can be output.
     */
    image = XGetImage(display, pixmap, pixmapX, pixmapY, pixmapWidth,
                      pixmapHeight, AllPlanes, XYPixmap );
    maskmax = (int)pow((double)2,(double)image->depth);
    pixel = (unsigned long*)calloc(maskmax,sizeof(unsigned long));

    if(clip_pixmap)
	clip_image = XGetImage(display, clip_pixmap, pixmapX, pixmapY,
			       pixmapWidth, pixmapHeight, AllPlanes, XYPixmap);
    fprintf( destFile, "    %% BeginImage\n" );
    for(y=0;y<image->height;y++) {
        for(x=0;x<image->width;x++) {
            masknum = XGetPixel(image, x, y);
#ifdef X2PS_DEBUG
	    printf("%2d",masknum);
#endif /* X2PS_DEBUG */
            if(!pixel[masknum]) {
                pixel[masknum] = 1;
                xcolor.pixel = masknum;
                XQueryColor(display,colormap,&xcolor);
                /* color values is scaled between 0 and 65536 */
                fprintf(destFile,"\t%1.2f %1.2f %1.2f setrgbcolor %% %lu\n",
                        ((float)xcolor.red)/65536,
                        ((float)xcolor.green)/65536,
                        ((float)xcolor.blue)/65536,masknum);
                bitString = XImageMaskToBitString(image,
                                                  &bitStringWidth,
                                                  &bitStringLen,
                                                  masknum,
						  clip_image);
                if( !bitString ) return( 1 );
                /*
                 * Print out the image as a block of hexadecimal characters.
                 */

                fprintf(destFile,"\t%u %u true [ %u 0 0 %u 0 0 ]\n",
                        pixmapWidth,pixmapHeight,pixmapWidth,pixmapHeight );
                fprintf( destFile, "\t{<");

                for(currentByte = 0, ptr = bitString ;
                    currentByte < bitStringLen ;
                    currentByte++, ptr++ )
                {
                    if(currentByte &&
                       !(currentByte % BYTES_PER_OUTPUT_LINE) )
                        fprintf (destFile, "\n\t\t");
                    c = (int)(*ptr);
                    if ( c < 0 ) c += 256;
                    fprintf (destFile, "%02x", c);
                }
                fprintf( destFile, ">} %% mask data\n\t\timagemask\n");
/*                XtFree( (char *)bitString);*/
                free( (char *)bitString);
            }
        }
#ifdef X2PS_DEBUG
	printf("\n");
#endif /* X2PS_DEBUG */
    }
    fprintf( destFile, "\t%% EndImage\n" );
    free((char*)pixel);
    XDestroyImage(image);
    if(clip_image)
	XDestroyImage(clip_image);

    return(0);
}

void lower(string)
    char* string;
{
    int i;

    for(i=0;i<strlen(string);i++)
        if((string[i]>='A')&&(string[i]<='Z'))
            string[i] = string[i] - ('A'-'a');
}

char* mapX2PSfont(Xpinfo,font_struct)
    XpStruct* Xpinfo;
    XFontStruct* font_struct;
{
    Display*   display = Xpinfo->display;
    FILE*      fontmapfile;
    char*      fontmapfilename;
    XpFontmap* xpfontmap = Xpinfo->xpfontmap;
    char       line[160];
    char       Xfont[80];
    char*      PSfont = (char*)calloc(80,sizeof(char));
    int        max_pr_num = 14;

    int   i,j,k,prop_num,fit;
    char  token[80];
    char* parse[40];
    char* Xfont_prop[15];

    Atom  atom;
    char* Xfnname;

    fit = 0;

    /* ask the server for the full name of the font */
    XGetFontProperty(font_struct,XA_FONT,&atom);
    Xfnname = XGetAtomName(display,atom);
    lower(Xfnname);

#ifdef X2PS_DEBUG
    printf("Xfnname = %s\n",Xfnname);
#endif /* X2PS_DEBUG */

    /* if no map exist, create XpFontmap */
    if(!xpfontmap)
        xpfontmap = Xpinfo->xpfontmap = (XpFontmap*)XpFontmapCreate();

    /* check if already mapped */
    if(xpfontmap->XLFD)
        if(!strcmp(Xfnname,xpfontmap->XLFD)) {
#ifdef X2PS_DEBUG
            printf("Font %s already mapped to %s\n",xpfontmap->XLFD,
                   xpfontmap->PSfont);
#endif /* X2PS_DEBUG */
            free(Xfnname);
            strcpy(PSfont,xpfontmap->PSfont);
            return PSfont;
        }

/* Parse the XLFD string Xfnname */
    i = 0;
    k = 0;
    if(Xfnname[i]=='-') {
        while(Xfnname[i]!='\0') {
            i++;
            j = 0;
            while((Xfnname[i]!='-')&&(Xfnname[i]!='\0'))
                token[j++] = Xfnname[i++];
            token[j] = '\0';
            Xfont_prop[k] = (char*)calloc(strlen(token)+1,sizeof(char));
            strcpy(Xfont_prop[k++],token);
        }
    }
    else {
        printf("Not the right XLFD syntax. fontname: %s\n",Xfnname);
        return strdup("");
/*        break; */ /* break while(!feof(fontmapfile)) { */
    }

    if(k!=max_pr_num) {
        printf("Not the right XLFD syntax. fontname: %s\n",Xfnname);
        return strdup("");
/*        break; */ /* break while(!feof(fontmapfile)) { */
    }

    fontmapfilename = strdup(Xpinfo->fontmapfile);
    fontmapfile = fopen(fontmapfilename,"r");
    if(!fontmapfile) {
        printf("Could not open fontmapfile: %s\n",fontmapfilename);
        for(prop_num=0;prop_num<max_pr_num;prop_num++) {
            free(Xfont_prop[prop_num]);
        }
        free(PSfont);
        free(fontmapfilename);
        return strdup("");
    }

    for(prop_num=0;prop_num<max_pr_num;prop_num++) {
        parse[prop_num] = NULL;
    }

    while(!feof(fontmapfile)) {
        while(!feof(fontmapfile)) {
            fscanf(fontmapfile,"%[^\n]\n",line);
            if(line[0]!='#')
                break;
        }

        if(feof(fontmapfile)&&(line[0]=='#'))
            break; /* break while(!feof(fontmapfile)) { */

        sscanf(line,"%[^,],%s\n",Xfont,PSfont);

        if(!strcmp(Xfont,"-*-*-*-*-*-*-*-*-*-*-*-*-*-*")) {
/*            printf("Couldn't map to any of the fonts, map to default\n");*/
            fit = 1;
            break; /* break while(!feof(fontmapfile)) { */
        }

/* Parse the XLFD string */
        i = 0;
        k = 0;
        if(Xfont[i]=='-') {
            while(Xfont[i]!='\0') {
                i++;
                j = 0;
                while((Xfont[i]!='-')&&(Xfont[i]!='\0'))
                    token[j++] = Xfont[i++];
                token[j] = '\0';
                if(parse[k])
                  free(parse[k]);
                parse[k] = (char*)calloc(strlen(token)+1,sizeof(char));
                strcpy(parse[k++],token);
            }
        }
        else {
            printf("Not the right syntax in fontmapfile. fontname: %s\n",
                   Xfont);
            break; /* break while(!feof(fontmapfile)) { */
        }

        if(k!=max_pr_num) {
            printf("Not the right syntax in fontmapfile. fontname: %s\n",
                   Xfont);
            break; /* break while(!feof(fontmapfile)) { */
        }

        fit = 1;

        for(prop_num=0;prop_num<max_pr_num;prop_num++) {
            if(strcmp(parse[prop_num],"*")) {
                if(strcmp(Xfont_prop[prop_num],parse[prop_num])) {
                    fit=0;
                    break; /* break for(prop_num=0;pro.... */
                }
            }
        }

        if(fit) {
#ifdef X2PS_DEBUG
            printf("found font: %s\n",Xfont);
#endif /* X2PS_DEBUG */
            break;
        }
    } /* end while(!feof(fontmapfile)) { */

    fclose(fontmapfile);

    if(fit && PSfont) {
        if(xpfontmap->XLFD)
            free(xpfontmap->XLFD);
        xpfontmap->XLFD = (char*)calloc(strlen(Xfnname)+1,sizeof(char));
        strcpy(xpfontmap->XLFD,Xfnname);
        if(xpfontmap->PSfont)
            free(xpfontmap->PSfont);
        xpfontmap->PSfont = (char*)calloc(strlen(PSfont)+1,sizeof(char));
        strcpy(xpfontmap->PSfont,PSfont);
#ifdef X2PS_DEBUG
        printf("Mapping to PSfont: %s\n",PSfont);
#endif /* X2PS_DEBUG */
    }
    else {
#ifdef X2PS_DEBUG
        printf("No PSfont for mapping: %s\n",Xfont);
#endif /* X2PS_DEBUG */
        free(PSfont);
        PSfont = strdup("");
    }
    for(prop_num=0;prop_num<max_pr_num;prop_num++) {
        free(parse[prop_num]);
        free(Xfont_prop[prop_num]);
    }
    XFree(Xfnname);
     free(fontmapfilename);

    return PSfont;
}

void XpClearArea(display, w, x, y, width, height, exposures)
    Display*        display;
    Window            w;
    int                x;
    int                y;
    unsigned int    width;
    unsigned int    height;
    Bool             exposures;
{
    XpStruct* Xpinfo = (XpStruct*)display;

    if(XpIsXpStruct(Xpinfo)) {
        switch(Xpinfo->flag) {
            case XpDISPLAY:
	    XClearArea(Xpinfo->display,w,x,y,width,height,exposures);
            return;
            case XpGETBBOX:
            break;
            case XpPS:
            case XpEPS:
            fprintf(Xpinfo->PSfile,"%% XpClearArea %d %d %u %u\n",
                    x, y, width, height);
        }
    }
    else
        XClearArea(display, w, x, y, width, height, exposures);
}

void XpClearWindow(display, w)
    Display*    display;
    Window        w;
{
    XpStruct* Xpinfo = (XpStruct*)display;

    if(XpIsXpStruct(Xpinfo)) {
        switch(Xpinfo->flag) {
            case XpDISPLAY:
	    XClearWindow(Xpinfo->display, w);
            return;
            case XpGETBBOX:
            break;
            case XpPS:
            case XpEPS:
            fprintf(Xpinfo->PSfile,"%% XpClearWindow\n");
        }
    }
    else
        XClearWindow(display, w);
}

void XpCopyArea(display, src, dest, gc, src_x, src_y, width, height,
           dest_x, dest_y)
    Display*        display;
    Drawable        src;
    Drawable        dest;
    GC                gc;
    int                src_x;
    int                src_y;
    unsigned int    width;
    unsigned int    height;
    int                dest_x;
    int                dest_y;
{
    XpStruct *Xpinfo = (XpStruct*)display;
    int x1,x2,y1,y2;
    typedef struct _XGC* XGC;
    XGC xgc = (XGC)gc;

    if(XpIsXpStruct(Xpinfo)) {
        switch(Xpinfo->flag) {
            case XpDISPLAY:
	    XCopyArea(Xpinfo->display, src, dest, gc, src_x,
                             src_y, width, height, dest_x, dest_y);
            return;
            case XpGETBBOX:
            x1 = dest_x;
            y1 = dest_y;
            x2 = dest_x + width;
            y2 = dest_y + height;
            update_bbox(Xpinfo, x1, y1, x2, y2);
            break;
            case XpPS:
            case XpEPS:
            fprintf(Xpinfo->PSfile,"%% XpCopyArea\n");
            fprintf(Xpinfo->PSfile,"/savematrix mtrx currentmatrix def\n");
            fprintf(Xpinfo->PSfile,"newpath %d %d translate %u -%u scale\n",
                    dest_x, dest_y+height, width, height);
            PixmapToColorPostScript(Xpinfo->display, Xpinfo->PSfile,
                                    Xpinfo->colormap, xgc->values.clip_mask,
                                    src, src_x, src_y, width, height);
            fprintf(Xpinfo->PSfile,"savematrix setmatrix\n");
        }
    }
    else
        XCopyArea(display, src, dest, gc, src_x, src_y, width, height,
                  dest_x, dest_y);
}

void XpDrawArc(display, d, gc, x, y,    width, height, angle1, angle2)
    Display*        display;
    Drawable        d;
    GC                gc;
    int                x;
    int                y;
    unsigned int    width;
    unsigned int    height;
    int                angle1;
    int                angle2;
{
    XpStruct* Xpinfo = (XpStruct*)display;
    int x1,x2,y1,y2;

    if(XpIsXpStruct(Xpinfo)) {
        switch(Xpinfo->flag) {
            case XpDISPLAY:
	    XDrawArc(Xpinfo->display, d, gc, x, y, width,
                            height, angle1, angle2);
            return;
            case XpGETBBOX:
            x1 = x;
            y1 = y;
            x2 = x + width;
            y2 = y + height;
            update_bbox(Xpinfo, x1, y1, x2, y2);
            break;
            case XpPS:
            case XpEPS:
            fprintf(Xpinfo->PSfile,"%% XpDrawArc\n");
            fprintf(Xpinfo->PSfile,"%d %d %u %u %d %d DrawArc ",
                    x, y, width, height, -angle1/64, -angle2/64);
            fprintf(Xpinfo->PSfile,"gsave ");
            PSSetLineWidth(Xpinfo,gc);
            PSSetRGBColor(Xpinfo,gc);
            PSSetDash(Xpinfo,gc);
            fprintf(Xpinfo->PSfile,"stroke grestore\n");
        }
    }
    else
        XDrawArc(display, d, gc, x, y, width, height, angle1, angle2);
}

#if NeedFunctionPrototypes
void XpDrawImageString(
    register Display* display,
    Drawable d,
    GC gc,
    int x,
    int y,
    const char* string,
    int length)
#else
void XpDrawImageString(display, d, gc, x, y, string, length)
    register Display* display;
    Drawable d;
    GC gc;
    int x, y;
    char* string;
    int length;
#endif
/*
void XpDrawImageString(display, d, gc, x, y, string, length)
    Display*        display;
    Drawable        d;
    GC                gc;
    int                x;
    int                y;
    char*            string;
    int                length;
*/
{
    XpStruct* Xpinfo = (XpStruct*)display;
    int x1,x2,y1,y2;
    int width,height;
    GContext graphics_context;
    XFontStruct* font_struct;
    unsigned long pixel_size;
    char* PSfont;
    char* substring;
    unsigned long valuemask;
    XColor back_xcolor;
    XGCValues xgcvalues;
    XColor xcolor,exact_named,screen_named;

    if(XpIsXpStruct(Xpinfo)) {
        switch(Xpinfo->flag) {
            case XpDISPLAY:
	    XDrawImageString(Xpinfo->display, d, gc, x, y, string, length);
            return;
            case XpGETBBOX:
            graphics_context = XGContextFromGC(gc);
            font_struct = XQueryFont(Xpinfo->display,graphics_context);
            width = XTextWidth(font_struct,string, length);
            height = font_struct->ascent + font_struct->descent - 1;
            x1 = x;
            y1 = y - height;
            x2 = x + width;
            y2 = y;
            update_bbox(Xpinfo,x1,y1,x2,y2);
            XFreeFontInfo(NULL,font_struct,1);
            break;
            case XpPS:
            case XpEPS:
            graphics_context = XGContextFromGC(gc);
            font_struct = XQueryFont(Xpinfo->display,graphics_context);
            XGetFontProperty(font_struct,
                             XInternAtom(Xpinfo->display,"PIXEL_SIZE",False),
                             &pixel_size);
            PSfont = mapX2PSfont(Xpinfo,font_struct);
            substring = (char*)calloc(length+1,sizeof(char));
            strncpy(substring,string,length);
            width = XTextWidth(font_struct,string, length);
            height = font_struct->ascent + font_struct->descent - 1;
            fprintf(Xpinfo->PSfile,"%% XpDrawImageString\n");
            fprintf(Xpinfo->PSfile,"%% Rectangle to clear background\n");
            fprintf(Xpinfo->PSfile,"%d %d %u %u DrawRectangle ",
                    x,y-height+2,width,height);

            valuemask = GCForeground | GCBackground;
            XGetGCValues(Xpinfo->display,gc,valuemask,&xgcvalues);

            if(Xpinfo->drawbackground == False)
            {
                back_xcolor.pixel = Xpinfo->background;
                XQueryColor(Xpinfo->display,Xpinfo->colormap,&back_xcolor);
                xcolor.pixel = xgcvalues.background;
                XQueryColor(Xpinfo->display,Xpinfo->colormap,&xcolor);
                if(XpRGBCompare(xcolor,back_xcolor)) {
                    XAllocNamedColor(Xpinfo->display,Xpinfo->colormap,"white",
                                     &screen_named,&exact_named);
                    xcolor.pixel = screen_named.pixel;
                    XQueryColor(Xpinfo->display,Xpinfo->colormap,&xcolor);
                }
                else {
                    if(Xpinfo->colormode == XpCOLOR)
                        xcolor.pixel = xgcvalues.background;
                    else
                        xcolor.pixel = screen_named.pixel;
                        XQueryColor(Xpinfo->display,Xpinfo->colormap,&xcolor);
                }
            }
            else {
                if(Xpinfo->colormode == XpCOLOR)
                    xcolor.pixel = xgcvalues.background;
                else {
                    XAllocNamedColor(Xpinfo->display,Xpinfo->colormap,"black",
                                     &screen_named,&exact_named);
                    xcolor.pixel = screen_named.pixel;
                }
                XQueryColor(Xpinfo->display,Xpinfo->colormap,&xcolor);
            }
            /* color values is scaled between 0 and 65536 */
            fprintf(Xpinfo->PSfile,"gsave %1.2f %1.2f %1.2f setrgbcolor fill stroke grestore\n",
                    ((float)xcolor.red)/65536,
                    ((float)xcolor.green)/65536,
                    ((float)xcolor.blue)/65536);
            fprintf(Xpinfo->PSfile,"/savematrix mtrx currentmatrix def\n");
            fprintf(Xpinfo->PSfile,"%d %d translate 1 -1 scale\n",x,y);
            if(PSfont)
		fprintf(Xpinfo->PSfile,"/%s findfont\n",PSfont);
            fprintf(Xpinfo->PSfile,"%lu scalefont setfont\n",pixel_size);
            fprintf(Xpinfo->PSfile,"0 0 moveto (%s) ",substring);
            PSSetRGBColor(Xpinfo,gc);
            PSSetDash(Xpinfo,gc);
            fprintf(Xpinfo->PSfile,"show\n");
            fprintf(Xpinfo->PSfile,"savematrix setmatrix\n");
            XFreeFontInfo(NULL,font_struct,1);
            if(PSfont)
		free(PSfont);
            free(substring);
        }
    }
    else
        XDrawImageString(display, d, gc, x, y, string, length);
}

void XpDrawLine(display, d, gc, x1, y1, x2, y2)
    Display*    display;
    Drawable    d;
    GC            gc;
    int            x1;
    int            x2;
    int            y1;
    int            y2;
{
    XpStruct* Xpinfo = (XpStruct*)display;

    if(XpIsXpStruct(Xpinfo))
        switch(Xpinfo->flag) {
            case XpDISPLAY:
	    XDrawLine(Xpinfo->display, d, gc, x1, y1, x2, y2);
            return;
            case XpGETBBOX:
            update_bbox(Xpinfo, x1, y1, x2, y2);
            break;
            case XpPS:
            case XpEPS:
            fprintf(Xpinfo->PSfile,"%% XpDrawLine\n");
            fprintf(Xpinfo->PSfile,"%d %d %d %d DrawLine ",x1,y1,x2,y2);
            fprintf(Xpinfo->PSfile,"gsave ");
            PSSetLineWidth(Xpinfo,gc);
            PSSetRGBColor(Xpinfo,gc);
            PSSetDash(Xpinfo,gc);
            fprintf(Xpinfo->PSfile,"stroke grestore\n");
        }
    else
        XDrawLine(display, d, gc, x1, y1, x2, y2);
}

void XpDrawLines(display, d, gc, points, npoints, mode)
    Display*    display;
    Drawable    d;
    GC            gc;
    XPoint*        points;
    int            npoints;
    int            mode;
{
    XpStruct* Xpinfo = (XpStruct*)display;
    int x1,x2,y1,y2;
    int i;

    if(XpIsXpStruct(Xpinfo)) {
        switch(Xpinfo->flag) {
            case XpDISPLAY:
	    XDrawLines(Xpinfo->display, d, gc, points, npoints, mode);
            return;
            case XpGETBBOX:
            if(mode==CoordModeOrigin) {
                for(i=0;i<npoints-1;i++) {
                    x1 = points[i].x;
                    y1 = points[i].y;
                    x2 = points[i+1].x;
                    y2 = points[i+1].y;
                    update_bbox(Xpinfo, x1, y1, x2, y2);
                }
            }
            if(mode==CoordModePrevious) {
                x1 = points[0].x;
                y1 = points[0].y;
                for(i=1;i<npoints;i++) {
                    x2 = x1 + points[i].x;
                    y2 = y1 + points[i].y;
                    update_bbox(Xpinfo, x1, y1, x2, y2);
                    x1 = x2;
                    y1 = y2;
                }
            }
            break;
            case XpPS:
            case XpEPS:
            fprintf(Xpinfo->PSfile,"%% XpDrawLines ");
            if(mode==CoordModeOrigin) {
                fprintf(Xpinfo->PSfile,"CoordModeOrigin\n");
                fprintf(Xpinfo->PSfile,"newpath %d %d moveto ",
                        points[0].x,points[0].y);
                for(i=1;i<npoints;i++)
                    fprintf(Xpinfo->PSfile,"%d %d lineto ",
                            points[i].x,points[i].y);
            }
            if(mode==CoordModePrevious) {
                fprintf(Xpinfo->PSfile,"CoordModePrevious\n");
                fprintf(Xpinfo->PSfile,"newpath %d %d moveto ",
                        points[0].x,points[0].y);
                for(i=1;i<npoints;i++)
                    fprintf(Xpinfo->PSfile,"%d %d rlineto ",
                            points[i].x,points[i].y);
            }
            fprintf(Xpinfo->PSfile,"gsave ");
            PSSetLineWidth(Xpinfo,gc);
            PSSetRGBColor(Xpinfo,gc);
            PSSetDash(Xpinfo,gc);
            fprintf(Xpinfo->PSfile,"stroke grestore\n");
        }
    }
    else
        XDrawLines(display, d, gc, points, npoints, mode);
}

void XpDrawRectangle(display, d, gc, x, y, width, height)
    Display*        display;
    Drawable        d;
    GC                gc;
    int                x;
    int                y;
    unsigned int    width;
    unsigned int    height;
{
    XpStruct* Xpinfo = (XpStruct*)display;
    int x1,x2,y1,y2;

    if(XpIsXpStruct(Xpinfo)) {
        switch(Xpinfo->flag) {
            case XpDISPLAY:
	    XDrawRectangle(Xpinfo->display, d, gc, x, y, width, height);
            return;
            case XpGETBBOX:
            x1 = x;
            x2 = x + width;
            y1 = y;
            y2 = y + height;
            update_bbox(Xpinfo, x1, y1, x2, y2);
            break;
            case XpPS:
            case XpEPS:
            fprintf(Xpinfo->PSfile,"%% XpDrawRectangle\n");
            fprintf(Xpinfo->PSfile,"%d %d %u %u DrawRectangle ",
                    x,y,width,height);
            fprintf(Xpinfo->PSfile,"gsave ");
            PSSetLineWidth(Xpinfo,gc);
            PSSetRGBColor(Xpinfo,gc);
            PSSetDash(Xpinfo,gc);
            fprintf(Xpinfo->PSfile,"stroke grestore\n");
        }
    }
    else
        XDrawRectangle(display, d, gc, x, y, width, height);
}

void XpDrawSegments(display, d, gc, segments, nsegments)
    Display*    display;
    Drawable    d;
    GC            gc;
    XSegment*    segments;
    int            nsegments;
{
    int i;
    XpStruct* Xpinfo = (XpStruct*)display;

    if(XpIsXpStruct(Xpinfo)) {
        switch(Xpinfo->flag) {
            case XpDISPLAY:
	    XDrawSegments(Xpinfo->display, d, gc, segments, nsegments);
            return;
            case XpGETBBOX:
            for(i=0;i<nsegments;i++) {
                update_bbox(Xpinfo, segments[i].x1, segments[i].y1,
                            segments[i].x2, segments[i].y2);
            }
            break;
            case XpPS:
            case XpEPS:
            fprintf(Xpinfo->PSfile,"%% XpDrawSegments\n");
            for(i=0;i<nsegments;i++) {
                fprintf(Xpinfo->PSfile,"%d %d %d %d DrawLine ",
                        segments[i].x1,segments[i].y1,segments[i].x2,
                        segments[i].y2);
                fprintf(Xpinfo->PSfile,"gsave ");
                PSSetLineWidth(Xpinfo,gc);
                PSSetRGBColor(Xpinfo,gc);
                PSSetDash(Xpinfo,gc);
                fprintf(Xpinfo->PSfile,"stroke grestore\n");
            }
        }
    }
    else
        XDrawSegments(display, d, gc, segments, nsegments);
}

#if NeedFunctionPrototypes
void XpDrawString(
    register Display* display,
    Drawable d,
    GC gc,
    int x,
    int y,
    const char* string,
    int length)
#else
void XpDrawString(display, d, gc, x, y, string, length)
    register Display* display;
    Drawable d;
    GC gc;
    int x, y;
    char* string;
    int length;
#endif
/*
void XpDrawString(display, d, gc, x, y, string, length)
    Display*    display;
    Drawable    d;
    GC            gc;
    int            x;
    int            y;
    char*        string;
    int            length;
*/
{
    XpStruct* Xpinfo = (XpStruct*)display;
    int x1,x2,y1,y2;
    int width,height;
    GContext graphics_context;
    XFontStruct* font_struct;
    unsigned long pixel_size;
    char* PSfont;
    char* substring;

    if(XpIsXpStruct(Xpinfo)) {
        switch(Xpinfo->flag) {
            case XpDISPLAY:
	    XDrawString(Xpinfo->display, d, gc, x, y, string, length);
            return;
            case XpGETBBOX:
            graphics_context = XGContextFromGC(gc);
            font_struct = XQueryFont(Xpinfo->display,graphics_context);
            width = XTextWidth(font_struct,string, length);
            height = font_struct->ascent + font_struct->descent - 1;
            x1 = x;
            y1 = y - height;
            x2 = x + width;
            y2 = y;
            update_bbox(Xpinfo,x1,y1,x2,y2);
            XFreeFontInfo(NULL,font_struct,1);
            break;
            case XpPS:
            case XpEPS:
            graphics_context = XGContextFromGC(gc);
            font_struct = XQueryFont(Xpinfo->display,graphics_context);
            XGetFontProperty(font_struct,
                             XInternAtom(Xpinfo->display,"PIXEL_SIZE",False),
                             &pixel_size);
            PSfont = mapX2PSfont(Xpinfo,font_struct);
            substring = (char*)calloc(length+1,sizeof(char));
            strncpy(substring,string,length);
            fprintf(Xpinfo->PSfile,"%% XpDrawString\n");
            fprintf(Xpinfo->PSfile,"/savematrix mtrx currentmatrix def\n");
            fprintf(Xpinfo->PSfile,"%d %d translate 1 -1 scale\n",x,y);
            if(PSfont)
		fprintf(Xpinfo->PSfile,"/%s findfont\n",PSfont);
            fprintf(Xpinfo->PSfile,"%ld scalefont setfont\n",pixel_size);
            fprintf(Xpinfo->PSfile,"0 0 moveto (%s) ",substring);
            PSSetRGBColor(Xpinfo,gc);
            PSSetDash(Xpinfo,gc);
            fprintf(Xpinfo->PSfile,"show\n");
            fprintf(Xpinfo->PSfile,"savematrix setmatrix\n");
            XFreeFontInfo(NULL,font_struct,1);
            if(PSfont)
		free(PSfont);
            free(substring);
        }
    }
    else
        XDrawString(display, d, gc, x, y, string, length);
}

void XpFillArc(display, d, gc, x, y, width, height, angle1, angle2)
    Display*        display;
    Drawable        d;
    GC                gc;
    int                x;
    int                y;
    unsigned int    width;
    unsigned int    height;
    int                angle1;
    int                angle2;
{
    XpStruct* Xpinfo = (XpStruct*)display;
    int x1,x2,y1,y2;

    if(XpIsXpStruct(Xpinfo)) {
        switch(Xpinfo->flag) {
            case XpDISPLAY:
	    XFillArc(Xpinfo->display, d, gc, x, y, width, height, angle1, angle2);
            return;
            case XpGETBBOX:
            x1 = x;
            y1 = y;
            x2 = x + width;
            y2 = y + height;
            update_bbox(Xpinfo, x1, y1, x2, y2);
            break;
            case XpPS:
            case XpEPS:
            fprintf(Xpinfo->PSfile,"%% XpDrawArc\n");
            fprintf(Xpinfo->PSfile,"%d %d %u %u %d %d DrawArc ",
                    x, y, width, height, -angle1/64, -angle2/64);
            fprintf(Xpinfo->PSfile,"gsave ");
            PSSetLineWidth(Xpinfo,gc);
            PSSetRGBColor(Xpinfo,gc);
            PSSetDash(Xpinfo,gc);
            fprintf(Xpinfo->PSfile,"fill stroke grestore\n");
        }
    }
    else
        XFillArc(display, d, gc, x, y, width, height, angle1, angle2);
}

void XpFillPolygon(display, d, gc, points, npoints, shape, mode)
    Display*        display;
    Drawable        d;
    GC                gc;
    XPoint*            points;
    int                npoints;
    int                shape;
    int                mode;
{
    XpStruct* Xpinfo = (XpStruct*)display;
    int x1,x2,y1,y2;
    int i;

    if(XpIsXpStruct(Xpinfo)) {
        switch(Xpinfo->flag) {
            case XpDISPLAY:
	    XFillPolygon(Xpinfo->display, d, gc, points, npoints, shape, mode);
            return;
            case XpGETBBOX:
            if(mode==CoordModeOrigin) {
                for(i=0;i<npoints-1;i++) {
                    x1 = points[i].x;
                    y1 = points[i].y;
                    x2 = points[i+1].x;
                    y2 = points[i+1].y;
                    update_bbox(Xpinfo, x1, y1, x2, y2);
                }
            }
            if(mode==CoordModePrevious) {
                x1 = points[0].x;
                y1 = points[0].y;
                for(i=1;i<npoints;i++) {
                    x2 = x1 + points[i].x;
                    y2 = y1 + points[i].y;
                    update_bbox(Xpinfo, x1, y1, x2, y2);
                    x1 = x2;
                    y1 = y2;
                }
            }
            break;
            case XpPS:
            case XpEPS:
            fprintf(Xpinfo->PSfile,"%% XpFillPolygon ");
            if(mode==CoordModeOrigin) {
                fprintf(Xpinfo->PSfile,"CoordModeOrigin\n");
                fprintf(Xpinfo->PSfile,"newpath %d %d moveto ",
                        points[0].x,points[0].y);
                for(i=1;i<npoints;i++)
                    fprintf(Xpinfo->PSfile,"%d %d lineto ",
                            points[i].x,points[i].y);
            }
            if(mode==CoordModePrevious) {
                fprintf(Xpinfo->PSfile,"CoordModePrevious\n");
                fprintf(Xpinfo->PSfile,"newpath %d %d moveto ",
                        points[0].x,points[0].y);
                for(i=1;i<npoints;i++)
                    fprintf(Xpinfo->PSfile,"%d %d rlineto ",
                            points[i].x,points[i].y);
            }
            fprintf(Xpinfo->PSfile,"gsave ");
            PSSetLineWidth(Xpinfo,gc);
            PSSetRGBColor(Xpinfo,gc);
            PSSetDash(Xpinfo,gc);
            fprintf(Xpinfo->PSfile,"fill stroke grestore\n");
        }
    }
    else
        XFillPolygon(display, d, gc, points, npoints, shape, mode);
}

void XpFillRectangle(display, d, gc, x, y, width, height)
    Display*        display;
    Drawable        d;
    GC                gc;
    int                x;
    int                y;
    unsigned int    width;
    unsigned int    height;
{
    XpStruct* Xpinfo = (XpStruct*)display;
    int x1,x2,y1,y2;

    if(XpIsXpStruct(Xpinfo)) {
        x1 = x;
        x2 = x + width;
        y1 = y;
        y2 = y + height;
        switch(Xpinfo->flag) {
            case XpDISPLAY:
	    XFillRectangle(Xpinfo->display, d, gc, x, y, width, height);
            return;
            case XpGETBBOX:
            update_bbox(Xpinfo, x1, y1, x2, y2);
            break;
            case XpPS:
            case XpEPS:
            fprintf(Xpinfo->PSfile,"%% XpFillRectangle\n");
            fprintf(Xpinfo->PSfile,"%d %d %u %u DrawRectangle ",
                    x,y,width,height);
            fprintf(Xpinfo->PSfile,"gsave ");
            PSSetLineWidth(Xpinfo,gc);
            PSSetRGBColor(Xpinfo,gc);
            fprintf(Xpinfo->PSfile,"fill stroke grestore\n");
        }
    }
    else
        XFillRectangle(display, d, gc, x, y, width, height);
}

void XpFillRectangles(display, d, gc, rectangles, nrectangles)
    Display*        display;
    Drawable        d;
    GC                gc;
    XRectangle*        rectangles;
    int                nrectangles;
{
    XpStruct* Xpinfo = (XpStruct*)display;
    int i;

    if(XpIsXpStruct(Xpinfo)) {
        switch(Xpinfo->flag) {
            case XpDISPLAY:
	    XFillRectangles(Xpinfo->display, d, gc, rectangles, nrectangles);
            return;
            case XpGETBBOX:
            break;
            case XpPS:
            case XpEPS:
            fprintf(Xpinfo->PSfile,"%% XpFillRectangles\n");
            for(i=0;i<nrectangles;i++) {
                fprintf(Xpinfo->PSfile,"%d %d %u %u DrawRectangle ",
                        rectangles[i].x,rectangles[i].y,rectangles[i].width,
                        rectangles[i].height);
                fprintf(Xpinfo->PSfile,"gsave ");
                PSSetLineWidth(Xpinfo,gc);
                PSSetRGBColor(Xpinfo,gc);
                fprintf(Xpinfo->PSfile,"fill stroke grestore\n");
            }
        }
    }
    else
        XFillRectangles(display, d, gc, rectangles, nrectangles);
}

