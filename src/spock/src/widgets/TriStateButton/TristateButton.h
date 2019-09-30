/*
 * ISC License
 *
 * Copyright (C) 2000-2018 by
 *	Xander Burgerhout
 *	Simon de Graaf
 *	N.P. van der Meijs
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
#ifndef __TRISTATEBUTTON_H__
#define __TRISTATEBUTTON_H__

// Qt includes
#include <qbutton.h>

//! A tristate button that works with three colors.
/*!
    This button is used by the CConditionListDlg to display and edit the
    presen/absent/don't care states of the layers.

    The three states are color coded as follows:
        * present - green - Qt state: On
        * absent - red - Qt state: Off
        * don't care - white - Qt state: NoChange

    \sa CConditionListDlg

    \author Xander Burgerhout
*/
class CTristateButton : public QButton
{
    public:
	CTristateButton(QWidget* parent, const char* name=0);
	CTristateButton(ToggleState state, QWidget* parent, const char* name=0);

	virtual void        setPixmap(const QPixmap&);
	virtual void        setText(const QString&);
	virtual void        changeState(ToggleState newState);

    protected:
	virtual void        drawButton(QPainter* painter);
	virtual void        drawButtonLabel(QPainter* painter);
};

#endif //__TRISTATEBUTTON_H__
