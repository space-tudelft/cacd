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

// Project inlcudes
#include "src/spock/src/guibuilder/GUITree.h"
#include "src/spock/src/guibuilder/DataConnection.h"
#include "src/spock/src/datastruct/Keywords.h"
#include "src/spock/src/datastruct/Component.h"
#include "src/spock/src/datastruct/ComponentTree.h"
#include "src/spock/src/widgets/SpreadSheet/SpreadSheet.h"
#include "src/spock/src/widgets/SpreadSheet/AllCells.h"
#include "src/spock/src/guibuilder/SerializerVisitor.h"
#include "src/spock/src/gui/validators/IdentValidator.h"

// Qt includes
#include <qlistview.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qregexp.h>

// STL includes
#include <algorithm>
#include <sstream>
#include <iostream>

extern string oldValue;

using namespace std;

//============================================================================
//! Default constructor
/*!
    \param tree Associated component tree.

    A GUI tree is always associated with a component tree, therefore this
    parameter must never be 0. The constructor will assert of the \a tree
    parameter is 0.
 */
CGUITree::CGUITree(CComponentTree* tree)
{
    ASSERT(tree != 0);
    m_componentTree = tree;
    m_fileGen = 0;
}

//============================================================================
//! Maps a Qt widget to the corresponding component in the component tree.
/*!
    \param component The component being mapped to
    \param The widget that is being mapped.

    \sa getComponent(), getWidget()
 */
void CGUITree::makeMapping(CComponent* component, QWidget* widget)
{
    m_mapping[component] = widget;
}

//============================================================================
//! Adds a data connection to the list of known connections.
/*!
    The list of data connections is created by the buildDataConnections()
    method. This method is executed after the gui tree has been created by the
    CGUIBuilderVisitor class.

    \sa CGUIBuilderVisitor, buildDataConnections(), CDataConnection

 */
void CGUITree::addDataConnection(CDataConnection* connection)
{
    vector<CDataConnection*>::iterator pos;
    pos = find(m_dataConnections.begin(), m_dataConnections.end(), connection);
    if (pos == m_dataConnections.end())
        m_dataConnections.push_back(connection);
}

//============================================================================
//! Test the generateFile process.
bool CGUITree::isFileGen()
{
    return m_fileGen;
}

//============================================================================
//! Flag the generateFile process.
void CGUITree::toggleFileGen()
{
    m_fileGen = !m_fileGen;
}

//============================================================================
//! Tries to find a component named \a name.
/*!
    \return Returns the component if found, otherwise returns 0.

    \sa CComponent, CComponentTree::findComponent()
 */
CComponent* CGUITree::findComponent(const string& name)
{
    return m_componentTree->findComponent(name);
}

//============================================================================
//! Returns the component associated with \a widget
/*!
    \param widget The widget that is associated with the requested component.

    \return The requested component. Returns 0 if the widget has no associated
    component.

    \sa makeMapping(), getWidget()
 */
CComponent* CGUITree::getComponent(QWidget* widget)
{
    map<CComponent*, QWidget*>::iterator pos;

    for (pos = m_mapping.begin(); pos != m_mapping.end(); ++pos) {
        if (pos->second == widget)
            return pos->first;
    }
    return 0;
}

//============================================================================
//! Returns the widget representing \a component.
/*!
    \param component The component representing the requested widget.
    \return The widget representing \a component or 0 if the component has no
    associated widget.
 */
QWidget* CGUITree::getWidget(CComponent* component)
{
    if (m_mapping.count(component)>0)
        return m_mapping[component];
    return 0;
}

//============================================================================
//! Returns the tabpages in this gui tree
/*!
    \return A vector containing the tabpages present in this gui tree.
 */
vector<QWidget*> CGUITree::getTabPages()
{
    vector<QWidget*> result;

    vector<CComponent*> rootComps = m_componentTree->getRootComponents();
    for (unsigned int i=0; i<rootComps.size(); ++i) {
        QWidget* widget = getWidget(rootComps[i]);
        if (widget != 0)
            result.push_back(widget);
    }

    // If we were to iterate over m_mapping and check for tabpages,
    // the tabpages found will not be in the same order as in the
    // ui specification file. That is why it is done in this fashion
    // instead. getRootComponents() returns them in the correct order.

    return result;
}

