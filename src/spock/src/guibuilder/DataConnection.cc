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

#include <ctype.h>

//Project includes
#include "src/spock/src/guibuilder/DataConnection.h"
#include "src/spock/src/datastruct/Keywords.h"
#include "src/spock/src/datastruct/Component.h"
#include "src/spock/src/widgets/SpreadSheet/SpreadSheet.h"
#include "src/spock/src/widgets/SpreadSheet/Cell.h"
#include "src/spock/src/widgets/SpreadSheet/ConditionListCell.h"
#include "src/spock/src/guibuilder/GUITree.h"

// Qt includes
#include <qcombobox.h>

// STL includes
#include <algorithm>

using namespace std;

extern string oldValue;
extern string newValue;

//============================================================================
//! Default constructor.
/*!
    Intializes this datasource object.

    \param tree Is the CGUITree object from which the data is to be retrieved.
    \param comp Is the component acting as the data source.

    Both the \a tree and \a comp parameters must be valid, that is not 0.
    Otherwise, the constructor will assert.

    Since the default implementation assumes that the source component
    \a comp is a column in spreadsheet, the constructor will assert if
    this is not the case. Of course, derived implementations can override
    this behaviour because all members are made protected.

    The constructor sets some convenience members.
 */
CDataSource::CDataSource(CGUITree* tree, CComponent* comp)
{
    ASSERT(tree != 0);
    m_tree = tree;
    m_sourceSheet = 0;

    if (comp) {
	m_sourceComp = comp->getParent();
	ASSERT(m_sourceComp != 0);
	ASSERT(m_sourceComp->getType() == KEY_SPREADSHEET);

	QWidget* widget = tree->getWidget(m_sourceComp);
	m_sourceSheet = dynamic_cast<CSpreadSheetView*>(widget);
	ASSERT(m_sourceSheet != 0);

	m_sourceColumn = m_sourceComp->childPos(comp->getName());
	ASSERT(m_sourceColumn != -1);

	connect(m_sourceSheet, SIGNAL(valueChanged(CSpreadSheetView*, int)),
	    this, SLOT(onValueChanged(CSpreadSheetView*, int)));
    }
}

//============================================================================
//! Called when a source spreadsheet cell changes value.
void CDataSource::onValueChanged(CSpreadSheetView* sheet, int col)
{
    ASSERT(sheet == m_sourceSheet);
    if (col == m_sourceColumn) emit valueChanged();
}

//============================================================================
//! Returns the gui tree this data source uses.
/*!
    \return The gui tree this data source uses.
 */
CGUITree* CDataSource::guiTree()
{
    return m_tree;
}

//============================================================================
//! Returns the current value(s) of this data source
/*!
    \return The current value(s) of the source.
 */
vector<string> CDataSource::getValues(bool add)
{
    vector<string> retval;
    if (m_sourceSheet) {
	QListViewItem* item = m_sourceSheet->firstChild();
	while(item) {
	    if (!item->text(m_sourceColumn).isNull())
		retval.push_back(item->text(m_sourceColumn).latin1());
	    else if (add == true)
		retval.push_back("");
	    item = item->nextSibling();
	}
    }
    return retval;
}

//============================================================================
//! Default constructor
/*!
    \param tree         The CGUITree object associated with this target.
    \param targetComp   The target component.

    Both arguments must be vaild, non-null pointers or the constructor
    will assert.
 */
CDataTarget::CDataTarget(CGUITree* tree, CComponent* targetComp)
{
    ASSERT(tree != 0);
    ASSERT(targetComp != 0);

    m_tree = tree;
    m_targetComp = targetComp;
}

//============================================================================
//! Returns the gui tree associated with this data target
/*!
    \return The gui tree associated with this data target

    \sa CGUITree, component()
 */
CGUITree* CDataTarget::guiTree()
{
    return m_tree;
}

//============================================================================
//! Returns the target component.
/*!
    \return The target component.

    \sa CComponent, guiTree()
 */
CComponent* CDataTarget::component()
{
    return m_targetComp;
}

//============================================================================
//! Default constructor
/*!
    \param tree         The CGUITree object associated with this target.
    \param targetComp   The target component.

    Both arguments must be vaild, non-null pointers or the constructor
    will assert.

    If the \a targetComp component is not a spreadsheet column the constructor
    will assert.
 */
