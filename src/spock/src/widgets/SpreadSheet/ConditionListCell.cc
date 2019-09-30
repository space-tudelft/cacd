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
#include "src/spock/src/widgets/SpreadSheet/ConditionListCell.h"
#include "src/spock/src/gui/dialogs/ConditionListDlg/ConditionListDlg.h"

using namespace std;

//============================================================================
//! Constructor
/*!
    \param parent   The CMultiTypeItem owning this cell.
    \param extended If true, the extra columns 'one side' and 'other side' are
                    also displayed.

    \sa CCell, CMultiTypeItem, CSpreadSheetView, CConditionListDlg
 */
CConditionListCell::CConditionListCell(CMultiTypeItem* parent, bool extended)
    : CCell(parent)
{
    m_extended = extended;
    m_defval = "";
}

//============================================================================
//! Destructor
CConditionListCell::~CConditionListCell()
{
}

void CConditionListCell::setDefault(const QString& val)
{
    m_defval = val;
}

// inherited documentation
CCell* CConditionListCell::startEdit()
{
    CMultiTypeItem* par = parent();
    ASSERT(par != 0);
    QListView* lv = par->listView();
    CConditionListDlg* dlg = new CConditionListDlg(lv, m_layers, m_layerComments, m_extended);
    dlg->setConditions(getValue());
    if (dlg->exec()) {
	QString text = dlg->getConditions().c_str();
	if (text == "") text = m_defval;
	CCell::setValue(text);
    }
    return this;
}

// inherited documentation
void CConditionListCell::endEdit()
{
    // Does not need to do anything, this cell uses a dialog.
}

//============================================================================
//! Sets the layer names
/*!
    \param values   A vector containing the layer names.

    \sa setComments()
 */
void CConditionListCell::setValue(const vector<QString>& values)
{
    m_layers.clear();
    for (unsigned int i=0; i<values.size(); ++i) {
        if (!values[i].isNull())
            m_layers.push_back(values[i].latin1());
        else
            m_layers.push_back("");
    }
}

//============================================================================
//! Sets the layer comments
/*!
    \param values   A vector containing the layer descriptions/comments
    \sa setValue(), getComments()
 */
void CConditionListCell::setComments(const vector<string>& values)
{
    m_layerComments = values;
}

//============================================================================
//! Retrieves the layer comments.
/*!
    \return  A vector containing the layer descriptions/comments.

    \sa setComments()
 */
vector<string> CConditionListCell::getComments()
{
    return m_layerComments;
}
