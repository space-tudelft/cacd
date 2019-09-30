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
#include "src/spock/src/gui/dialogs/ConditionListDlg/ConditionListDlg.h"
#include "src/spock/src/widgets/TriStateButton/TristateButton.h"
#include "src/spock/src/helper/log/Helper.h"

// Qt includes
#include <qlayout.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qframe.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlistbox.h>

// STL includes
#include <algorithm>

using namespace std;

static int e_onChanged = 1;

//============================================================================
//! Constructor
/*!
    \param parent   The parent widget.
    \param names    A vector containing the layer names
    \param comments A vector containing the layer descriptions
    \param extended True if three columns of buttons should be used, false otherwise.
    \param name     The QObject internal name for this instance.

    \a names and \a comments do not have to be the same size. \a comments is made
    the same size as \a names by the constructor.
 */
CConditionListDlg::CConditionListDlg(QWidget* parent,
                                     const vector<string>& names,
                                     const vector<string>& comments,
                                     bool extended,
                                     const char* name)
    : QDialog(parent, name, true)
{
    setCaption ("Condition list editor");

    // Set the data
    m_names = names;
    m_comments = comments;
    m_extended = extended;

    int mandatorySize = m_names.size();
    m_comments.resize(mandatorySize, "");

    // Create a main layout and add the legend, input area, status displays and buttons.
    m_layout = new QVBoxLayout(this, 4, 4);

    buildLegend();

    m_horizFrame = new QFrame(this);
    m_horizontal = new QHBoxLayout(m_horizFrame, 4, 4);
    m_layout->addWidget(m_horizFrame);

    buildLayers();
    buildListBox();
//    buildCurrentEdit();
//    buildButtons();
    buildStatus();

    // Some buttons to leave the dialog
    QFrame* buttonFrame = new QFrame(this);
    QBoxLayout* buttonLayout = new QHBoxLayout(buttonFrame, 4, 4);
    buttonLayout->setAutoAdd(true);
    buttonLayout->addStretch(1);

    QPushButton* okButton = new QPushButton("OK", buttonFrame);
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));

    QPushButton* cancelButton = new QPushButton("Cancel", buttonFrame);
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

    buttonLayout->addStretch(1);

    m_layout->addWidget(buttonFrame);
}

//============================================================================
//! Builds the legend used by the dialog box.
/*!
    The legend displays the possible states of the buttons and their meaning.
 */
void CConditionListDlg::buildLegend()
{
    // The legend
    QGroupBox* legend = new QGroupBox(6, Qt::Horizontal, "Legend", this);

    CTristateButton* button = new CTristateButton(QButton::On, legend);
    button->setEnabled(false);
    new QLabel("present", legend);

    button = new CTristateButton(QButton::Off, legend);
    button->setEnabled(false);
    new QLabel("absent", legend);

    button = new CTristateButton(QButton::NoChange, legend);
    button->setEnabled(false);
    new QLabel("don't care", legend);

    m_layout->addWidget(legend);
}

