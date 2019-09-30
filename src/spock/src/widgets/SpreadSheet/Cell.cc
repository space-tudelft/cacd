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
#include "src/spock/src/widgets/SpreadSheet/Cell.h"
#include "src/spock/src/helper/log/assert.h"

// Qt includes
#include <qglobal.h>
#include <qwidget.h>
#include <qlineedit.h>
#include <qheader.h>
#include <qvalidator.h>

using namespace std;

//============================================================================
//! Default constructor
/*!
    \param parent The CMultiTypeItem owning this cell.
 */
CCell::CCell(CMultiTypeItem* parent)
{
    ASSERT(parent != 0);
    m_parent = parent;
    m_validator = 0;
}

//============================================================================
//! Default destructor
CCell::~CCell()
{
}

//============================================================================
//! Retrieves the coordinates of this cells outer rectangle.
/*!
    \return The coordinates of this cells outer rectangle.
 */
QRect CCell::getCellRect()
{
    CMultiTypeItem* par = parent();
    QListView* lv = par->listView();

    int x = 2;
    for (int i=0; i<column(); ++i)
        x += lv->columnWidth(i);

    int width  = lv->columnWidth(column());
    int height = par->height() + 1;
    int y = parent()->itemPos() + lv->header()->height() + 1;

    QRect ret;
    ret.setLeft(x);
    ret.setTop(y);
    ret.setWidth(width);
    ret.setHeight(height);
    return ret;
}

//============================================================================
//! Returns this cell's owner.
/*!
    \return This cell's owner.
 */
CMultiTypeItem* CCell::parent()
{
    return m_parent;
}

//============================================================================
//! Returns the value of this cell.
/*!
    \return The value of this cell.
    \sa setValue(), setIntValue(), getIntValue()
 */
QString CCell::getValue()
{
    QString retval = m_parent->text(column());
    if (retval.isNull()) retval = "";
    return retval;
}

//============================================================================
//! Returns the integer value of this cell.
/*!
    \return The integer value of this cell.
    \sa setValue(), getValue(), setIntValue()
  */
int CCell::getIntValue()
{
    return (atoi(getValue().latin1()));
}

//============================================================================
//! Sets the value of this cell
/*!
    \param val The new value
    \sa getValue(), getIntValue(), setIntValue()
 */
string oldValue = "";
string newValue = "";
void CCell::setValue(const QString& val)
{
    QString old = m_parent->text(column());
    if (old.isNull()) old = "";
    if (val != old) {
	int col = column();
	oldValue = old.latin1();
	newValue = val.latin1();
	m_parent->setText(col, val);
	emit valueChanged(col);
    }
}

void CCell::setTxtValue(const string& val, int col)
{
	m_parent->setText(col, val.c_str());
}

//============================================================================
//! Sets the value of this cell
/*!
    \param val The new vector of values
    \sa getValue(), getIntValue(), setIntValue()
 */
void CCell::setValue(const vector<QString>& value)
{
    if (value.size()>0) setValue(value[0]);
}

//============================================================================
//! Sets the value of this cell
/*!
    \param val The new vector of values
    \sa getValue(), getIntValue(), setIntValue()
 */
void CCell::setValue(const vector<string>& value)
{
    vector<QString> tmp;
    for (unsigned int i=0; i<value.size(); ++i)
        tmp.push_back(value[i].c_str());
    setValue(tmp);
}

//============================================================================
//! Sets the integer value of this cell
/*!
    \param val The new vector of values
    \sa getValue(), getIntValue(), setValue()
 */
void CCell::setIntValue(int newVal)
{
    int col = column();
    QString buf;
    buf.sprintf("%d", newVal);
    m_parent->setText(col, buf);
    emit valueChanged(col);
}

//============================================================================
//! Starts edit mode on this cell
CCell* CCell::startEdit()
{
    return this;
}

//============================================================================
//! Ends edit mode on this cell
void CCell::endEdit()
{
}

//============================================================================
//! Draws this cell.
void CCell::paintCell(QPainter *pPainter, const QColorGroup& cg, int column, int width, int align)
{
    // Set the correct colors
    QColor textColor = cg.text();
    QColor cellColor = cg.base();

    if (m_parent->isSelected()) {
        textColor = cg.highlightedText();
        cellColor = cg.highlight();
    }

    // Draw the cell
    int w = width;
    int h = m_parent->height();

    pPainter->fillRect(0, 0, w-1, h-1, cellColor);
    pPainter->setPen(textColor);
    pPainter->drawText(1, 1, w-1, h-1, align,  m_parent->text(column));

    pPainter->setPen(cg.background());
    pPainter->drawLine(-1, h-1, w, h-1);
    pPainter->drawLine(w-1, -1, w-1, h-1);
}

//============================================================================
//! Returns the column number of this cell
int CCell::column()
{
    ASSERT(m_parent->findColumn(this) != -1);   // changed parent's m_cells[] behind our back....
    return m_parent->findColumn(this);          // Did you remove but not destroy this object?
   // A CCell cannot be used safely once removed from its CMultiTypeItem context.
   // Also, if you just created a new cell and the column for the cell has not yet been added
   // and you call a setValue() method, this assert will occur as well.
}

void CCell::setValidator(QValidator* validator)
{
    m_validator = validator;
}

QValidator* CCell::getValidator()
{
    return m_validator;
}
