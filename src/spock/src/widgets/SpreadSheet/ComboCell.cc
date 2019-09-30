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
#include "src/spock/src/widgets/SpreadSheet/ComboCell.h"
#include "src/spock/src/helper/log/assert.h"

// Qt includes
#include <qglobal.h>
#include <qwidget.h>
#include <qlineedit.h>
#include <qvalidator.h>
#include <qheader.h>

using namespace std;

//============================================================================
//! Default constructor
/*!
    \param parent       The CMultiTypeItem this cell belongs to
    \param readWrite    If true, the edit mode is a combobox. If false a dropdown.
 */
CComboCell::CComboCell(CMultiTypeItem* parent, bool readWrite /*=false*/)
    : CCell(parent)
{
    m_default = -1;
    m_defval  = "";
    m_combo = 0;
    m_readWrite = readWrite;

    m_prevCombo = 0;
    m_parkFocus = new QWidget(0);
    m_parkFocus->hide();
}

//============================================================================
//! Destructor
/*!
    Frees any memory still allocated.
 */
CComboCell::~CComboCell()
{
    if (m_prevCombo != 0) {
        delete m_prevCombo;
        m_prevCombo = 0;
    }
    if (m_combo != 0) {
        delete m_combo;
        m_combo = 0;
    }
    if (m_parkFocus != 0) {
        delete m_parkFocus;
        m_parkFocus = 0;
    }
}

//============================================================================
//! Sets a default value for the cell.
/*!
    \param def Index of the value to set as default.

    \a def is the location of the default value in the vector of values.
 */
void CComboCell::setAsDefault(int def)
{
    m_default = def;
}

void CComboCell::setDefault(const QString& val)
{
    m_defval = val;
}

//============================================================================
//! Sets the cell to a new value.
/*!
    The new value is the one at location \a val in the vector of values.
 */
void CComboCell::setIntValue(int val)
{
    if (val>=0 && (unsigned)val<m_values.size())
        CCell::setValue(m_values[val]);
}

//============================================================================
//! Sets the combobox entries and selects the default.
/*!
    This method has been reimplemented to allow setting the combobox values.
    If a default has been specified, the default value is set as the
    current value.

    \param val The new combobox values.
 */
void CComboCell::setValue(const vector<QString>& val)
{
    m_values = val;
//  if (m_default>=0 && (unsigned)m_default<m_values.size())
//      CCell::setValue(m_values[m_default]);
}

//============================================================================
//! Returns the location of the current cell value in the vector of values.
/*!
    \return The location of the current cell value in the vector of values.
 */
int CComboCell::getIntValue()
{
    QString txt = getValue();
    for (int i=0; (unsigned)i < m_values.size(); ++i)
        if (m_values[i] == txt)
            return i;
    return m_default;
}

//============================================================================
//! Initializes the edit mode widget with the correct values.
/*!
    After filling the combobox/dropdown widget, the default value is selected.
 */
void CComboCell::fillCombo()
{
    if (m_combo != 0) {
        unsigned int i, j, k;
	int nr = -1;
	QString txt = getValue();
        m_combo->clear();
        for (k=i=0; i<m_values.size(); ++i) {
	    for (j=0; j<i; ++j) {
		if (m_values[j] == m_values[i]) break;
	    }
	    if (j == i) {
		m_combo->insertItem(m_values[i]);
		if (m_values[i] == txt) nr = k;
		++k;
	    }
	}
	if (nr == -1) { // txt not found
	    m_combo->insertItem(txt);
	    nr = k;
	}
	m_combo->setCurrentItem(nr);
    }
}

// documentation is inherited
CCell* CComboCell::startEdit()
{
    //debug("CComboCell::startEdit() called...");

    CMultiTypeItem* par = parent();
    QListView* lv = par->listView();
    QRect r = getCellRect();
    int x = r.x() - lv->contentsX();
    int y = r.y() - lv->contentsY();
    int width = r.width();
    int height = r.height();

    if (m_combo != 0) {
        //debug("   Strange....edit was not yet deleted! Did you call endEdit()?");
        m_combo->setValidator(0);
        delete m_combo;
        m_combo = 0;
    }

    //debug("   Creating & intitializing new combo widget...");
    m_combo = new QComboBox(m_readWrite, parent()->listView(), "inplace combo");
    m_combo->move(x, y);
    m_combo->resize(width, height);
    m_combo->show();
    fillCombo();
    lv->setFocusProxy(m_combo);
    m_combo->setFocus();
    //debug("   done.");
    return this;
}

// documentation is inherited
void CComboCell::endEdit()
{
    //debug("CComboCell::endEdit() called...");

    // delete the old edit (at last....)
    if (m_prevCombo != 0) {  // This might seem strange...Why not delete the
        //m_prevCombo->setValidator(0);
        delete m_prevCombo;  // m_combo widget immediately? Well, I've tried that
        m_prevCombo = 0;     // but Qt apparently doesn't always remove the
    }                        // widget from the focus chain immediately, thus
    m_prevCombo = m_combo;   // leaving a dangerous a pointer in there.
    // If we keep the pointer for a while, things
    // appear to be alright again....

    m_combo->reparent(m_parkFocus, QPoint(0,0));
    m_parkFocus->setFocus();
    QListView* lv = parent()->listView();
    lv->setFocusProxy(0);
    //debug("   Retrieving data & deleting widget...");
    QString text = m_combo->currentText();
    if (text == "") text = m_defval;
    CCell::setValue(text);
    m_combo->disconnect();
    m_combo = 0;
    lv->setFocus();
}

// No longer used
void CComboCell::onGainedFocus()
{
    //debug("CComboCell::onGainedFocus() emitting gainedFocus(this)");
    emit gainedFocus(this);
}