//============================================================================
//! Builds the table with the layer names and comments
void CConditionListDlg::buildLayers()
{
    // Setup the grid (input area)
    m_gridFrame = new QFrame(m_horizFrame);
    m_horizontal->addWidget(m_gridFrame, 1);
    m_gridLayout = new QGridLayout(m_gridFrame, 0, 0, 4, 4);

    // Add some labels and lines
    QLabel* label = new QLabel("Layer", m_gridFrame);
    m_gridLayout->addWidget(label, 0, 0);

    if (m_extended) {
        label = new QLabel("one side:", m_gridFrame);
        m_gridLayout->addWidget(label, 0, 2, Qt::AlignCenter);
    }

    label = new QLabel("tile:", m_gridFrame);
    m_gridLayout->addWidget(label, 0, 4, Qt::AlignCenter);

    if (m_extended) {
        label = new QLabel("other side:", m_gridFrame);
        m_gridLayout->addWidget(label, 0, 6, Qt::AlignCenter);
    }

    QFrame* line = new QFrame(m_gridFrame);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    m_gridLayout->addMultiCellWidget(line, 1, 1, 0, 6);

    if (m_extended) {
        line = new QFrame(m_gridFrame);
        line->setFrameStyle(QFrame::VLine | QFrame::Sunken);
        m_gridLayout->addMultiCellWidget(line, 2, 2+m_names.size(), 3, 3);

        line = new QFrame(m_gridFrame);
        line->setFrameStyle(QFrame::VLine | QFrame::Sunken);
        m_gridLayout->addMultiCellWidget(line, 2, 2+m_names.size(), 5, 5);
    }

    // Add the layers
    for (unsigned int i=0; i<m_names.size(); ++i) {
        label = new QLabel(m_names[i].c_str(), m_gridFrame);
        m_gridLayout->addWidget(label, 2 + i, 0);
        //debug("Comment for layer %s (%d) = %s", m_names[i].c_str(), i, m_comments[i].c_str());
        label = new QLabel(m_comments[i].c_str(), m_gridFrame);
        m_gridLayout->addWidget(label, 2 + i, 1);

        if (m_extended) {
            CTristateButton* button = new CTristateButton(QButton::NoChange, m_gridFrame);
            m_gridLayout->addWidget(button, 2 + i, 2);
            m_left.push_back(button);
        }

        CTristateButton* button = new CTristateButton(QButton::NoChange, m_gridFrame);
        m_gridLayout->addWidget(button, 2 + i, 4);
        m_tile.push_back(button);

        if (m_extended) {
            button = new CTristateButton(QButton::NoChange, m_gridFrame);
            m_gridLayout->addWidget(button, 2 + i, 6);
            m_right.push_back(button);
        }
    }

    // Respond to changes
    for (unsigned int i=0; i<m_tile.size(); ++i) {
        if (m_extended) {
            connect(m_left[i], SIGNAL(stateChanged(int)), this, SLOT(onChanged()));
            connect(m_right[i], SIGNAL(stateChanged(int)), this, SLOT(onChanged()));
        }
        connect(m_tile[i], SIGNAL(stateChanged(int)), this, SLOT(onChanged()));
    }
}

//============================================================================
//! Builds the listbox containing the expressions.
void CConditionListDlg::buildListBox()
{
    QFrame* vert = new QFrame(m_horizFrame);
    QBoxLayout* lay = new QVBoxLayout(vert);
    m_horizontal->addWidget(vert, 1);

    // Label
    QLabel* title = new QLabel("OR expressions:", vert);
    lay->addWidget(title);

    // Listbox
    m_orList = new QListBox(vert);
    lay->addWidget(m_orList);
    connect(m_orList, SIGNAL(clicked(QListBoxItem*)), this, SLOT(onClickedListBox(QListBoxItem*)));

    // Buttons
    QFrame* buttonFrame = new QFrame(vert);
    QBoxLayout* buttonLay = new QHBoxLayout(buttonFrame);
    buttonLay->setAutoAdd(true);
    lay->addWidget(buttonFrame);

    QPushButton* buttonAddOR = new QPushButton("New expression", buttonFrame);
    connect(buttonAddOR, SIGNAL(clicked()), this, SLOT(onButtonAddOR()));

    QPushButton* buttonDel = new QPushButton("Delete expression", buttonFrame);
    connect(buttonDel, SIGNAL(clicked()), this, SLOT(onButtonDel()));
}

//============================================================================
//! Builds the state buttons
void CConditionListDlg::buildButtons()
{
    // Add some buttons
    QWidget* buttonFrame = new QWidget(this);
    QBoxLayout* buttonFrameLayout = new QHBoxLayout(buttonFrame);
    buttonFrameLayout->setAutoAdd(true);
    buttonFrameLayout->addStretch(1);
    QPushButton* buttonSet = new QPushButton("set", buttonFrame);
    QPushButton* buttonAdd = new QPushButton("add", buttonFrame);
    connect(buttonSet, SIGNAL(clicked()), this, SLOT(onButtonSet()));
    connect(buttonAdd, SIGNAL(clicked()), this, SLOT(onButtonAdd()));

    m_layout->addWidget(buttonFrame);
}

//============================================================================
//! Builds the edit containing the resulting condition list.
/*!
    Obsolete.
 */
