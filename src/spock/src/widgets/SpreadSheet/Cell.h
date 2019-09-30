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
#ifndef __CCELL_H__
#define __CCELL_H__

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
#include <qrect.h>
#include <qvalidator.h>

// STL includes
#include <vector>
#include <string>

using namespace std;

// Forward declarations
class CSpreadSheetView;
class CMultiTypeItem;

//! A base class for special cell types.
/*!
    \author Xander Burgerhout
*/
class CCell : public QObject
{
    Q_OBJECT

    private:
	CMultiTypeItem*         m_parent;
	QValidator*             m_validator;

    protected:
	QRect                   getCellRect();

    public:
	CCell(CMultiTypeItem* parent);
	virtual ~CCell();

	CMultiTypeItem*         parent();

	virtual CCell*          startEdit();
	virtual void            endEdit();
	virtual void            paintCell(QPainter *p, const QColorGroup& cg,
					int column, int width, int align);

	virtual QString         getValue();
	virtual void            setValue(const QString& value);
	virtual void            setValue(const vector<QString>& val);
	void                    setValue(const vector<string>& val); // convenience for above.
	virtual int             getIntValue();
	virtual void            setIntValue(int newVal);
	virtual void            setTxtValue(const string& val, int col);
	virtual int             column();

	virtual void            setValidator(QValidator* validator);
	virtual QValidator*     getValidator();

    signals:
	void                    valueChanged(int col); // CCell* point to this cell
};

#endif  //__CCELL_H__
