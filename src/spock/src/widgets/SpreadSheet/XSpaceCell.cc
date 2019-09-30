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

// Project includes
#include "src/spock/src/widgets/SpreadSheet/SpreadSheet.h"
#include "src/spock/src/widgets/SpreadSheet/XSpaceCell.h"
#include "src/spock/src/helper/log/assert.h"

// Qt includes
#include <qglobal.h>
#include <qwidget.h>
#include <qcolordialog.h>
#include <qvalidator.h>
#include <qheader.h>

using namespace std;

//============================================================================
//! Constructor
/*!
    \param parent  The CMultiTypeItem this cell belongs to.

    \sa CMultiTypeItem, CSpreadSheetView
 */
CXSpaceCell::CXSpaceCell(CMultiTypeItem* parent)
    : CCell(parent)
{
}

//============================================================================
//! Destructor
CXSpaceCell::~CXSpaceCell()
{
}

// inherited documentation
CCell* CXSpaceCell::startEdit()
{
    debug("CXSpaceCell::startEdit() called...");

    CMultiTypeItem* par = parent();
    QListView* lv = par->listView();

    QColor initial( getValue() );
    QColor color = QColorDialog::getColor(initial, lv);
    if (color.isValid()) setValue(color.name());
    debug("   done.");
    return this;
}

// inherited documentation
void CXSpaceCell::endEdit()
{
    debug("CXSpaceCell::endEdit() called...");
}

// inherited documentation
void CXSpaceCell::paintCell(QPainter *pPainter, const QColorGroup& cg, int /*column*/, int width, int /*align*/)
{
    // Set the correct colors
    QColor cellColor = cg.base();
    QColor fillColor = QColor(getValue());

    // Draw the cell
    int w = width;
    int h = parent()->height();

    pPainter->fillRect(0, 0, w-1, h-1, cellColor);
    pPainter->fillRect(2, 2, w-3, h-3, fillColor);

    pPainter->setPen(cg.background());
    pPainter->drawLine(-1, h-1, w, h-1);
    pPainter->drawLine(w-1, -1, w-1, h-1);
}
