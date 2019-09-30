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
 * MenuShell Widget
 *
 * Public and Private definitions for MenuShell widget
 */
#ifndef _XtMenuShellPrivate_h
#define _XtMenuShellPrivate_h

#include <X11/Shell.h>

/* Resources:
Name		Class		RepType		Default Value
----		-----		-------		-------------
menuUnderCursor MenuUnderCursor	Boolean		True
putbackCursor	PutbackCursor	Boolean		False
*/

/* MenuShell specific atoms
 */
#define XtNputbackCursor   "putbackCursor"
#define XtCPutbackCursor   "PutbackCursor"
#define XtNmenuUnderCursor "menuUnderCursor"
#define XtCMenuUnderCursor "MenuUnderCursor"
#define XtNmenuVSpace      "menuVSpace"
#define XtCMenuVSpace      "MenuVSpace"

#ifndef MENUSHELL
extern WidgetClass menuShellWidgetClass;
#endif

#include <X11/Xaw/Command.h>
#include <X11/ShellP.h>

/************************************
 *
 *  Class structure
 *
 ***********************************/

/* New fields for the MenuShell widget class record */

typedef struct {int foo;} MenuShellClassPart;

/* Full class record declaration */

typedef struct _MenuShellClassRec {
  	CoreClassPart      core_class;
	CompositeClassPart composite_class;
	ShellClassPart  shell_class;
	OverrideShellClassPart  override_shell_class;
	MenuShellClassPart menu_shell_class;
} MenuShellClassRec, *MenuShellClass;

extern MenuShellClassRec menuShellClassRec;

/***************************************
 *
 *  Instance (widget) structure
 *
 **************************************/

/* New fields for the menu shell widget */

typedef struct _MenuShell_Part {
	Boolean putback_cursor;
	XtJustify menu_under_cursor;
	int menu_vspace;
/* private */
	Position cursor_x, cursor_y;
} MenuShellPart;

/* Full widget declaration */

typedef  struct {
	CorePart 	core;
	CompositePart 	composite;
	ShellPart 	shell;
	OverrideShellPart override;
	MenuShellPart menu;
} MenuShellRec, *MenuShellWidget;

#endif /* _XtMenuShellPrivate_h */
