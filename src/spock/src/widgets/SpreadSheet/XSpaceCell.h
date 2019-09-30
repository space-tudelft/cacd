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
#ifndef __XSPACECELL_H__
#define __XSPACECELL_H__

//============================================================================
//! Declaration of CCell, a base for special cell types.
/*! \file
    CCell is a base class for special cell types.

    \author Xander Burgerhout
*/
//============================================================================

// Qt includes
#include <qobject.h>
#include <qpainter.h>
#include <qlistview.h>
#include <qcolordialog.h>

// Project includes
#include "src/spock/src/widgets/SpreadSheet/Cell.h"

// Forward declarations
class CSpreadSheetView;
class CMultiTypeItem;

//! A CCell derived class with a color dialog as edit mode.
/*!
    The edit mode of this cell displays the QColorDialog. A color can then be
    selected by the user. The selected color is displayed in the cell's area.

    \image html colordlg.jpg
    \image latex colordlg.eps "The QColorDialog dialog box" width=10cm

    \sa CCell, CMultiTypeItem, CSpreadSheetView

    \author Xander Burgerhout
*/
class CXSpaceCell : public CCell
{
    Q_OBJECT

    public:
	CXSpaceCell(CMultiTypeItem* parent);
	virtual ~CXSpaceCell();

	virtual CCell*          startEdit();
	virtual void            endEdit();
	virtual void            paintCell(QPainter *p, const QColorGroup& cg,
					int column, int width, int align);
};

#endif
