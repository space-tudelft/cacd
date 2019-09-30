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
#ifndef __COMBOCELL_H__
#define __COMBOCELL_H__

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
#include <qcombobox.h>

#include "src/spock/src/widgets/SpreadSheet/Cell.h"

class CSpreadSheetView;
class CMultiTypeItem;

//! A CCell derived class with a combobox widget as edit mode.
/*!
    The edit mode can be either a dropdown or a combobox, depending
    on the readWrite parameter given to the constructor.

    The setIntValue() and getIntValue() are overriden to allow the use of
    the combobox indices to select values.

    The possible combobox values can be set with the vectored variant of
    the setValue() method.

    \sa CCell, CMultiTypeItem, CSpreadSheetView

    \author Xander Burgerhout
*/
class CComboCell : public CCell
{
    Q_OBJECT

    private:
	int                     m_default;
	QString			m_defval;
	vector<QString>         m_values;

	QComboBox*              m_combo;
	bool                    m_readWrite;

	QWidget*                m_parkFocus;
	QWidget*                m_prevCombo;

	void                    fillCombo();

    private slots:
	void                    onGainedFocus();

    public:
	CComboCell(CMultiTypeItem* parent, bool readWrite=false);
	virtual ~CComboCell();

	virtual void            setAsDefault(int def);
	virtual void		setDefault(const QString& val);
	virtual void            setIntValue(int val);
	virtual void            setValue(const vector<QString>& val);
	virtual int             getIntValue();

	virtual CCell*          startEdit();
	virtual void            endEdit();

    signals:
	void                    gainedFocus(CComboCell*);
};

#endif  //__COMBOCELL_H__
