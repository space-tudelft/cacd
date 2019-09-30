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
#include "src/spock/src/guibuilder/GUIBuilderVisitor.h"
#include "src/spock/src/guibuilder/GUITree.h"
#include "src/spock/src/datastruct/Component.h"
#include "src/spock/src/datastruct/ComponentTree.h"
#include "src/spock/src/widgets/SpreadSheet/SpreadSheet.h"
#include "src/spock/src/widgets/SectionWidget/SectionWidget.h"
#include "src/spock/src/datastruct/Keywords.h"

// Qt includes
#include <qvalidator.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qlistview.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qlabel.h>
#include <qfont.h>
#include <qfontmetrics.h>

using namespace std;

//============================================================================
//! Default constructor
CGUIBuilderVisitor::CGUIBuilderVisitor()
{
    m_tree = 0;
}

//============================================================================
//! Returns true if the component type \a type falls in the line edit catagory
/*!
    \return True if the component type \a type falls in the line edit catagory.
    False otherwise.
    Currently, these types are of the line edit catagory:
    strings, reals, integers and identifiers.
 */
bool CGUIBuilderVisitor::isLineEditType(const string& type)
{
    if (type == KEY_STRING)
        return true;

//    if (type == KEY_CONDITIONLIST)
//        return true;

    if (type == KEY_REAL)
        return true;

    if (type == KEY_INTEGER)
        return true;

    if (type == KEY_IDENTIFIER)
        return true;

    return false;
}

//============================================================================
//! Returns true if component \a comp's parent is a spreadsheet.
/*!
    \return True if component \a comp's parent is a spreadsheet. False otherwise.
 */
bool CGUIBuilderVisitor::parentIsASpreadSheet(CComponent* comp)
{
   if (comp->getParent()==0)
       return false;

   CComponent* parent = comp->getParent();

    if (parent->getType() == KEY_SPREADSHEET)
        return true;

    return false;
}

//============================================================================
//! Creates the widget for component \a comp.
/*!
    The component \a comp is analyzed and a suitable widget is created for it
    if required.

    The creation of the widget is delegated to one of the private build...()
    methods.
 */
void CGUIBuilderVisitor::visitComponent(CComponent* comp)
{
    ASSERT(m_tree != 0);
    if (comp == 0)
        return;

    string type = comp->getType();
//  debug("CGUIBuilderVisitor::visitComponent(%s %s)", type.c_str(), comp->getFullContextName().c_str());

    if (m_tree->getWidget(comp) != 0)
        return;

    if (type == KEY_SPREADSHEET)
        buildSpreadSheet(comp);

    if (isLineEditType(type))
        buildLineEdit(comp);

    if (type == KEY_DROPDOWN || type == KEY_COMBOBOX)
        buildCombobox(comp);

    if (type == KEY_PARAMLIST)
        buildParamList(comp);

    if (type == KEY_SECTION)
        buildSection(comp);

    if (type == KEY_MULTISECTION)
        buildMultiSection(comp);

    if (type == KEY_TABPAGE)
        buildTabPage(comp);
}

//============================================================================
//! Builds a validator
/*!
    Just calls CGUITree::buildValidator(). Look there for more information.

    \sa CGUITree::buildValidator(), CGUITree

    \param parent   The widget to create a validator for
    \param type     Type of component

    \return Returns the validator or 0 if unsuccesful.
 */
QValidator* CGUIBuilderVisitor::buildValidator(QWidget* parent, const string& type, CComponent* comp)
{
    ASSERT(m_tree != 0);
    return m_tree->buildValidator(parent, type, comp);
}

//============================================================================
//! Builds a line edit from \a comp
/*!
    \param comp The component describing the edit

    The line edit is build from the information provided by the component \a comp.
    The properties are read and acted upon.

    After the widget ha been created, it is added to the mapping of the
    CGUITree object being built.
 */
void CGUIBuilderVisitor::buildLineEdit(CComponent* comp)
{
    ASSERT(m_tree != 0);
    ASSERT(comp != 0);
    string fullName = comp->getFullContextName();

    //QWidget* parent = parentWidget(comp);
    QLineEdit* edit = new QLineEdit(0, fullName.c_str());
    m_tree->makeMapping(comp, edit);

    QValidator* validator = buildValidator(edit, comp->getType(), comp);
    edit->setValidator(validator);

    string def;
    if (comp->getProp(PROP_DEFAULT, def)) edit->setText(def.c_str());

    QToolTip::add(edit, comp->getProperty(PROP_HINT).c_str());
}

//============================================================================
//! Builds a spreadsheet.
/*!
    \param comp The component describing the spreadsheet.

    The children of the \a comp will become the columns of the spreadsheet.
    Therefore, the children are passed to the buildSpreadSheetColumn() method.

    The spreadsheets onPleaseCreateCell() signal is connected to the CGUITree
    object that is build.

    \sa CGUITree, buildSpreadSheetColumn(), CSpreadSheetView
 */
