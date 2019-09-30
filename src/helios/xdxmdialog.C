/*
 * ISC License
 *
 * Copyright (C) 1995-2018 by
 *	Xianfeng Ni
 *	Ulrich Geigenmuller
 *	Simon de Graaf
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

#include "src/helios/xdclass.h"
#include <Xm/DialogS.h>

xd_XmDialog_c::xd_XmDialog_c()
{
    xd_childwidget=0;
    return;
}

Widget xd_XmDialog_c::xd_getchildwidget()
{
    if (!xd_childwidget)
    {
	Cardinal numchildren;
	GetValue(XmNnumChildren, &numchildren);
	if (numchildren!=0)
	{
	    WidgetList children;
	    GetValue(XmNchildren, &children);
	    xd_childwidget=*children;
	}
    }
    return xd_childwidget;
}

void xd_XmDialog_c::xd_show()
{
    if (!_xd_rootwidget) // Nothing to show
	return;
    if (!xd_getchildwidget()) // Nothing to show
	return;
    XtManageChild(xd_childwidget);
    return;
}

void xd_XmDialog_c::xd_hide()
{
    if (!_xd_rootwidget) // Nothing to hide
	return;
    if (!xd_getchildwidget()) // Nothing to hide
	return;
    XtUnmanageChild(xd_childwidget);
    return;
}

void xd_XmDialog_c::Raise()
{
    (void) XRaiseWindow(XtDisplay(_xd_rootwidget), XtWindow(_xd_rootwidget));
    return;
}
