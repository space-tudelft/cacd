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
#include "src/spock/src/widgets/SpreadSheet/DaliCell.h"
#include "src/spock/src/helper/log/assert.h"

// Qt includes
#include <qglobal.h>
#include <qwidget.h>
#include <qcolordialog.h>
#include <qvalidator.h>
#include <qheader.h>

using namespace std;

//============================================================================
//! Default constructor
/*!
    \param parent   The CMultiTypeItem owning this cell.
    \sa CMultiTypeItem, CCell
 */
CDaliCell::CDaliCell(CMultiTypeItem* parent)
    : CCell(parent)
{
}

//============================================================================
//! Destructor
CDaliCell::~CDaliCell()
{
}

//============================================================================
//! Sets the dali style of this cell.
/*!
    \param val  The new style.

    The \a val paramter is parsed. It must be a string containing to integer
    values separated by a space. For example "1 1" is a valid dalistyle.

    The integers must be in a valid dalistyle range.
 */
void CDaliCell::setValue(const QString& val)
{
    QString color;
    QString fill;

    int splitpos = val.find(' ');
    color = val.left(splitpos);
    fill = val.right(val.length()-splitpos);

    int c = atoi(color.latin1());
    int f = atoi(fill.latin1());

    m_style.SetColor(c);
    m_style.SetFill(f);

    CCell::setValue(val);
    parent()->repaint();
}

// inherited documentation
CCell* CDaliCell::startEdit()
{
    debug("CDaliCell::startEdit() called...");

    CMultiTypeItem* par = parent();
    QListView* lv = par->listView();

    CDaliColorDlg* dlg = new CDaliColorDlg(lv);
    if (dlg->exec())
        m_style = dlg->getStyle();

    QString tmp;
    tmp.sprintf("%d %d", m_style.GetColor(), m_style.GetFill());
    CCell::setValue(tmp);

    debug("   done.");
    return this;
}

// inherited documentation
void CDaliCell::endEdit()
{
    debug("CDaliCell::endEdit() called...");
}

// inherited documentation
void CDaliCell::paintCell(QPainter *pPainter, const QColorGroup& cg, int /*column*/, int width, int /*align*/)
{
    // Set the correct colors
    QColor cellColor = cg.base();

    // Draw the cell
    int w = width;
    int h = parent()->height();

    pPainter->fillRect(0, 0, w-1, h-1, cellColor);

    pPainter->setPen(cg.background());
    pPainter->drawLine(-1, h-1, w, h-1);
    pPainter->drawLine(w-1, -1, w-1, h-1);

    CDaliPixmap* pm = new CDaliPixmap(w-2, h-2, m_style.GetColor(), m_style.GetFill());
    //pm->CreateDaliPixmap(w-4, h-4);

    if (!pm->isNull())
        pPainter->drawPixmap(1, 1, *pm);
}
