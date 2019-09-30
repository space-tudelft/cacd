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
#ifndef __DALICELL_H__
#define __DALICELL_H__

//============================================================================
//! Declaration of CCell, a base for special cell types.
/*! \file
    CCell is a base class for special cell types.

    \author Xander Burgerhout
    \date August 6, 2000        New CCell class for the QListView derived
                                implementation CSpreadSheetView.
*/
//============================================================================

// Qt includes
#include <qobject.h>
#include <qpainter.h>
#include <qlistview.h>

// Project includes
#include "src/spock/src/widgets/SpreadSheet/Cell.h"
#include "src/spock/src/widgets/SpreadSheet/DaliColorSelectDlg.h"

// Forward declarations
class CSpreadSheetView;
class CMultiTypeItem;

//! A CCell derived class with a dali color dialog as edit mode.
/*!
    The cell displays the current selected dali fill color and pattern. If the
    cell switches to edit mode, the dali colorpicker dialog is displayed and a
    new value can be chosen.

    \sa CCell, CMultiTypeItem, CSpreadSheetView, CDaliColorSelectDlg

    \author Xander Burgerhout
*/
class CDaliCell : public CCell
{
    Q_OBJECT

    private:
	CDaliStyle              m_style;

    public:
	CDaliCell(CMultiTypeItem* parent);
	virtual ~CDaliCell();

	virtual void            setValue(const QString& val);
	virtual CCell*          startEdit();
	virtual void            endEdit();
	virtual void            paintCell(QPainter *p, const QColorGroup& cg,
					int column, int width, int align);
};

#endif