CColumnDataTarget::CColumnDataTarget(CGUITree* tree, CComponent* targetComp)
    : CDataTarget(tree, targetComp)
{
    CComponent* parent = component()->getParent();
    QWidget* widget = guiTree()->getWidget(parent);
    m_targetSheetWidget = dynamic_cast<CSpreadSheetView*>(widget);

    ASSERT(m_targetSheetWidget != 0);

    m_targetColumn = parent->childPos(targetComp->getName());
    ASSERT(m_targetColumn != -1);
}

//============================================================================
//! Updates the value(s) of this target.
/*!
    \param sources  The datasources to use for updating.

    This method is called by the correct CDataConnection object when necessary.

    \sa CDataConnection, CDataConnection::onSynchronize()
 */
void CColumnDataTarget::updateValues(const vector<CDataSource*>& sources, int s)
{
    QListViewItem* item = m_targetSheetWidget->firstChild();
    if(!item) return;

    CComponent* comp = component(); // maybe items must be added
    ASSERT(comp != 0);
    vector<QString> tmp;
    int i, nr = comp->numChildren();
    for (i=0; i<nr; ++i) {
	tmp.push_back(comp->getChild(i)->getProperty(PROP_TITLE).c_str());
    }
    nr = sources.size();
    ASSERT(nr > 0);
    for (i=0; i<nr; ++i) {
	vector<string> data = sources[i]->getValues();
	for (unsigned int j=0; j<data.size(); ++j)
	    tmp.push_back(data[j].c_str());
    }

    if (s == -1) { // initial update, test for correct cell value
	if (comp->getType() == KEY_DROPDOWN) nr = tmp.size();
	else s = -2;
    }
    else if (oldValue == "") s = -2;

    do {
	CMultiTypeItem* multi = dynamic_cast<CMultiTypeItem*>(item);
	if (multi != 0) {
	    CCell* cell = multi->getCell(m_targetColumn);
	    cell->setValue(tmp); // set CComboCell m_values
	    if (s == -1) { // initial update, test for correct cell value
		QString txt = cell->getValue();
		if (txt != "") {
		for (i=0; i<nr; ++i)
		    if (txt == tmp[i]) break;
		if (i == nr) { // value not found!
		    fprintf(stderr, "column '%s': '%s' unknown dropdown value!\n",
			comp->getName().c_str(), txt.latin1());
		}
		}
	    }
	    else if (s >= 0) {
		QString oldTxt = cell->getValue();
		string txt = oldTxt.latin1();
		string::size_type idx = txt.find(oldValue);
		if (idx != string::npos) { // found oldValue in oldTxt
		    int ok = oldValue.length();
		    if (idx > 0) {
			char cs = txt[idx-1];
			if (isalnum (cs) || cs == '_') ok = 0;
		    }
		    if (ok && idx+ok < txt.length()) {
			char cs = txt[idx+ok];
			if (isalnum (cs) || cs == '_') ok = 0;
		    }
		    if (ok) {
			if (newValue == "") {
			    cell->setTxtValue(newValue,m_targetColumn); // set CCell
			}
			else {
			    txt.replace(idx, ok, newValue);
			    cell->setTxtValue(txt,m_targetColumn); // set CCell
			}
		    }
		}
	    }
	}
	item = item->nextSibling();
    } while (item);
}

//============================================================================
//! Default constructor
/*!
    \param tree         The CGUITree object associated with this target.
    \param targetComp   The target component.

    Both arguments must be vaild, non-null pointers or the constructor
    will assert.

    If the \a targetComp component is not a combobox or dropdown the constructor
    will assert.
 */
CComboDataTarget::CComboDataTarget(CGUITree* tree, CComponent* targetComp)
    : CDataTarget(tree, targetComp)
{
    QWidget* widget = guiTree()->getWidget(targetComp);
    m_targetCombo = dynamic_cast<QComboBox*>(widget);
    ASSERT(m_targetCombo != 0);
}

//============================================================================
//! Updates the value(s) of this target.
/*!
    \param sources  The datasources to use for updating.

    This method is called by the correct CDataConnection object when necessary.

    \sa CDataConnection, CDataConnection::onSynchronize()
 */