void CGUIBuilderVisitor::buildSpreadSheet(CComponent* comp)
{
    ASSERT(m_tree != 0);
    ASSERT(comp != 0);
    string fullName = comp->getFullContextName();

    CSpreadSheetView* sheetWidget = new CSpreadSheetView(0, fullName.c_str());

    m_tree->makeMapping(comp, sheetWidget);
    QToolTip::add(sheetWidget, comp->getProperty(PROP_HINT).c_str());

    for (int i=0; i<comp->numChildren(); ++i)
        buildSpreadSheetColumn(sheetWidget, comp->getChild(i));

    sheetWidget->connect(sheetWidget, SIGNAL(pleaseCreateCell(CMultiTypeItem*, int)),
            m_tree, SLOT(onPleaseCreateCell(CMultiTypeItem*, int)));
}

//============================================================================
//! Builds a spreadsheet column
/*!
    \param sheet    The CSpreadSheetView the column will be added to.
    \param comp     The component describing the column.

    A new column is added to the spreadsheet \a sheet. The title and aligment
    is set using the properties found in \a comp.

    \sa buildSpreadSheet(), CSpreadSheetView, CGUITree
 */
void CGUIBuilderVisitor::buildSpreadSheetColumn(CSpreadSheetView* sheet, CComponent* comp)
{
    ASSERT(m_tree != 0);
    ASSERT(sheet != 0);

    string title = comp->getProperty(PROP_TITLE);
    sheet->addColumn(title.c_str());
    int col = sheet->columns()-1;

    string align = comp->getProperty(PROP_ALIGN);
    if (align != "") {
        if (align == "left")
            sheet->setColumnAlignment(col, Qt::AlignLeft);
        if (align == "right")
            sheet->setColumnAlignment(col, Qt::AlignRight);
        if (align == "center")
            sheet->setColumnAlignment(col, Qt::AlignCenter);
    }
    m_tree->makeMapping(comp, 0);
}

//============================================================================
//! Builds a combobox or dropdown.
/*!
    \param comp The component describing the combobox or dropdown.

    Depending on the type of the \a comp parameter a dropdown or combobox is
    created. Properties futher influence the widget.
 */
void CGUIBuilderVisitor::buildCombobox(CComponent* comp)
{
    ASSERT(m_tree != 0);
    ASSERT(comp != 0);
    string fullName = comp->getFullContextName();

    bool editable = false;
    if (comp->getType() == KEY_COMBOBOX) editable = true;
//  if (comp->getType() == KEY_DROPDOWN) editable = false;

    QComboBox* combo = new QComboBox(editable, 0, fullName.c_str());
    m_tree->makeMapping(comp, combo);

QFont font = combo->font();
QFontMetrics fm(font);
combo->setMinimumWidth(16*fm.width('0'));

    string def;
    int nr = comp->getProp(PROP_DEFAULT, def)? -1 : -2;
    int nc = comp->numChildren();
    for (int i=0; i<nc; ++i) {
        CComponent* child = comp->getChild(i);
        combo->insertItem(child->getProperty(PROP_TITLE).c_str());
        if (nr == -1 && def == child->getName()) nr = i;
    }
    if (nr == -2) { if (nc) nr = 0; }
    else { combo->insertItem(def.c_str()); nr = nc; }
    if (nr >= 0) combo->setCurrentItem(nr);

    if (editable) {
        QValidator* validator = buildValidator(combo, comp->getProperty(PROP_VALIDATOR), comp);
        combo->setValidator(validator);
    }

    QToolTip::add(combo, comp->getProperty(PROP_HINT).c_str());
}

//============================================================================
//! Builds a parameter list.
/*!
    \param comp Describes the parameter list widget to create.

    The parameter list widget consists of three columns. The first column
    contains the labels, the second column the input widgets and the third
    another label.

    The children of the parameter list component \a comp describe the rows in
    this widget.
 */
void CGUIBuilderVisitor::buildParamList(CComponent* comp)
{
    ASSERT(m_tree != 0);
    ASSERT(comp != 0);

    string fullName = comp->getFullContextName();

    QFrame* paramFrame = new QFrame(0, fullName.c_str());
    QGridLayout* layout = new QGridLayout(paramFrame, 0, 0, 6);

    m_tree->makeMapping(comp, paramFrame);

    for (int i=0; i<comp->numChildren(); ++i) {
        CComponent* child = comp->getChild(i);

        string labelText = child->getProperty(PROP_TITLE);
        QLabel* label = new QLabel(labelText.c_str(), paramFrame);
        layout->addWidget(label, i, 0, Qt::AlignRight | Qt::AlignVCenter);

        QWidget* childWidget = m_tree->getWidget(child);
	ASSERT(childWidget != 0);
        childWidget->reparent(paramFrame, QPoint(0,0));
        layout->addWidget(childWidget, i, 1, Qt::AlignLeft);

        labelText = child->getProperty(PROP_UNIT);
        QLabel* unitLabel = new QLabel(labelText.c_str(), paramFrame);
        layout->addWidget(unitLabel, i, 2, Qt::AlignLeft | Qt::AlignVCenter);

        layout->setColStretch(2, 1);
    }
}