//============================================================================
//! Returns a pointer to the associated component tree.
/*!
    \return A pointer to the associated component tree.

    \sa CComponentTree, CComponent
 */
CComponentTree* CGUITree::getComponentTree()
{
    return m_componentTree;
}

//============================================================================
//! Retrieves current value(s) of the component \a comp.
/*!
    \param comp The component to retrieve the current value(s) from
    \param field Can be used to influence the retrieved value.

    The \a field argument can be used to retrieve something other than the
    default value(s). For example, for dalistyle type components this can be
    either "clr" or "fill" to retrieve only the color or only the fill style.

    \returns A vector with the requested values.
 */
vector<string> CGUITree::getValues(CComponent* comp, const string& field /*=""*/)
{
    vector<string> retval;

    if (comp == 0)
        return retval;

    string type = comp->getType();
    string parentType = "";

    CComponent* parent = comp->getParent();
    if (parent != 0)
        parentType = parent->getType();

    if (parentType == KEY_SPREADSHEET)
        return getSpreadSheetValues(parent, comp, field);

    if (type == KEY_CONDITIONLIST || type == KEY_IDENTIFIER ||
            type == KEY_REAL || type == KEY_INTEGER || type == KEY_STRING) {
        QWidget* widget = getWidget(comp);
        QLineEdit* edit = dynamic_cast<QLineEdit*>(widget);
        ASSERT(edit != 0);
        if (!edit->text().isNull())
            retval.push_back(edit->text().latin1());
    }

    if (type == KEY_DROPDOWN || type == KEY_COMBOBOX) {
        QWidget* widget = getWidget(comp);
        QComboBox* combo = dynamic_cast<QComboBox*>(widget);
        ASSERT(combo != 0);
        if (!combo->currentText().isNull())
            retval.push_back(combo->currentText().latin1());
    }

    if (type == KEY_SPREADSHEET) {
        if (field == "numrows") {
            QWidget* widget = getWidget(comp);
            CSpreadSheetView* sheetWidget = dynamic_cast<CSpreadSheetView*>(widget);
            ASSERT(sheetWidget != 0);
            ostringstream tmp;
            tmp << sheetWidget->childCount() << ends;
            retval.push_back(tmp.str());
        }
    }
    return retval;
}

int realTest(const string& value)
{
    QString v;
    string::size_type idx = value.find('e');
    if (idx == string::npos) idx = value.find('E');
    if (idx != string::npos) {
	QRegExp exp2("^-?[1-9][0-9]?$");
	v = value.substr(idx+1).c_str();
	if (exp2.search(v, 0) != 0) return 1;
	v = value.substr(0,idx).c_str();
    }
    else v = value.c_str();
    QRegExp exp1("^-?[0-9]+\\.?[0-9]*$");
    if (exp1.search(v, 0) == 0) return 0;
    QRegExp expr("^-?[0-9]*\\.?[0-9]+$");
    return expr.search(v, 0);
}

int intTest(const string& value)
{
    QRegExp expr("^-?[0-9]+$");
    return expr.search(value.c_str(), 0);
}

int identTest(const string& value)
{
    QRegExp expr("^[a-zA-Z_][a-zA-Z_0-9]*$");
    return expr.search(value.c_str(), 0);
}

//============================================================================
//! Sets the value of the widget associated with \a comp.
/*!
    \param comp Component associated with the widget who's values must be set.
    \param values   The new value(s).

    This method tries to set the value(s) of the widget associated with \a comp.
    This is done differently depending on the type of widget.

    \sa getValues(), setSpreadSheetValues()
 */