void CConditionListDlg::buildCurrentEdit()
{
    QWidget* frame = new QWidget(this);
    QBoxLayout* lay = new QHBoxLayout(frame);
    lay->setAutoAdd(true);
//    lay->addStretch(2);
    m_currentEdit = new QLineEdit(frame);
    m_currentEdit->setEnabled(false);

    m_layout->addWidget(frame);
}

//============================================================================
//! Builds the edit in which the condition list is entered and edited.
void CConditionListDlg::buildStatus()
{
    // The conditionlist result edit
    m_layout->addWidget(new QLabel("Condition list:", this));
    m_edit = new QLineEdit(this);
    m_layout->addWidget(m_edit);
}

//============================================================================
//! Retrieves the current conditon list as edited in the dialog box.
/*!
    The buttons and the listbox are scanned and the resulting condition list
    is returned.
 */
string CConditionListDlg::getCurrentList()
{
    string condList = "(";

    for (unsigned int i=0; i<m_tile.size(); ++i) {
        string layName = m_names[i];

        if (m_extended) {
            if (m_left[i]->state() == QButton::On)
		condList += " -" + layName;
            if (m_left[i]->state() == QButton::Off)
		condList += " !-" + layName;
        }

        if (m_tile[i]->state() == QButton::On)
	    condList += " " + layName;
        if (m_tile[i]->state() == QButton::Off)
	    condList += " !" + layName;

        if (m_extended) {
            if (m_right[i]->state() == QButton::On)
		condList += " =" + layName;
            if (m_right[i]->state() == QButton::Off)
		condList += " !=" + layName;
        }
    }
    condList += " )";

    return condList;
}

//============================================================================
//! Called when the set button is pressed
void CConditionListDlg::onButtonSet()
{
    string condList = getCurrentList();
    m_edit->setText(condList.c_str());
}

//============================================================================
//! Called when the add button is pressed
void CConditionListDlg::onButtonAdd()
{
    string condList = getCurrentList();
    QString tmp = m_edit->text();
    tmp += " ";
    tmp += condList.c_str();
    m_edit->setText(tmp);
}

//============================================================================
//! Called when the add as or button is pressed
void CConditionListDlg::onButtonAddOR()
{
    m_orList->insertItem("()");
    m_orList->setCurrentItem(m_orList->count()-1);
    onClickedListBox(m_orList->item(m_orList->count()-1));
    /*
    string condList = getCurrentList();
    QString tmp = m_edit->text();
    tmp += " | ";
    tmp += condList.c_str();
    m_edit->setText(tmp);
    */
}

//============================================================================
//! Called when the delete button is pressed
void CConditionListDlg::onButtonDel()
{
    m_orList->removeItem(m_orList->currentItem());
    int index = m_orList->currentItem();
    onClickedListBox(m_orList->item(index));
}

//============================================================================
//! Called when something in the dialog box changes.
/*!
    This updates the current condition list displayed in the boxes, the buttons
    and the result edit.
 */
void CConditionListDlg::onChanged()
{
    if (!e_onChanged) return;
    string condList = getCurrentList();

    if (m_orList->count() == 0) {
        m_orList->insertItem(condList.c_str());
        m_orList->setCurrentItem(0);
    } else {
        m_orList->changeItem(condList.c_str(), m_orList->currentItem());
    }

    QString complete = "";
    for (unsigned int i=0; i<m_orList->count(); ++i) {
        complete += m_orList->text(i);
        if (i != m_orList->count()-1)
            complete += " | ";
    }
    m_edit->setText(complete);
}

//============================================================================
//! Called when the listbox is clicked
void CConditionListDlg::onClickedListBox(QListBoxItem* item)
{
    if (item != 0)
        onSetButtons(item->text());
}

//============================================================================
//! Returns the condition list currently entered into the condition list dialog.
/*!
    \return The condition list currently entered into the condition list dialog.
 */
string CConditionListDlg::getConditions()
{
    if (!m_edit->text().isNull())
        return m_edit->text().latin1();
    return "";
}

//============================================================================
//! Sets the current condition list.
/*!
    \param The condition list.

    The given condition list \a cond is parsed and the widgets of the dialog box
    are updated to represent the result.
 */
