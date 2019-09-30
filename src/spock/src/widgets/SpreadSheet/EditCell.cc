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
#include "src/spock/src/widgets/SpreadSheet/EditCell.h"
#include "src/spock/src/datastruct/Component.h"
#include "src/spock/src/datastruct/Keywords.h"
#include "src/spock/src/helper/log/assert.h"

// Qt includes
#include <qglobal.h>
#include <qwidget.h>
#include <qlineedit.h>
#include <qvalidator.h>
#include <qheader.h>
#include <qmessagebox.h>

using namespace std;

//============================================================================
//! Default constructor
/*!
    \param parent   The CMultiTypeItem this cell belongs to.

    Does some simple initialization.

    \sa CMultiTypeItem
 */
CEditCell::CEditCell(CMultiTypeItem* parent, CComponent* comp)
    : CCell(parent)
{
    m_edit = 0;
    //m_defval = 0;
    m_prevEdit = 0;
    m_parkFocus = new QWidget(0);
    m_parkFocus->hide();
    m_comp = comp;
}

//============================================================================
//! Default destructor
/*!
    Frees memory that was still allocated.
 */
CEditCell::~CEditCell()
{
    if (m_prevEdit != 0) {
        delete m_prevEdit;
        m_prevEdit = 0;
    }
    if (m_edit != 0) {
        delete m_edit;
        m_edit = 0;
    }
    if (m_parkFocus != 0) {
        delete m_parkFocus;
        m_parkFocus = 0;
    }
}

void CEditCell::setDefault(const QString& val)
{
    m_defval = val;
}

CCell* CEditCell::startEdit()
{
    debug("CEditCell::startEdit() called...");

    CMultiTypeItem* par = parent();
    QListView* lv = par->listView();

    QRect r = getCellRect();
    int x = r.x() - lv->contentsX();
    int y = r.y() - lv->contentsY();
    int width = r.width();
    int height = r.height();

    if (m_edit != 0) {
        debug("   Strange....edit was not yet deleted! Did you call endEdit()?");
        m_edit->setValidator(0);
        delete m_edit;
        m_edit = 0;
    }

    debug("   Creating & intitializing new edit widget...");
    m_edit = new QLineEdit(parent()->listView(), "inplace edit");
    m_edit->setValidator(getValidator());
    m_edit->move(x, y);
    m_edit->resize(width, height);
    m_edit->show();
    m_edit->setText(getValue());
    m_edit->setSelection(0, getValue().length());
    lv->setFocusProxy(m_edit);
    m_edit->setFocus();
    debug("   done.");
    return this;
}

void CEditCell::endEdit()
{
    debug("CEditCell::endEdit() called...");

    if (m_edit != 0) {
	CMultiTypeItem* par = parent();
	QListView* lv = par->listView();
        debug("   An edit exists...");
	QString newvalue = m_edit->text();

        if (m_edit->validator() != 0) {
            debug("   A validator exists...");
	    int p = m_edit->cursorPosition();
	    p = m_edit->validator()->validate(newvalue, p) != QValidator::Acceptable;
	    if (p || newvalue == "") {
		p = 1;
		if (newvalue != "" || !m_defval) {
		    newvalue = getValue(); // back to oldvalue
		    if (newvalue != "")
			QMessageBox::warning(0, "Typo!", "Whoops! This value is not possible!");
		}
	    }
	    if (!p && m_comp->getProperty(PROP_IMPLEMENT) == "unique") {
		QListViewItem* item = lv->firstChild();
		ASSERT (item != 0);
		int r = (m_comp->getType() == KEY_REAL);
		int col = par->findColumn(this);
		ASSERT (col >= 0);
		double v = r? atof(newvalue.latin1()) : 0;
		while (item) {
		    QString value = item->text(col);
		    CMultiTypeItem* multi = dynamic_cast<CMultiTypeItem*>(item);
		    if (multi != par) { // not current row
		    if ((!r && newvalue == value) || (r && v == atof(value.latin1()))) {
			newvalue = getValue(); // back to oldvalue
			QMessageBox::warning(0, "Typo!", "Whoops! This value is not unique!");
			break;
		    }}
		    item = item->nextSibling();
		}
	    }
	}
	else
	    newvalue = newvalue.simplifyWhiteSpace();

        // delete the old edit (at last....)
        if (m_prevEdit != 0) {  // This might seem strange...Why not delete the
            //m_prevEdit->setValidator(0);
            delete m_prevEdit;  // m_edit widget immediately? Well, I've tried that
            m_prevEdit = 0;     // but Qt apparently doesn't always remove the
        }                       // widget from the focus chain immediately, thus
        m_prevEdit = m_edit;    // leaving a dangerous dangling pointer in there.
                                // If we keep the pointer for a while, things
                                // appear to be alright again....
        m_edit->reparent(m_parkFocus, QPoint(0,0));
        m_parkFocus->setFocus();

	lv->setFocusProxy(0);
	debug("   Retrieving data & deleting widget...");
	if (newvalue == "" && m_defval) newvalue = m_defval;
	setValue(newvalue);
        m_edit->disconnect();
        m_edit = 0;
	lv->setFocus();
    }
}
