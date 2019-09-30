/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
 *	Arjan van Genderen
 *	Simon de Graaf
 *	Nick van der Meijs
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

/*
 * MenuBox Widget
 *
 * Public and Private definitions for MenuBox widget
 */
#ifndef _MenuBoxP_h
#define _MenuBoxP_h

/*
 * General Description:
 *
 * A MenuBox can be used to contain Menu Buttons. It should be the child
 * widget of a menuShell, and its children should be menu buttons,
 * although this is not enforced. It is the same as a box, except that
 * it makes all of its managed children the same width.
 */

/* Parameters:
 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 background	     Background		pixel		White
 border		     BorderColor	pixel		Black
 borderWidth	     BorderWidth	int		1
 destroyCallback     Callback		Pointer		NULL
 hSpace 	     HSpace		int		4
 height		     Height		int		0
 mappedWhenManaged   MappedWhenManaged	Boolean		True
 vSpace 	     VSpace		int		4
 width		     Width		int		0
 x		     Position		int		0
 y		     Position		int		0
*/

/* Class record constants */
extern WidgetClass menuboxWidgetClass;

#include <X11/Xaw/BoxP.h>

/************************************
 *
 *  Class structure
 *
 ***********************************/

/* New fields for the MenuBox widget class record */
typedef struct {int empty;} MenuBoxClassPart;

/* Full class record declaration */
typedef struct _MenuBoxClassRec {
    CoreClassPart	core_class;
    CompositeClassPart  composite_class;
    BoxClassPart	box_class;
    MenuBoxClassPart    menu_box_class;
} MenuBoxClassRec, *MenuBoxWidgetClass;

extern MenuBoxClassRec menuboxClassRec;

/***************************************
 *
 *  Instance (widget) structure
 *
 **************************************/

/* New fields for the MenuBox widget record */
typedef struct {
/* private */
	int does_nothing;
} MenuBoxPart;

/* full widget declaration */

typedef struct _MenuBoxRec {
    CorePart	    core;
    CompositePart   composite;
    BoxPart 	    box;
    MenuBoxPart     menu_box;
} MenuBoxRec, *MenuBoxWidget;

#endif /* _MenuBoxP_h */