//============================================================================
//! Builds a section widget.
/*!
    \param comp The component describing the section.

    The section widget can have an icon, title and hint set. The children of
    the section widget are added vertically to the area displayed when the
    section is expanded.

    \sa CSectionWidget
 */
void CGUIBuilderVisitor::buildSection(CComponent* comp)
{
    ASSERT(m_tree != 0);
    ASSERT(comp != 0);
    string fullName = comp->getFullContextName();

    CSectionWidget* sectionWidget = new CSectionWidget(0, comp->getFullContextName().c_str());
    QFrame*         content = new QFrame(sectionWidget, comp->getFullContextName().c_str());
    QBoxLayout*     layout = new QVBoxLayout(content,6);
    m_tree->makeMapping(comp, sectionWidget);

    sectionWidget->setLabel(comp->getProperty(PROP_TITLE).c_str());
    if (comp->getProperty(PROP_PIXMAP) != "")
        sectionWidget->setPixmap(comp->getProperty(PROP_PIXMAP).c_str());
    sectionWidget->setContentWidget(content);
    QToolTip::add(sectionWidget, comp->getProperty(PROP_HINT).c_str());

    for (int i=0; i<comp->numChildren(); ++i) {
        QWidget* child = m_tree->getWidget(comp->getChild(i));
        if (child != 0) {
            child->reparent(content, QPoint(0,0));
            layout->addWidget(child);
        }
    }
}

//============================================================================
//! Builds a scrollable area
/*!
    The name of this method is somewhat misleading. It has historically grown
    this way and should be renamed in a future version.

    \param comp The component describing the scrollframe.

    The children of \a comp are added vertically to the frame. If the
    contents of the frame no longer fit inside the frame, the frame will
    display scroll bars.
 */
void CGUIBuilderVisitor::buildMultiSection(CComponent* comp)
{
    ASSERT(m_tree != 0);
    ASSERT(comp != 0);
    string fullName = comp->getFullContextName();

    CMultiSectionWidget* multiWidget = new CMultiSectionWidget(0, fullName.c_str());
    m_tree->makeMapping(comp, multiWidget);

    for (int i=0; i<comp->numChildren(); ++i) {
        QWidget* childWidget = m_tree->getWidget(comp->getChild(i));
        if (childWidget != 0) {
            childWidget->reparent(multiWidget, QPoint(0,0));
            multiWidget->addWidget(childWidget);
        }
    }
    QToolTip::add(multiWidget, comp->getProperty("hint").c_str());
}

//============================================================================
//! Builds a tabpage
/*!
    \param comp The component describing the tabpage.

    The children of the tabpage are added vertically to the contents of the
    tabpage.
 */
void CGUIBuilderVisitor::buildTabPage(CComponent* comp)
{
    ASSERT(m_tree != 0);
    ASSERT(comp != 0);
    string fullName = comp->getFullContextName();

    QFrame*     tabFrame  = new QFrame(0, comp->getFullContextName().c_str());
    QBoxLayout* tabLayout = new QVBoxLayout(tabFrame);

    tabFrame->setCaption(comp->getProperty(PROP_TITLE).c_str());
    m_tree->makeMapping(comp, tabFrame);

    for (int i=0; i<comp->numChildren(); ++i) {
        QWidget* child = m_tree->getWidget(comp->getChild(i));
        child->reparent(tabFrame, QPoint(0,0));
        tabLayout->addWidget(child);
        child->updateGeometry();
    }
}

//============================================================================
//! Builds a CGUITree object from a given component tree
/*!
    \param CComponentTree   The component tree containing the required information.

    After the tree has been created, the data connections are build by calling
    the CGUITree::buildDataConnections() method.

    \return The CGUITree object requested.

    \sa CComponentTree, CGUITree, CGUITree::buildDataConnections()
 */
CGUITree* CGUIBuilderVisitor::buildGUI(CComponentTree* compTree)
{
    ASSERT(compTree != 0);

    m_tree = new CGUITree(compTree);

    vector<CComponent*> roots = compTree->getRootComponents();

    for (unsigned int i=0; i<roots.size(); ++i)
        roots[i]->accept(*this);

    m_tree->buildDataConnections();

    return m_tree;
}
