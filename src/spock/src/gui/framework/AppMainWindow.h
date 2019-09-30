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
#ifndef __APPMAINWINDOW_H__
#define __APPMAINWINDOW_H__

// Qt includes
#include <qmainwindow.h>
#include <qmenubar.h>
#include <qtoolbar.h>
#include <qstatusbar.h>

// STL includes
#include <string>
#include <vector>

using namespace std;

// Forward declarations
class CStatusIndicator;

//! The application main window class.
/*!
    The main window of the application consists of the menu bar, tool bar,
    work area and status bar.

    The actions related to these areas are set up and controlled by this class.

    \image html appmainwindow.jpg
    \image latex appmainwindow.eps "The application" width=10cm

    \author Xander Burgerhout
*/
class CAppMainWindow : public QMainWindow
{
    Q_OBJECT

    private:
	// Main window widgets
	QMenuBar*           m_menuBar;
	QToolBar*           m_toolBar;
	QToolBar*           m_editBar;
	QStatusBar*         m_statusBar;
	CStatusIndicator*   m_processIndicator;

	QPopupMenu*         m_processMenu;

	void                setupActions();
	void                setupStatusBar();
	void                rebuildProcessMenu();

	void                saveGeneratedFile(const string& dir,
				const string& name, const string& contents);


    public:
	CAppMainWindow(QWidget* pParent);
	~CAppMainWindow();

	void                updateTitleBar();
	void 		    closeEvent ( QCloseEvent * e );

    public slots:
	void                onFileNewProcess();
	void                onFileRenameProcess();
	void                onFileLoad();
	void                onFileSave();
	void                onFileSaveAs();
	void                onFileClose();
	void                onFileQuit();

	void                onEditGenerate();
	void                onClickedProcessMenu(int menuID);

	void                onEditBarAddRow();
	void                onEditBarDelRow();
	void                onEditBarInsAboveRow();
	void                onEditBarInsBelowRow();
	void                onEditBarMoveRowUp();
	void                onEditBarMoveRowDown();

	void                onHelpAbout();
	void                onHelpAboutQt();
};

#endif // __APPMAINWINDOW_H__
