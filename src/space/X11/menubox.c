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
 * MenuBox composite widget
 *
 * Code for MenuBox widget
 */

#include "src/space/include/config.h"

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>

#define AssignMax(x, y)   if ((y) > (x)) x = (y)

#include "src/space/X11/menubox.h"
#include "src/space/auxil/portable.h"

/* local operations */
#ifdef __cplusplus
extern "C" {
#endif
static void ChangeManaged (Widget w);
#ifdef __cplusplus
}
#endif

static Dimension defVSpace = 1;

static XtResource resources[] = {
	{ XtNvSpace, XtCVSpace, XtRDimension, sizeof(Dimension),
		  XtOffset(MenuBoxWidget, box.v_space), XtRDimension,
	          (caddr_t) &defVSpace },
};

MenuBoxClassRec menuboxClassRec = {
  {
/* core_class fields      */
    /* superclass         */    (WidgetClass) &boxClassRec,
    /* class_name         */    "MenuBox",
    /* widget_size        */    sizeof(MenuBoxRec),
    /* class_initialize   */    NULL,
    /* class_part_init    */	NULL,
    /* class_inited       */	FALSE,
    /* initialize         */    NULL,
    /* initialize_hook    */	NULL,
    /* realize            */    XtInheritRealize,
    /* actions            */    NULL,
    /* num_actions	  */	0,
    /* resources          */    resources,
    /* num_resources      */    XtNumber(resources),
    /* xrm_class          */    NULLQUARK,
    /* compress_motion	  */	TRUE,
    /* compress_exposure  */	TRUE,
    /* compress_enterleave*/	TRUE,
    /* visible_interest   */    FALSE,
    /* destroy            */    NULL,
    /* resize             */    XtInheritResize,
    /* expose             */    NULL,
    /* set_values         */    NULL,
    /* set_values_hook    */	NULL,
    /* set_values_almost  */    XtInheritSetValuesAlmost,
    /* get_values_hook    */	NULL,
    /* accept_focus       */    NULL,
    /* version            */	XtVersion,
    /* callback_private   */    NULL,
    /* tm_table           */    NULL,
    /* query_geometry     */	XtInheritQueryGeometry,
    /* display_accelerator*/	XtInheritDisplayAccelerator,
    /* extension	  */	NULL,
  },{
/* composite_class fields */
    /* geometry_manager   */    XtInheritGeometryManager,
    /* change_managed     */    ChangeManaged,
    /* insert_child	  */	XtInheritInsertChild,
    /* delete_child	  */	XtInheritDeleteChild,
  },{
/* Box class fields */
    /* empty		  */	0,
  },{
/* MenuBox class fields */
   /* empty               */    0,
  },
};

WidgetClass menuboxWidgetClass = (WidgetClass)&menuboxClassRec;

/*
 * Private Routines
 */

/*
 * This procedure makes sure that all the boxes are the same width and height;
 * Could check first that children are subclass of labelWidget, but don't bother.
 */
static void ChangeManaged (Widget w)
{
    int j;
    Dimension width = 0;
    Dimension height = 0;
    MenuBoxWidget mbw = (MenuBoxWidget) w;
    WidgetClass wc;
    XtWidgetProc wpr;
    Widget  child;
    XtWidgetGeometry intended, preferred;

    /* get max width and heigth of current children */
    width = 0;
    height = 0;
    intended.request_mode = 0;

    for (j = 0; j < mbw->composite.num_children; j++) {
	child = mbw->composite.children[j];
	if (child->core.managed) {
		XtQueryGeometry(child, &intended, &preferred);
		AssignMax(width, preferred.width);
		AssignMax(height, preferred.height);
	}
    }

    /* make any one smaller than max width or height larger */
    for (j = 0; j < mbw->composite.num_children; j++) {
	child = mbw->composite.children[j];
	if (child->core.width != width
	||  child->core.height != height)
		XtResizeWidget(child, width, height, child->core.border_width);
    }

    /* call superclass changed managed to do layout */
    wc = XtSuperclass(w);
    wpr = ((CompositeWidgetClass) wc)->composite_class.change_managed;
    (*wpr)(w);
}