void CComboDataTarget::updateValues(const vector<CDataSource*>& sources, int s)
{
    QString oldT = m_targetCombo->currentText();
    if (oldT.isNull()) oldT = "";
    string oldTxt = oldT.latin1();

    m_targetCombo->clear();

    CComponent* comp = component();
    ASSERT(comp != 0);
    string tmp;
    int j, nr = -1, nnr = -1;
    for (j = 0; j<comp->numChildren(); ++j) { // add items
	tmp = comp->getChild(j)->getProperty(PROP_TITLE);
	if (tmp == oldTxt) nr = j;
	if (tmp == newValue) nnr = j;
	m_targetCombo->insertItem(tmp.c_str());
    }
    ASSERT(sources.size() > 0);
    for (unsigned int i=0; i<sources.size(); ++i) {
	vector<string> sourceData = sources[i]->getValues();
	for (unsigned int k=0; k<sourceData.size(); ++k) {
	    tmp = sourceData[k];
	    if (tmp == oldTxt) nr = j;
	    if (tmp == newValue) nnr = j;
	    m_targetCombo->insertItem(tmp.c_str());
	    ++j;
	}
    }

    if (nr < 0) { // current value not found!
	int ok = 0;
	if (oldTxt == oldValue) { nr = nnr; ok = 1; }
	else if (comp->getType() == KEY_COMBOBOX && oldTxt != "") {
	    if (newValue == "") ok = 0;
	    else if ((ok = oldValue.length()) > 0) {
		string::size_type idx = oldTxt.find(oldValue);
		if (idx != string::npos) { // found oldValue in oldTxt
		    if (idx > 0) {
			char cs = oldTxt[idx-1];
			if (isalnum (cs) || cs == '_') ok = 0;
		    }
		    if (ok && idx+ok < oldTxt.length()) {
			char cs = oldTxt[idx+ok];
			if (isalnum (cs) || cs == '_') ok = 0;
		    }
		    if (ok) oldTxt.replace(idx, ok, newValue);
		    else ok = 1;
		}
	    }
	    else ok = 1;
	    if (ok) { m_targetCombo->insertItem(oldTxt.c_str()); nr = j; }
	}
	else if (s == -1 && oldTxt != "") { // initial dropdown update
	    fprintf(stderr, "field '%s': '%s' unknown dropdown value!\n",
		comp->getName().c_str(), oldTxt.c_str());
	}
	if (!ok) nr = comp->childPos(comp->getProperty(PROP_DEFAULT));
    }
    if (nr >= 0) m_targetCombo->setCurrentItem(nr);
}

//============================================================================
//! Default constructor
/*!
    The \a target must be valid and not 0.
 */
CDataConnection::CDataConnection(CDataTarget* target)
{
    ASSERT(target != 0);
    m_target = target;
    m_enabled = true;
}

//============================================================================
//! Enables updating the values.
/*!
    If updating is enabled, every time the source signals a value change the
    target will be updated.

    The target will be updated by onSynchronize() when this method is called.

    \sa disable(), onSynchronize()
 */
void CDataConnection::enable()
{
    m_enabled = true;
    m_target->updateValues(m_sources, -1); // onSynchronize()
}

//============================================================================
//! Disables updating the values.
/*!
    If updating is disabled, value change signals emitted by the source will
    be ignored.

    Calling enable() will turn updating back on as well as performing an
    immediate synchronize.

    \sa enable(), onSynchronize()
*/
void CDataConnection::disable()
{
    m_enabled = false;
}

//============================================================================
//! Adds a data source to this connection
/*!
    \param source The data source to add.

    The new source \a source is added to the list of sources. A connection
    is made between the source object's valueChanged() signal and this objects
    onSynchronize() slot.

    The target is not immediately updated. If this is desired a manual call to
    onSynchronize() is necessary.

    \sa CDataSource, onSynchronize()
 */
void CDataConnection::addDataSource(CDataSource* source)
{
    if (source == 0) return;
//  for (unsigned int i=0; i<m_sources.size(); ++i)
//	if (m_sources[i] == source) return;

    m_sources.push_back(source);
if ((m_sources.size() % 2) == 0)
    connect(source, SIGNAL(valueChanged ()), this, SLOT(onSynchronize2()));
else
    connect(source, SIGNAL(valueChanged ()), this, SLOT(onSynchronize()));
}

//============================================================================
//! Returns true if \a comp is the target of this data connection.
/*!
    \return True if \a comp is the target of this data connection, false
    if this is not the case.
 */
bool CDataConnection::isTarget(CComponent* comp)
{
    if (comp == m_target->component())
        return true;
    return false;
}

//============================================================================
//! Updates the target's value with the current source values.
/*!
    \sa CDataTarget::updateValues()
 */
void CDataConnection::onSynchronize()
{
    if (m_enabled) m_target->updateValues(m_sources, 0);
}

void CDataConnection::onSynchronize2()
{
    if (m_enabled) m_target->updateValues(m_sources, 1);
}