void CGUITree::setValues(CComponent* comp, const vector<string>& values)
{
    ASSERT(comp != 0);
    ASSERT(values.size() > 0);

    string type = comp->getType();
    string parentType = "";

    CComponent* parent = comp->getParent();
    if (parent != 0)
        parentType = parent->getType();

    if (parentType == KEY_SPREADSHEET) {
        setSpreadSheetValues(parent, comp, values);
	return;
    }
    else if (type == KEY_REAL || type == KEY_INTEGER ||
		type == KEY_STRING || type == KEY_IDENTIFIER) {
	string value = values[0];
	if (value == "") goto ret; // use default
        QWidget* widget = getWidget(comp);
        QLineEdit* edit = dynamic_cast<QLineEdit*>(widget);
        ASSERT(edit != 0);
	debug("Setting edit %s = %s", comp->getName().c_str(), value.c_str());
	int j = 0;
	if (type == KEY_REAL) {
	    j = realTest(value);
	    if (j) fprintf(stderr, "field '%s': '%s' incorrect real value!\n",
		comp->getName().c_str(), value.c_str());
	    else {
		vector<string> range = comp->getPropertyVector("range");
	      if (range.size() > 0) {
		double m_min, m_max, v = 1e100;
		m_min = (range[0] == "")? -v : atof(range[0].c_str());
		m_max = (range[1] == "")?  v : atof(range[1].c_str());
		v = atof(value.c_str());
		if (v < m_min || v > m_max) {
		    fprintf(stderr, "field '%s': '%s' value out of range!\n",
			comp->getName().c_str(), value.c_str());
		    value = (v < m_min)? range[0] : range[1];
		}
	      }
	    }
	}
	else if (type == KEY_INTEGER) {
	    j = intTest(value);
	    if (j) fprintf(stderr, "field '%s': '%s' incorrect integer value!\n",
		comp->getName().c_str(), value.c_str());
	    else {
		vector<string> range = comp->getPropertyVector("range");
	      if (range.size() > 0) {
		double m_min, m_max, v = 2147483647;
		m_min = (range[0] == "")? -v : atof(range[0].c_str());
		m_max = (range[1] == "")?  v : atof(range[1].c_str());
		v = atof(value.c_str());
		if (v < m_min || v > m_max) {
		    fprintf(stderr, "field '%s': '%s' value out of range!\n",
			comp->getName().c_str(), value.c_str());
		    value = (v < m_min)? range[0] : range[1];
		}
	      }
	    }
	}
	else if (type == KEY_IDENTIFIER) {
	    j = identTest(value);
	    if (j) fprintf(stderr, "field '%s': '%s' incorrect identifier value!\n",
		comp->getName().c_str(), value.c_str());
	}
	if (!j) edit->setText(value.c_str());
    }
    else if (type == KEY_DROPDOWN || type == KEY_COMBOBOX) {
	if (values[0] == "") goto ret; // use default
        QWidget* widget = getWidget(comp);
        QComboBox* combo = dynamic_cast<QComboBox*>(widget);
        ASSERT(combo != 0);
	if (combo->editable())
	    combo->setEditText(values[0].c_str());
	else {
	    for (int i=0; i<combo->count(); ++i) {
		if (combo->text(i) == values[0].c_str()) {
		    combo->setCurrentItem(i);
		    goto ret;
		}
	    }
	    fprintf(stderr, "field '%s': '%s' unknown dropdown value!\n",
		comp->getName().c_str(), values[0].c_str());
	}
    }
    else {
	fprintf(stderr, "field '%s': '%s' unknown type!\n",
		comp->getName().c_str(), type.c_str());
	return;
    }
ret:
    if (values.size() != 1)
	fprintf(stderr, "field '%s': '%d' incorrect value size!\n",
		comp->getName().c_str(), (int)values.size());
}

//============================================================================
//! Retrieves values of a spreadsheet column
/*!
    \param sheet The spreadsheet
    \param comp  The column
    \param field Can be used to influence the retrieved value. See alse getValues()

    \return The values in the requested column of the spreadsheet.

    \sa setSpreadSheetValues(), getValues(), setValues()
 */