void CConditionListDlg::setConditions(const QString& cond)
{
    e_onChanged = 0;
//  // First, clear all conditions.
//  for (unsigned int i=0; i<m_tile.size(); ++i) {
//      if (m_extended) {
//          m_left[i]->changeState(QButton::NoChange);
//          m_right[i]->changeState(QButton::NoChange);
//      }
//      m_tile[i]->changeState(QButton::NoChange);
//  }

    // Now, analyze the given string and set the states.
    string expr = "";
    if (!cond.isNull())
        expr = cond.latin1();

    debug("Analyzing [%s]", expr.c_str());

    string complete = "";
    vector<string> res = splitString(expr, "|");

    if (res[0] != "")
    for (unsigned int part = 0; part<res.size(); ++part) {
	unsigned int i, n;
	vector<string> orx = splitString(res[part], " ");
	vector<string> Or;
	for (i=0; i<orx.size(); ++i)
	    if (orx[i] != "") Or.push_back(orx[i]);
	if (Or.size() == 0) break;
	i = Or.size() - 1;
	n = Or[i].length() - 1;
	if (Or[i].rfind(')') == n) {
	    if (n > 0) {
		Or[i] = Or[i].substr(0,n);
		Or.push_back(")");
	    }
	}
	else Or.push_back(")");

	string orexp = "(";
	i = 0;
	if (Or[i].find('(') == 0) {
	    if (Or[i].length() > 1)
		orexp += " " + Or[i].substr(1);
	    i = 1;
	}
	for (; i<Or.size(); ++i) orexp += " " + Or[i];
        m_orList->insertItem(orexp.c_str());
	if (part == 0) {
	    complete = orexp;
	    onSetButtons(orexp.c_str());
	}
	else
	    complete += " | " + orexp;
    }
    m_orList->setCurrentItem(0);

    if (complete != "") m_edit->setText(complete.c_str());
    e_onChanged = 1;
}

//============================================================================
//! Sets the buttons of the current expression
/*!
    \param expr An expression without OR's, e.g.  cmf !cms -caa =caa
 */
// expr = something ANDed like cmf !cms -caa =caa
void CConditionListDlg::onSetButtons(const QString& expr)
{
    string tmp = "";
    vector<string> res;
    if (!expr.isNull())
        tmp = expr.latin1();

    string::iterator pos;
    pos = remove(tmp.begin(), tmp.end(), '(');
    tmp.erase(pos, tmp.end());
    pos = remove(tmp.begin(), tmp.end(), ')');
    tmp.erase(pos, tmp.end());

    res  = splitString(tmp, " ");

    for (unsigned int i=0; i<m_tile.size(); ++i) {
        QButton::ToggleState state = QButton::NoChange;

        debug("Clearing buttons for %s", m_names[i].c_str());

        if (m_extended) {
            m_left[i]->changeState(state);
            m_right[i]->changeState(state);
        }
        m_tile[i]->changeState(state);

        for (unsigned int j=0; j<res.size(); ++j) {
            debug("res[j] = [%s]", res[j].c_str());
            bool left = false;
            bool right = false;
            bool tile = false;
            bool switchOn = false;

            // extract switches
            if (res[j].find("-") != string::npos)
                left = true;
            if (res[j].find("=") != string::npos)
                right = true;
            if (!left && !right)
                tile = true;

            if (res[j].find("!") != string::npos)
                switchOn = false;
            else
                switchOn = true;

            // extract exact name
            string resName = res[j];
            string::iterator pos;
	if (left) {
            pos = std::find(resName.begin(), resName.end(), '-');
            resName.erase(pos);
	}
	if (right) {
            pos = std::find(resName.begin(), resName.end(), '=');
            resName.erase(pos);
	}
	if (!switchOn) {
            pos = std::find(resName.begin(), resName.end(), '!');
            resName.erase(pos);
	}

            debug("layName == resName : [%s] == [%s]", m_names[i].c_str(), resName.c_str());

            string layName = m_names[i];
            if (resName == layName) {
                if (switchOn)
                    state = QButton::On;
                else
                    state = QButton::Off;

                if (m_extended && left)
                    m_left[i]->changeState(state);
                if (m_extended && right)
                    m_right[i]->changeState(state);
                if (tile)
                    m_tile[i]->changeState(state);
            }
        }
    }
}