//============================================================================
//! Default constructor
/*!
    \param tree         The CGUITree object associated with this target.
    \param targetComp   The target component.

    Both arguments must be vaild, non-null pointers or the constructor
    will assert.

    If the \a targetComp component is not a column in a spreadsheet the
    constructor will assert.
 */
CConditionListDataTarget::CConditionListDataTarget(CGUITree* tree, CComponent* targetComp)
    : CDataTarget(tree, targetComp)
{
    CComponent* parent = component()->getParent();
    QWidget* widget = guiTree()->getWidget(parent);
    m_targetSheetWidget = dynamic_cast<CSpreadSheetView*>(widget);

    ASSERT(m_targetSheetWidget != 0);

    m_targetColumn = parent->childPos(targetComp->getName());
    ASSERT(m_targetColumn != -1);
}

//============================================================================
//! Updates the value(s) of this target.
/*!
    \param sources  The datasources to use for updating.

    This method is called by the correct CDataConnection object when necessary.

    The condition list data target can have one or two data sources.
    The first data source is used for the layer names. The second (optional)
    data source is used for the layer comments.

    \sa CDataConnection, CDataConnection::onSynchronize()
 */
void CConditionListDataTarget::updateValues(const vector<CDataSource*>& sources, int s)
{
    QListViewItem* item = m_targetSheetWidget->firstChild();
    if(!item) return;

    int k, nr = sources.size();
    ASSERT(nr > 0);
    vector<string> comments;
    vector<QString> nameVect;

    for (k=0; k<nr; ++k) {
	vector<string> names = sources[k]->getValues(true); // Do names
	vector<string> comms; // Do comments, if source was specified.
	unsigned int i = 0;
	if (++k < nr) {
	    debug("Comments too....");
	    comms = sources[k]->getValues(true);
	    i = comms.size();
	}
	for (unsigned int j=0; j<names.size(); ++j) {
	    if (names[j] != "") {
		nameVect.push_back(names[j].c_str());
		if (j < i)
		    comments.push_back(comms[j]);
		else
		    comments.push_back("");
	    }
	}
    }

    do {
        CMultiTypeItem* multi = dynamic_cast<CMultiTypeItem*>(item);
        if (multi != 0) {
            CCell* tmpcell = multi->getCell(m_targetColumn);
            CConditionListCell* cell = dynamic_cast<CConditionListCell*>(tmpcell);
            cell->setValue(nameVect);
	    cell->setComments(comments);

	    if (s == -1) { // initial update, test for correct value(s)
		char *v, cs, tmp[128];
		strcpy (tmp, tmpcell->getValue().latin1());
		v = tmp;
again:		while (*v && !isalnum (*v) && *v != '_') ++v;
		if (*v) {
		    char *t = v + 1;
		    while (isalnum (*t) || *t == '_') ++t;
		    if ((cs = *t) != '\0') *t = '\0';
		    nr = nameVect.size();
		    for (k=0; k<nr; ++k)
			if (strcmp(v, nameVect[k].latin1()) == 0) break;
		    if (k == nr) {
			CComponent* comp = component();
			fprintf(stderr, "column '%s': '%s' unknown mask value!\n",
			    comp? comp->getName().c_str():"Condlist", v);
		    }
		    if (cs) { v = t + 1; goto again; }
		}
	    }
	    else if (!s && oldValue != "") {
		string txt = tmpcell->getValue().latin1();
		string::size_type idx, n_idx = 0;
		while ((idx = txt.find(oldValue, n_idx)) != string::npos) {
		    int ok = oldValue.length();
		    n_idx = idx + 1;
		    if (idx > 0) {
			char cs = txt[idx-1];
			if (isalnum (cs) || cs == '_') ok = 0;
		    }
		    if (ok && idx+ok < txt.length()) {
			char cs = txt[idx+ok];
			if (isalnum (cs) || cs == '_') ok = 0;
		    }
		    if (ok) {
			if (newValue == "" && idx > 0) {
			    char cs = txt[idx-1];
			    if (cs == '!' || cs == '-' || cs == '=') {
				--idx; ++ok;
				if (cs != '!' && idx > 0) {
				    cs = txt[idx-1];
				    if (cs == '!') { --idx; ++ok; }
				}
			    }
			}
			txt.replace(idx, ok, newValue);
			s = 1;
		    }
		}
		if (s) {
		    s = 0;
		    tmpcell->setTxtValue(txt,m_targetColumn); // set CCell
		}
	    }
        }
	item = item->nextSibling();
    } while (item);
}
