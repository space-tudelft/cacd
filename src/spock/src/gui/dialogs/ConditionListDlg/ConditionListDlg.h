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
#ifndef __CONDITIONLISTDLG_H__
#define __CONDITIONLISTDLG_H__

// Qt includes
#include <qdialog.h>

// Project includes
#include <vector>
#include <string>
#include <map>

using namespace std;

// Forward declarations
class QBoxLayout;
class QFrame;
class QLineEdit;
class QGridLayout;
class QListBox;
class QListBoxItem;
class CTristateButton;

//! A dialog box to edit condition lists.
/*!
    A condition list can be created and edited with this dialog box. The dialog
    box consist of an area with the layer names and descriptions, the buttons
    that represent the absence/presence/don't care state of the layers and a
    listbox of expressions that are OR-ed together. The resulting condition
    list is diaplayed near the bottom of the dialog box.

    \image html conditionlist.jpg
    \image latex conditionlist.eps "The conditionlist editor" width=10cm

    \author Xander Burgerhout
*/
class CConditionListDlg : public QDialog
{
    Q_OBJECT

    private:
	vector<string>              m_names;
	vector<string>              m_comments;
	bool                        m_extended;

	vector<CTristateButton*>    m_left;
	vector<CTristateButton*>    m_tile;
	vector<CTristateButton*>    m_right;

	QBoxLayout*     m_layout;
	QBoxLayout*     m_horizontal;
	QFrame*         m_horizFrame;
	QFrame*         m_gridFrame;
	QGridLayout*    m_gridLayout;
	QLineEdit*      m_edit;
	QLineEdit*      m_currentEdit;
	QListBox*       m_orList;

	void            buildLegend();
	void            buildLayers();
	void            buildListBox();
	void            buildButtons();
	void            buildCurrentEdit();
	void            buildStatus();

	string          getCurrentList();

    private slots:
	void            onButtonSet();
	void            onButtonAdd();
	void            onButtonAddOR();
	void            onButtonDel();
	void            onChanged();
	void            onSetButtons(const QString& expr);
	void            onClickedListBox(QListBoxItem* item);

    public:
	CConditionListDlg(QWidget* parent, const vector<string>& names,
		const vector<string>& comments, bool extended = true, const char* name = 0);

	string          getConditions();
	void            setConditions(const QString& cond);
};

#endif // __CONDITIONLISTDLG_H__
