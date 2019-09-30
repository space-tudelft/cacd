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
#ifndef __EDITCELL_H__
#define __EDITCELL_H__

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
#include <qlineedit.h>

// Project includes
#include "src/spock/src/widgets/SpreadSheet/Cell.h"

// Forward declarations
class CSpreadSheetView;
class CMultiTypeItem;
class CComponent;

//! A CCell derived class with an edit widget as edit mode.
/*!
    The CEditCell class implements the normal editable spreadsheet cell.
    Edit mode brings up a QLineEdit widget. Validation is done depending on
    the exact column type as specified by the associated CComponent object.

    \sa CCell, CMultiTypeItem, CSpreadSheetView

    \author Xander Burgerhout
*/
class CEditCell : public CCell
{
    Q_OBJECT

    private:
	QLineEdit*              m_edit;
	QString			m_defval;
	QWidget*                m_parkFocus;
	QWidget*                m_prevEdit;
	CComponent*	m_comp;

    public:
	CEditCell(CMultiTypeItem* parent, CComponent* comp);
	virtual ~CEditCell();

	virtual void		setDefault(const QString& val);
	virtual CCell*          startEdit();
	virtual void            endEdit();
};

#endif  //__CCELL_H__