vector<string> CGUITree::getSpreadSheetValues(CComponent* sheet, CComponent* comp, const string& field /*=""*/)
{
    ASSERT(sheet != 0);
    ASSERT(comp != 0);
    ASSERT(sheet->getType() == KEY_SPREADSHEET);

    vector<string> retval;

    QWidget* widget = getWidget(sheet);
    CSpreadSheetView* sheetWidget = dynamic_cast<CSpreadSheetView*>(widget);
    ASSERT(sheetWidget != 0);

    int column = sheet->childPos(comp->getName());
    ASSERT(column != -1);

    QListViewItem* item = sheetWidget->firstChild();
    while (item != 0) {
        QString tmp = item->text(column);
        string result = "";
        if (!tmp.isNull())
            result = tmp.latin1();

        string type = comp->getType();
        if (type == KEY_DALISTYLE && field == "clr") {
            string::size_type idx = result.find(' ');
            if (idx != string::npos)
                result = result.substr(0, idx);
        }
        if (type == KEY_DALISTYLE && field == "style") {
            string::size_type idx = result.find(' ');
            if (idx != string::npos)
                result = result.substr(idx+1);
        }
        if (type == KEY_COLOR && isFileGen()) {
            string::size_type idx = result.find('#');
            if (idx == 0)
                result = "@" + result.substr(idx+1);
        }
        retval.push_back(result);
        item = item->nextSibling();
    }
    return retval;
}

//============================================================================
//! Sets the column of a spreadsheet to the values in \a values.
/*!
    \param sheet    The spreadsheet
    \param comp     The column
    \param values   The new values.

    If the number of rows in the sheet is insufficient new rows are created.
 */
