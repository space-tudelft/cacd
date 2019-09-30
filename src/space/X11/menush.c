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
 * Code for menuShell widget
 */

#define MENUSHELL

#include "src/space/include/config.h"

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/CommandP.h>

#include "src/space/X11/menush.h"
#include "src/space/auxil/portable.h"

#ifdef __cplusplus
extern "C" {
#endif
static void Initialize (Widget request, Widget neww);
static void _GetLocation (Widget w);
static void Select (Widget w, XEvent *event, String *params, Cardinal *num);
static CommandWidget _Search_Select (MenuShellWidget w);
static void Setup (Widget w, XEvent *event, String *params, Cardinal *num);
#ifdef __cplusplus
}
#endif

static XtActionsRec actionsList[] =
{
	{ "select", Select },
	{ "setup", Setup },
};

static char  defaultTranslations[] =
       	"<BtnUp>: MenuPopdown() select()\n\
	<Map>: setup()\n\
	<Leave>:MenuPopdown()";


static Boolean defPutback = TRUE;
static XtJustify defMenuUnder = XtJustifyCenter;
static Dimension defMenuVSpace = 2;

#define offset(field) XtOffset(MenuShellWidget, field)

static XtResource resources[] = {

	{ XtNputbackCursor, XtCPutbackCursor, XtRBoolean,
	    sizeof(Boolean), offset(menu.putback_cursor), XtRBoolean,
	    (caddr_t) &defPutback },
	{ XtNmenuUnderCursor, XtCMenuUnderCursor, XtRJustify,
	    sizeof(XtJustify), offset(menu.menu_under_cursor), XtRJustify,
	    (caddr_t) &defMenuUnder },
	{ XtNmenuVSpace, XtCMenuVSpace, XtRDimension,
	    sizeof(Dimension), offset(menu.menu_vspace), XtRDimension,
	    (caddr_t)&defMenuVSpace },
};

#undef offset

MenuShellClassRec menuShellClassRec = {
  {
    /* superclass         */    (WidgetClass) &overrideShellClassRec,
    /* class_name         */    "MenuShell",
    /* size               */    sizeof(MenuShellRec),
    /* Class Initializer  */	NULL,
    /* class_part_initialize*/	NULL,
    /* Class init'ed ?    */	FALSE,
    /* initialize         */    (XtInitProc)Initialize,
    /* initialize_notify  */	NULL,
    /* realize            */    XtInheritRealize,
    /* actions            */    actionsList,
    /* num_actions        */    XtNumber(actionsList),
    /* resources          */    resources,
    /* resource_count     */	XtNumber(resources),
    /* xrm_class          */    NULLQUARK,
    /* compress_motion    */    FALSE,
    /* compress_exposure  */    TRUE,
    /* compress_enterleave*/ 	FALSE,
    /* visible_interest   */    FALSE,
    /* destroy            */    NULL,
    /* resize             */    XtInheritResize,
    /* expose             */    NULL,
    /* set_values         */    NULL,
    /* set_values_hook    */	NULL,
    /* set_values_almost  */	XtInheritSetValuesAlmost,
    /* get_values_hook    */	NULL,
    /* accept_focus       */    NULL,
    /* intrinsics version */	XtVersion,
    /* callback offsets   */    NULL,
    /* translation table  */    defaultTranslations,
    /* query geometry     */    NULL,
    /* display_accelerator*/	XtInheritDisplayAccelerator,
    /* extension	  */	NULL,
  },{
    /* geometry_manager   */    XtInheritGeometryManager,
    /* change_managed     */    XtInheritChangeManaged,
    /* insert_child	  */	XtInheritInsertChild,
    /* delete_child	  */	XtInheritDeleteChild,
  }, {
/* Shell Class Part */
   /* not used */               0,
  }, {
/* Override Shell Class Part */
    /* not used           */    0,
  }, {
/* Menu Shell Class Part */
    /* not used           */    0,
 },
};

WidgetClass menuShellWidgetClass = (WidgetClass) (&menuShellClassRec);

/*
 * Set popup_child_proc to address of procedure which puts the menu under
 * the cursor.  popup_child_proc is called by _XtPopup, so the cursor location
 * will be found every time the menu comes up.
 */

static void Initialize (Widget request, Widget neww)
{
    MenuShellWidget msw = (MenuShellWidget) neww;
    msw->shell.create_popup_child_proc = _GetLocation;
}

/*
 * Find out where the pointer is and put the menu there.
 * Geometry request should always be satisfied since we are an override shell.
 */
static void _GetLocation (Widget w)
{
    MenuShellWidget msw = (MenuShellWidget) w;
    Window root, child;
    int x_root, y_root, x_win, y_win;
    unsigned int mask;
    XtWidgetGeometry request;

    XQueryPointer( XtDisplay( msw ), XtWindow( msw ), &root, &child,
		  &x_root, &y_root, &x_win, &y_win, &mask);

    request.request_mode = CWX | CWY;
    switch (msw->menu.menu_under_cursor) {
    case XtJustifyLeft:
	    request.x = x_root - 20;
	    break;
    case XtJustifyCenter:
	    request.x = x_root - msw->core.width/2;
	    break;
    case XtJustifyRight:
	    request.x = x_root - msw->core.width;
	    break;
    default:
	    XtError("wrong type of menu justify.\n");
	    break;
    }
    request.y = y_root -30;
    XtMakeGeometryRequest((Widget)msw, &request, (XtWidgetGeometry *)NULL);
}

/*
 * Call SearchSelect to call callbacks, and put cursor back in original
 * position if putback_cursor is set.
 */
static void Select (Widget w, XEvent *event, String *params, Cardinal *num)
{
    MenuShellWidget msw = (MenuShellWidget) w;

    (void)_Search_Select(msw);

    if (msw->menu.putback_cursor)
	    XWarpPointer( XtDisplay( msw ), None, XtWindow( msw ),
			 0,0,0,0, msw->menu.cursor_x, msw->menu.cursor_y);
}

/*
 * Perform a depth-first search of the widget tree starting at the menuShell
 * to find the child menuButton which has been set, then call its callbacks.
 */
static CommandWidget _Search_Select (MenuShellWidget w)
{
    int j;
    CommandWidget mbw;

    /* only search children of composite widgets */

    if (XtIsSubclass((Widget)w, compositeWidgetClass)) {
	    CompositeRec *cr = (CompositeRec *) w;
	    for (j = 0; j < cr->composite.num_children; j++)
		    if ((mbw = _Search_Select((MenuShellWidget)
				       (cr->composite.children[j]))) != NULL)
			    return mbw;

    /* call callbacks of menu Button children that are set */

    } else if (XtIsSubclass((Widget)w, commandWidgetClass)) {
	    mbw = (CommandWidget) w;
	    if (mbw->command.set) {
		    XtCallCallbacks((Widget)mbw, XtNcallback, (caddr_t) NULL);
		    return mbw;
	    }
    }
    return NULL;
}

/*
 * Need this to save pointer loc in case putback cursor is set.
 */
static void Setup (Widget w, XEvent *event, String *params, Cardinal *num)
{
    Window root, child;
    int x_root, y_root;
    MenuShellWidget msw = (MenuShellWidget) w;
    unsigned int mask;
    int cursor_x, cursor_y;

    XQueryPointer( XtDisplay( w ), XtWindow( w ), &root, &child,
		  &x_root, &y_root, &cursor_x, &cursor_y, &mask);

    msw->menu.cursor_x = cursor_x;
    msw->menu.cursor_y = cursor_y;
}
