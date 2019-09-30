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
#ifndef __CONDITIONLISTCELL_H__
#define __CONDITIONLISTCELL_H__

// Qt includes
#include <qobject.h>
#include <qpainter.h>
#include <qlistview.h>

// Project includes
#include "src/spock/src/widgets/SpreadSheet/Cell.h"

// Forward declarations
class CSpreadSheetView;
class CMultiTypeItem;

//! A CCell derived spreadsheet cell that represents a condition list.
/*!
    The edit mode of this widget is the conditionlist editor, CConditionListDlg.
    The layer names are set with the setValue() method that has been
    reimplemented from CCell. Optionally, the layer descriptions can be set with
    the setComments() method.

    Depending on the \a extended paramenter of the constructor a conditionlist
    with one or three columns is created.

    \sa CCell, CMultiTypeItem, CSpreadSheetView, CConditionListDlg

    \author Xander Burgerhout
*/
class CConditionListCell : public CCell
{
    Q_OBJECT
    private:
	vector<string>          m_layers;
	vector<string>          m_layerComments;
	bool                    m_extended;
	QString			m_defval;

    public:
	CConditionListCell(CMultiTypeItem* parent, bool extended = true);
	virtual ~CConditionListCell();

	virtual CCell*          startEdit();
	virtual void            endEdit();
	virtual void		setDefault(const QString& val);
	virtual void            setValue(const vector<QString>& values);
	virtual void            setComments(const vector<string>& comments);
	vector<string>          getComments();
};

#endif