void CGUITree::setSpreadSheetValues(CComponent* sheet, CComponent* comp, const vector<string>& values)
{
    ASSERT(sheet != 0);
    ASSERT(comp != 0);

    QWidget* widget = getWidget(sheet);
    CSpreadSheetView* sheetWidget = dynamic_cast<CSpreadSheetView*>(widget);
    ASSERT(sheetWidget != 0);

    int column = sheet->childPos(comp->getName());
    ASSERT(column != -1);

    int i, size = values.size();
    int numToMake = size - sheetWidget->childCount();
    for (i=0; i<numToMake; ++i)
	sheetWidget->addNewRow();

    string type = comp->getType();
    int dropdown = 0;
    int test = 0;
    int uniq = 0;
    vector<string> range;
    double m_min = 0, m_max = 0;

    if (type == KEY_DROPDOWN)
	if (comp->getProperty(PROP_DATASOURCE) == "") dropdown = comp->numChildren();

    if (type == KEY_REAL) { test = 1;
	if (comp->getProperty(PROP_IMPLEMENT) == "unique") uniq = 1;
	range = comp->getPropertyVector("range");
	if (range.size() > 0) {
	    m_max = 1e100;
	    m_min = (range[0] == "")? -m_max : atof(range[0].c_str());
	    if (range[1] != "") m_max = atof(range[1].c_str());
	}
    }
    else if (type == KEY_INTEGER) { test = 2;
	if (comp->getProperty(PROP_IMPLEMENT) == "unique") uniq = 1;
	range = comp->getPropertyVector("range");
	if (range.size() > 0) {
	    m_max = 2147483647;
	    m_min = (range[0] == "")? -m_max : atof(range[0].c_str());
	    if (range[1] != "") m_max = atof(range[1].c_str());
	}
    }
    else if (type == KEY_IDENTIFIER) { test = 3;
	if (comp->getProperty(PROP_IMPLEMENT) == "unique") uniq = 1;
    }
    else if (type == KEY_DALISTYLE) test = 4;
    else if (type == KEY_COLOR) test = 5;

    QListViewItem* item = sheetWidget->firstChild();
    for (i=0; i<size; ++i) {
	if (!item) {
	    fprintf(stderr, "column '%s': row '%d' item not found!\n",
		comp->getName().c_str(), i);
	    return;
	}
	string value = values[i];
        CMultiTypeItem* multi = dynamic_cast<CMultiTypeItem*>(item);
        if (!multi) {
	    fprintf(stderr, "column '%s': row '%d' item type not found!\n",
		comp->getName().c_str(), i);
            item->setText(column, value.c_str());
	}
	else if (value != "") { // Empty values don't need to be set!
			// By KEY_DALISTYLE an empty string gives a core dump!
	    int j = 0;
	    if (test == 1) { // real
		j = realTest(value);
		if (j) fprintf(stderr, "column '%s': '%s' incorrect real value!\n",
			comp->getName().c_str(), value.c_str());
		else if (range.size() > 0) {
		    double v = atof(value.c_str());
		    if (v < m_min || v > m_max) {
			fprintf(stderr, "column '%s': '%s' value out of range!\n",
			    comp->getName().c_str(), value.c_str());
			value = (v < m_min)? range[0] : range[1];
		    }
		}
		if (uniq && !j) {
		    double v = atof(value.c_str());
		  for (int k=0; k<i; ++k)
		    if (v == atof(values[k].c_str())) {
			fprintf(stderr, "column '%s': '%s' not unique value!\n",
			    comp->getName().c_str(), value.c_str());
			break;
		    }
		}
	    }
	    else if (test == 2) { // int
		j = intTest(value);
		if (j) fprintf(stderr, "column '%s': '%s' incorrect integer value!\n",
			comp->getName().c_str(), value.c_str());
		else if (range.size() > 0) {
		    double v = atof(value.c_str());
		    if (v < m_min || v > m_max) {
			fprintf(stderr, "column '%s': '%s' value out of range!\n",
			    comp->getName().c_str(), value.c_str());
			value = (v < m_min)? range[0] : range[1];
		    }
		}
		if (uniq && !j)
		for (int k=0; k<i; ++k)
		    if (value == values[k]) {
			fprintf(stderr, "column '%s': '%s' not unique value!\n",
			    comp->getName().c_str(), value.c_str());
			break;
		    }
	    }
	    else if (test == 3) { // ident
		j = identTest(value);
		if (j) fprintf(stderr, "column '%s': '%s' incorrect identifier value!\n",
			comp->getName().c_str(), value.c_str());
		if (uniq && !j)
		for (int k=0; k<i; ++k)
		    if (value == values[k]) {
			fprintf(stderr, "column '%s': '%s' not unique identifier!\n",
			    comp->getName().c_str(), value.c_str());
			break;
		    }
	    }
	    else if (test == 4) { // dali
		QRegExp expr("^[0-7] [0-8]$");
		j = expr.search(value.c_str(), 0);
		if (j) fprintf(stderr, "column '%s': '%s' incorrect value!\n",
			comp->getName().c_str(), value.c_str());
	    }
	    else if (test == 5) { // color
		QRegExp expr("^#[0-9a-fA-F]+$");
		j = value.length() != 7 || expr.search(value.c_str(), 0) != 0;
		if (j) fprintf(stderr, "column '%s': '%s' incorrect value!\n",
			comp->getName().c_str(), value.c_str());
	    }
	    else {
		for (; j<dropdown; ++j)
		    if (comp->getChild(j)->getProperty(PROP_TITLE) == value) {
			j = 0; break;
		    }
		if (j) fprintf(stderr, "column '%s': '%s' unknown dropdown value!\n",
			comp->getName().c_str(), value.c_str());
	    }
	    if (!j && !sheetWidget->setValue(multi, column, value.c_str()))
		fprintf(stderr, "column '%s': row '%d' cell not found!\n",
			comp->getName().c_str(), i);
	}
        item = item->nextSibling();
    }
}

//============================================================================
//! Creates the correct type of cell for the spreadsheets in this gui tree.
/*!
  The spreadsheet widgets in this gui tree need to create the correct type of
  cells when they are created an when they grow. The pleaseCreateCell() signals
  from the spreadsheet are all connected to the onPleaseCreateCell() slot of
  the gui tree in which they live.

  The gui tree checks wich type of component must be represented by the column
  specified and creates the correct CCell derived object. It does so by calling
  the correct (private) build...Cell() methods.

  \param item   The CMultiTypeItem (row) of the spreadsheet.
  \param column The column of the spreadsheet

  \sa CSpreadSheetView
 */
void CGUITree::onPleaseCreateCell(CMultiTypeItem* item, int column)
{
    ASSERT(item != 0);

    CSpreadSheetView* sheetWidget = dynamic_cast<CSpreadSheetView*>(item->listView());
    ASSERT(sheetWidget != 0);

    CComponent* sheetComp = getComponent(sheetWidget);
    ASSERT(sheetComp != 0);

    // debug("Component: %s, numChildren: %d, requesting child %d.", sheetComp->getName().c_str(), sheetComp->numChildren(), column);

    CComponent* childComp = sheetComp->getChild(column);
    ASSERT(childComp != 0);

    string type = childComp->getType();

    if (type == KEY_COMBOBOX || type == KEY_DROPDOWN) {
	buildComboCell(item, column, childComp);
    }
    else if (type == KEY_DALISTYLE) {
	buildDaliCell(item, column, childComp);
    }
    else if (type == KEY_COLOR) {
	buildColorCell(item, column, childComp);
    }
    else if (type == KEY_CONDITIONLIST) {
	buildConditionListCell(item, column, childComp);
    }
    else {
	buildEditCell(item, column, childComp);
    }

    CDataConnection* connection = isADataTarget(childComp);
    if (connection != 0) {
        // debug("Hmm, cell is a datatarget! synchronizing...");
	oldValue = "";
        connection->onSynchronize();
    }
}

//============================================================================
//! Builds a validator of the correct type for the parent widget.
QValidator* CGUITree::buildValidator(QWidget* parent, const string& type, CComponent* comp)
{
    if (type == KEY_REAL)
        return new CDoubleValidator(parent, comp->getPropertyVector("range"));
    if (type == KEY_INTEGER)
        return new CIntValidator(parent, comp->getPropertyVector("range"));
    if (type == KEY_IDENTIFIER)
	return new CIdentValidator(parent);
    return 0;
}

//============================================================================
//! Creates a line edit cell for a spreadsheet.
void CGUITree::buildEditCell(CMultiTypeItem* item, int col, CComponent* comp)
{
    string type = comp->getType();
    QValidator* validator = buildValidator(0, type, comp);

    CEditCell* cell = new CEditCell(item, comp);
    item->setCell(col, cell);
    cell->setValidator(validator);

    string def;
    if (comp->getProp(PROP_DEFAULT, def)) {
	cell->setDefault(def.c_str()); // Set m_defval
	cell->setValue(def.c_str());
    }
}

//============================================================================
//! Creates a combobox or dropdown cell for a spreadsheet.
void CGUITree::buildComboCell(CMultiTypeItem* item, int col, CComponent* comp)
{
    string type = comp->getType();
    bool editable = (type == KEY_COMBOBOX)? true : false;

    CComboCell* cell = new CComboCell(item, editable);
    item->setCell(col, cell);       // If not set here, ASSERT will occur...

    string def;
    int pos = comp->getProp(PROP_DEFAULT, def) ? -1 : -2;

    // Set the data
    vector<QString> data;
    int nc = comp->numChildren();
    for (int i=0; i<nc; ++i) {
	CComponent* child = comp->getChild(i);
	data.push_back(child->getProperty(PROP_TITLE).c_str());
        if (pos == -1 && def == child->getName()) pos = i;
    }
    if (pos == -2 && nc) pos = 0; // use item 0 as default
    if (pos >= 0) {
	cell->setAsDefault(pos);
	def = data[pos].latin1();
    }

    cell->setValue(data);           // Set m_values
    if (pos >= -1) {
	cell->setDefault(def.c_str()); // Set m_defval
	cell->CCell::setValue(def.c_str());
    }
}

//============================================================================
//! Creates a dalistyle cell for a spreadsheet.
void CGUITree::buildDaliCell(CMultiTypeItem* item, int col, CComponent* comp)
{
    CDaliCell* cell = new CDaliCell(item);
    item->setCell(col, cell);
    string def;
    if (comp->getProp(PROP_DEFAULT, def)) cell->setValue(def.c_str());
}

//============================================================================
//! Creates a color cell for a spreadsheet.
void CGUITree::buildColorCell(CMultiTypeItem* item, int col, CComponent* comp)
{
    CCell* cell = new CXSpaceCell(item);
    item->setCell(col, cell);
    string def;
    if (comp->getProp(PROP_DEFAULT, def)) cell->setValue(def.c_str());
}

//============================================================================
//! Creates a conditionlist cell for a spreadsheet.
void CGUITree::buildConditionListCell(CMultiTypeItem* item, int col, CComponent* comp)
{
    bool ext = (comp->getProperty(PROP_IMPLEMENT) == "extended")? true : false;
    CConditionListCell* cell = new CConditionListCell(item, ext);
    item->setCell(col, cell);
    string def;
    if (comp->getProp(PROP_DEFAULT, def)) {
	cell->setDefault(def.c_str()); // Set m_defval
	cell->CCell::setValue(def.c_str());
    }
}

//============================================================================
//! Returns true if the component \a comp is a data target.
/*!
    \return True if \a comp is a data target, false if not.
 */
CDataConnection* CGUITree::isADataTarget(CComponent* comp)
{
    for (unsigned int i=0; i<m_dataConnections.size(); ++i)
        if (m_dataConnections[i]->isTarget(comp))
            return m_dataConnections[i];
    return 0;
}

//============================================================================
//! Builds the dataconnections present in this tree.
/*!
    The CFindPropertyVisitor searches the tree for datasources. Every datasource
    found is connected to the specified target.

    \sa addDataConnection(), disableDataConnectionUpdates(), enableDataConnectionUpdates()
 */
void CGUITree::buildDataConnections()
{
    debug("Building dataconnections...");
    CFindPropertyVisitor search(PROP_DATASOURCE, "", CFindPropertyVisitor::NOTEQUAL);

    ASSERT(m_componentTree != 0);

    m_componentTree->accept(search);

    debug("Found %d datatargets...connecting them to their sources.", search.hitCount());

    for (int i=0; i<search.hitCount(); ++i) {
        CComponent* targetComp = search.getHit(i);
        if (targetComp != 0) {
            vector<string> sources = targetComp->getPropertyVector(PROP_DATASOURCE);
            CDataTarget* target = 0;
	    string type = targetComp->getType();
	    if (targetComp->getParent() != 0) {
		if (targetComp->getParent()->getType() == KEY_SPREADSHEET) {
		    if (type == KEY_CONDITIONLIST) {
			debug("Connection target = conditionlist");
			target = new CConditionListDataTarget(this, targetComp);
		    } else {
			debug("Connection target = column");
			target = new CColumnDataTarget(this, targetComp);
		    }
		} else if (type == KEY_COMBOBOX || type == KEY_DROPDOWN) {
		    target = new CComboDataTarget(this, targetComp);
		}
	    }
	    ASSERT(target != 0);
	    if (!target) continue;

            CDataConnection* connection = new CDataConnection(target);
	    CComponent* sourceCompPrev = 0;
            for (unsigned int i=0; i<sources.size(); ++i) {
                CComponent* sourceComp = findComponent(sources[i]);
		if (sourceComp ||
		    (type == KEY_CONDITIONLIST && (i % 2) == 1 && sourceCompPrev)) {
		    CDataSource* source = new CDataSource(this, sourceComp);
		    connection->addDataSource(source);
		}
		sourceCompPrev = sourceComp;
            }
            addDataConnection(connection);
        }
    }
}

//============================================================================
//! Disables all data connection updates for this tree.
void CGUITree::disableDataConnectionUpdates()
{
    for (unsigned int i=0; i<m_dataConnections.size(); ++i)
        m_dataConnections[i]->disable();
}

//============================================================================
//! Enables all data connection updates for this tree.
void CGUITree::enableDataConnectionUpdates()
{
    oldValue = "";
    for (unsigned int i=0; i<m_dataConnections.size(); ++i)
        m_dataConnections[i]->enable();
}
