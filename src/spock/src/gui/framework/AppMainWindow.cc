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

// Qt includes
#include <qapplication.h>
#include <qframe.h>
#include <qaction.h>
#include <qtoolbar.h>
#include <qmenubar.h>
#include <qwidgetstack.h>
#include <qiconset.h>
#include <qfiledialog.h>
#include <qpixmap.h>
#include <qfocusdata.h>
#include <qmessagebox.h>
#include <qtabdialog.h>
#include <qmultilineedit.h>
#include <qfile.h>
#include <qtextstream.h>

// Project includes
#include "src/spock/src/gui/framework/AppMainWindow.h"
#include "src/spock/src/widgets/StatusIndicator/StatusIndicator.h"
#include "src/spock/src/widgets/SpreadSheet/SpreadSheet.h"
#include "src/spock/src/gui/framework/ProcessManager.h"
#include "src/spock/src/guibuilder/Process.h"
#include "src/spock/src/gui/dialogs/ProcessDescDlg/ProcessDescDlg.h"
#include "src/spock/src/gui/dialogs/GenerateDlg/GenerateDlg.h"
#include "src/spock/src/gui/dialogs/AboutDlg/AboutDlg.h"
#include "src/spock/src/gui/framework/icons.h"

// STL includes
#include <algorithm>

using namespace std;

//============================================================================
//! Constructor
/*!
    The constructor creates the widget stack used for the work area.
    It also calls setupActions() and setupStatusBar() to create and setup the
    menu bar,  toolbar  and status bar.

    \sa CProcessManager
 */
CAppMainWindow::CAppMainWindow(QWidget* pParent)
    : QMainWindow(pParent, "CAppMainWindow")
{
    m_menuBar       = 0;
    m_toolBar       = 0;
    m_statusBar     = 0;
    m_processIndicator = 0;

    QWidgetStack* stack = CProcessManager::instance()->getWidgetStack();
    stack->reparent(this, QPoint(0,0));
    debug("Setting up the central widget...");
    setCentralWidget(stack);

    debug("Setting up the menu, toolbar & statusbar...");
    setupActions();
    setupStatusBar();
}

//============================================================================
//! Destructor
CAppMainWindow::~CAppMainWindow()
{
}

//============================================================================
//! Sets up the menubar and toolbar.
/*!
    The mene bar and toolbar are setup using the QAction class. This class
    allows actions to be added to the menu bar and toolbar.
 */
void CAppMainWindow::setupActions()
{
    debug("Creating menubar...");
    m_menuBar = new QMenuBar(this);
    m_toolBar = new QToolBar(this);
    m_editBar = new QToolBar(this);

    addToolBar(m_toolBar);
    addToolBar(m_editBar);

    // -----------First, define the actions:

    // File->New
    QAction* newProc = new QAction(this);
    QPixmap* pm = new QPixmap( (const char**)icons::filenew_xpm );
    QIconSet* icon = new QIconSet(*pm);
    newProc->setIconSet(*icon);
    newProc->setMenuText("&New process...");
    newProc->setText("New process...");

    // File->Rename Process
    QAction* renProc = new QAction(this);
    renProc->setText("&Rename process...");

    // File->Open
    QAction* loadProc = new QAction(this);
    pm = new QPixmap((const char**)icons::fileopen_xpm);
    icon = new QIconSet(*pm);
    loadProc->setIconSet(*icon);
    loadProc->setMenuText("&Open process...");
    loadProc->setText("Open process...");

    // File->Save
    QAction* saveProc = new QAction(this);
    pm = new QPixmap((const char**)icons::filesave_xpm);
    icon = new QIconSet(*pm);
    saveProc->setIconSet(*icon);
    saveProc->setMenuText("&Save process");
    saveProc->setText("Save process");

    // File->Save as
    QAction* saveProcAs = new QAction(this);
    pm = new QPixmap((const char**)icons::filesaveas_xpm);
    icon = new QIconSet(*pm);
    saveProcAs->setIconSet(*icon);
    saveProcAs->setMenuText("Save &As process...");
    saveProcAs->setText("Save As process...");

    // File->Close process
    QAction* closeProc = new QAction(this);
    pm = new QPixmap((const char**)icons::fileclose_xpm);
    icon = new QIconSet(*pm);
    closeProc->setIconSet(*icon);
    closeProc->setMenuText("&Close process");
    closeProc->setText("Close process");

    // File->Quit
    QAction* quitApp = new QAction(this);
    pm = new QPixmap((const char**)icons::exit_xpm);
    icon = new QIconSet(*pm);
    quitApp->setIconSet(*icon);
    quitApp->setMenuText("&Quit");
    quitApp->setText("Quit");

    // Edit->Generate
    QAction* generate = new QAction(this);
    pm = new QPixmap((const char**)icons::editgenerate_xpm);
    icon = new QIconSet(*pm);
    generate->setIconSet(*icon);
    generate->setMenuText("&Generate process files...");
    generate->setText("Generate process...");

    // Help->About
    QAction* about = new QAction(this);
    pm = new QPixmap((const char**)icons::info_xpm);
    icon = new QIconSet(*pm);
    about->setIconSet(*icon);
    about->setMenuText("&About...");
    about->setText("About...");

    // Help->About Qt
    QAction* aboutQt = new QAction(this);
    aboutQt->setText("About &Qt...");

    // Toolbar moveRowUp
    QAction* moveRowUp = new QAction(this);
    pm = new QPixmap((const char**)icons::rowup_xpm);
    icon = new QIconSet(*pm);
    moveRowUp->setIconSet(*icon);
    moveRowUp->setText("Row up");

    // Toolbar moveRowDown
    QAction* moveRowDown = new QAction(this);
    pm = new QPixmap((const char**)icons::rowdown_xpm);
    icon = new QIconSet(*pm);
    moveRowDown->setIconSet(*icon);
    moveRowDown->setText("Row down");

    // Toolbar deleteRow
    QAction* delRow = new QAction(this);
    pm = new QPixmap((const char**)icons::rowdelete_xpm);
    icon = new QIconSet(*pm);
    delRow->setIconSet(*icon);
    delRow->setText("Delete row");

    // Toolbar insertAboveRow
    QAction* insAboveRow = new QAction(this);
    pm = new QPixmap((const char**)icons::rowinsertabove_xpm);
    icon = new QIconSet(*pm);
    insAboveRow->setIconSet(*icon);
    insAboveRow->setText("Insert row above");

    // Toolbar insertBelowRow
    QAction* insBelowRow = new QAction(this);
    pm = new QPixmap((const char**)icons::rowinsertbelow_xpm);
    icon = new QIconSet(*pm);
    insBelowRow->setIconSet(*icon);
    insBelowRow->setText("Insert row below");

    // Toolbar addRow
    QAction* addRow = new QAction(this);
    pm = new QPixmap((const char**)icons::rowadd_xpm);
    icon = new QIconSet(*pm);
    addRow->setIconSet(*icon);
    addRow->setText("Add row");

    // ---------Create the menus:

    // The file menu
    QPopupMenu* fileMenu = new QPopupMenu;
    // The Edit menu
    QPopupMenu* editMenu = new QPopupMenu;
    // The Process menu
    m_processMenu = new QPopupMenu;
    // The help menu
    QPopupMenu* helpMenu = new QPopupMenu;

    // ---------Add the actions to the menus
                                    // File menu:
    newProc->addTo(fileMenu);       // New
    renProc->addTo(fileMenu);       // Rename
    loadProc->addTo(fileMenu);      // Open
    fileMenu->insertSeparator();    // -----------
    saveProc->addTo(fileMenu);      // Save
    saveProcAs->addTo(fileMenu);    // Save as...
    fileMenu->insertSeparator();    // -----------
    closeProc->addTo(fileMenu);     // Close
    quitApp->addTo(fileMenu);       // Quit

                                    // Edit menu:
    generate->addTo(editMenu);      // Generate

                                    // Help menu:
    about->addTo(helpMenu);         // About...
    aboutQt->addTo(helpMenu);       // About Qt...

    // ---------Add the actions to the toolbar
    quitApp->addTo(m_toolBar);
    m_toolBar->addSeparator();
    newProc->addTo(m_toolBar);
    loadProc->addTo(m_toolBar);
    closeProc->addTo(m_toolBar);
    saveProc->addTo(m_toolBar);
    saveProcAs->addTo(m_toolBar);
    m_toolBar->addSeparator();
    generate->addTo(m_toolBar);
    m_toolBar->addSeparator();
    about->addTo(m_toolBar);

    addRow->addTo(m_editBar);
    insAboveRow->addTo(m_editBar);
    insBelowRow->addTo(m_editBar);
    delRow->addTo(m_editBar);
    m_editBar->addSeparator();
    moveRowUp->addTo(m_editBar);
    moveRowDown->addTo(m_editBar);

    // ---------Connect the actions to their slots
    connect(newProc, SIGNAL(activated()), SLOT(onFileNewProcess()));
    connect(renProc, SIGNAL(activated()), SLOT(onFileRenameProcess()));
    connect(loadProc, SIGNAL(activated()), SLOT(onFileLoad()));
    connect(saveProc, SIGNAL(activated()), SLOT(onFileSave()));
    connect(saveProcAs, SIGNAL(activated()), SLOT(onFileSaveAs()));
    connect(closeProc, SIGNAL(activated()), SLOT(onFileClose()));
    connect(quitApp, SIGNAL(activated()), SLOT(onFileQuit()));

    connect(generate, SIGNAL(activated()), SLOT(onEditGenerate()));

    connect(addRow, SIGNAL(activated()), SLOT(onEditBarAddRow()));
    connect(delRow, SIGNAL(activated()), SLOT(onEditBarDelRow()));
    connect(insAboveRow, SIGNAL(activated()), SLOT(onEditBarInsAboveRow()));
    connect(insBelowRow, SIGNAL(activated()), SLOT(onEditBarInsBelowRow()));
    connect(moveRowDown, SIGNAL(activated()), SLOT(onEditBarMoveRowDown()));
    connect(moveRowUp, SIGNAL(activated()), SLOT(onEditBarMoveRowUp()));

    connect(about, SIGNAL(activated()), SLOT(onHelpAbout()));
    connect(aboutQt, SIGNAL(activated()), SLOT(onHelpAboutQt()));

    // ---------Insert the menus into the menubar
    m_menuBar->insertItem("&File", fileMenu);
    m_menuBar->insertItem("&Edit", editMenu);
    rebuildProcessMenu();
    m_menuBar->insertItem("&Process", m_processMenu);
    m_menuBar->insertSeparator();
    m_menuBar->insertItem("&Help", helpMenu);
}

//============================================================================
//! Sets up the statusbar.
/*!
    Currently, the status bar only contains an indicator widget indicating the
    current active process.
 */
void CAppMainWindow::setupStatusBar()
{
    // Create a statusbar widget.
    m_statusBar = statusBar();

    // We want a process indicator in it
    m_processIndicator = new CStatusIndicator(m_statusBar, "main process indicator");
    m_statusBar->addWidget(m_processIndicator, 0, true);
}

//============================================================================
//! Rebuilds the process menu.
/*!
    The process menu contains the currently loaded processes. A loaded process
    can be activated and made current by selecting it in this menu.
 */
void CAppMainWindow::rebuildProcessMenu()
{
    ASSERT(m_processMenu != 0);
    vector<string> processNames = CProcessManager::instance()->getProcessNames();

    sort(processNames.begin(), processNames.end());

    m_processMenu->clear();
    for (unsigned int i=0; i<processNames.size(); ++i) {
        QString name = processNames[i].c_str();
        m_processMenu->insertItem(name, this, SLOT(onClickedProcessMenu(int)));
    }
    if (processNames.size() == 0) {
        int ID = m_processMenu->insertItem("There are no processes");
        m_processMenu->setItemEnabled(ID, false);
    }
}

//============================================================================
//! Application close event.
/*!
    The user has clicked on the main window close (exit) button.
 */
void CAppMainWindow::closeEvent( QCloseEvent * e )
{
    onFileQuit();
}

//============================================================================
//! Creates a new empty process.
/*!
    Before the process is created, the name and description of the process must
    be enetered. The name of the new process cannot be the name of a process
    currently loaded in the application.
 */
void CAppMainWindow::onFileNewProcess()
{
    CProcessDescDlg dlg(0);
    dlg.setCaption("New process");

    if (dlg.exec() == QDialog::Accepted) {
        string name = dlg.getProcessName();
	if (name == "") {
	    QMessageBox::information(0, "Whoops!", "No process name specified.\n", "OK");
	}
	else if (CProcessManager::instance()->isLoaded(name)) {
            QMessageBox::information(0, "Whoops!","A process with that name is already open.\n"
                    "If you wish to create a new process with this name,\n"
                    "please close the other process first.", "OK");
        } else {
	    QApplication::setOverrideCursor(Qt::waitCursor);
            CProcess* proc = CProcessManager::instance()->newProcess(name);
            proc->setDesc(dlg.getProcessDesc());
            CProcessManager::instance()->activateProcess(proc);
            rebuildProcessMenu();
            updateTitleBar();
	    QApplication::restoreOverrideCursor();
        }
    }
}

//============================================================================
//! Renames a process
void CAppMainWindow::onFileRenameProcess()
{
    CProcess* proc = CProcessManager::instance()->currentProcess();
    if (proc != 0) {
        CProcessDescDlg dlg(0);
        dlg.setCaption("Rename process");
        dlg.setProcessName(proc->getName());
        dlg.setProcessDesc(proc->getDesc());

        if (dlg.exec() == QDialog::Accepted) {
            string name = dlg.getProcessName();
	    if (name == "") {
		QMessageBox::information(0, "Whoops!", "No process name specified.\n", "OK");
	    }
	    else if (proc->getName() != name) {
                if (CProcessManager::instance()->isLoaded(name)) {
                    QMessageBox::information(0, "Whoops!",
			"A process with that name is already open.\n"
			"If you wish to rename this process to that name,\n"
			"please close the other process first.", "OK");
                    return;
                }
		QApplication::setOverrideCursor(Qt::waitCursor);
                proc->setName(name);
                proc->setDesc(dlg.getProcessDesc());
                CProcessManager::instance()->activateProcess(proc);
		rebuildProcessMenu();
		updateTitleBar();
		QApplication::restoreOverrideCursor();
            }
	    else
		proc->setDesc(dlg.getProcessDesc());
        }
    }
}

//============================================================================
//! Saves the current process without confirmation.
/*!
    If the project was not yet saved, a dialog box will pop up asking for a
    file name.
 */
void CAppMainWindow::onFileSave()
{
    CProcess* proc = CProcessManager::instance()->currentProcess();
    if (proc != 0) {
        if (proc->getFileName() == "")
            onFileSaveAs();
        else {
	    QApplication::setOverrideCursor(Qt::waitCursor);
            CProcessManager::instance()->saveProcess(proc);
	    QApplication::restoreOverrideCursor();
	}
    }
}

//============================================================================
//! Saves the process under a file name that must be specified.
/*!
    The save as... dialog box is displayed before the file is saved. A filename
    for the process can be given here.
 */
void CAppMainWindow::onFileSaveAs()
{
    CProcess* proc = CProcessManager::instance()->currentProcess();

    if (proc == 0) return;
    string fileName = proc->getFileName();
    if (fileName == "") {
	QDir directory(".");
	fileName = directory.absPath().latin1();
	fileName += "/" + proc->getName() + ".process";
    }
    else {
	string::size_type i, j;
	i = fileName.find("/");
	while ((j = fileName.find("/", i+1)) != string::npos) i = j;
	fileName = fileName.substr(0,i+1) + proc->getName() + ".process";
    }
    QString selected = QFileDialog::getSaveFileName(fileName.c_str(),
	"Processes (*.process*)", this, "", "Save process as");

    if ( !selected.isNull() ) {                // got a file name
	string::size_type i, j, k;
	fileName = selected.latin1();

	i = fileName.find("/");
	while ((j = fileName.find("/", i+1)) != string::npos) i = j;

	j = fileName.find(".", i+1);
	if ((k = fileName.find(".process", i+1)) == string::npos) {
	    fileName += ".process";
	}
	else if (j == k) {
	    j = fileName.find(".", j+1);
	}
	if (j != string::npos || fileName.find(" ", i+1) != string::npos) {
	    QString message = "Save to file '";
	    message += fileName.substr(i+1).c_str();
	    message += "'\nAre you sure?";
	    if (QMessageBox::information(0, "Save process file",
		message, "Yes", "No") == 1) return;
	}
	QApplication::setOverrideCursor(Qt::waitCursor);
        proc->setFileName(fileName);
	updateTitleBar();
        CProcessManager::instance()->saveProcess(proc);
	QApplication::restoreOverrideCursor();
    }
}

//============================================================================
//! Loads a file.
/*!
    The file is added to the current list of open processes.
 */
void CAppMainWindow::onFileLoad()
{
    QString selected = QFileDialog::getOpenFileName(QString::null,
	"Processes (*.process*)", this, "", "Open process");
    if (selected.isNull()) return;
// got a file name
    if (QFile::exists(selected) == false) {
	QMessageBox::information(0, "Whoops!", "Filename does not exists!\n", "OK");
    }
    else {
	QApplication::setOverrideCursor(Qt::waitCursor);
	CProcessManager::instance()->loadProcess(selected.latin1());
	rebuildProcessMenu();
	updateTitleBar();
	QApplication::restoreOverrideCursor();
    }
}

//============================================================================
//! Closes a process.
void CAppMainWindow::onFileClose()
{
    CProcess* current = CProcessManager::instance()->currentProcess();
    if (!current || QMessageBox::information(0, "Close Process",
	"You will lose unsaved process data.\n"
	"Do you really want to close?",
	"Yes", "No") != 0) return;
    QApplication::setOverrideCursor(Qt::waitCursor);
    CProcessManager::instance()->removeProcess(current);
    current = CProcessManager::instance()->currentProcess();
    CProcessManager::instance()->activateProcess(current);
    rebuildProcessMenu();
    updateTitleBar();
    QApplication::restoreOverrideCursor();
}

//============================================================================
//! Quit the application.
void CAppMainWindow::onFileQuit()
{
    CProcess* current = CProcessManager::instance()->currentProcess();
    if (current && QMessageBox::information(0, "Quit SPOCK",
	"You will lose unsaved process data.\n"
	"Do you really want to quit?",
	"Yes", "No") != 0) return;
    qApp -> quit();
}

//============================================================================
//! Starts the technology file generation process.
/*!
    The CGenerateDlg generation dialog is displayed.
 */
void CAppMainWindow::onEditGenerate()
{
    CProcess* current = CProcessManager::instance()->currentProcess();
    if (current != 0) {
        map<string, string> final;
        map<QMultiLineEdit*, string> edits;

        string procName = current->getName();
        string procDesc = current->getDesc();

        CGenerateDlg dlg(0, procName, procDesc);
        dlg.setFileList(current->getGeneratorTitles());

        if (dlg.exec() == QDialog::Accepted) {
            vector<string> selected = dlg.selectedFiles();

    if (selected.size() <= 0) {
	QMessageBox::information(0, "Whoops!", "No files selected!\n", "OK");
	return;
    }

	if (dlg.wantExamineResults()) {

            QTabDialog* genResults = new QTabDialog(0, 0, true);
            genResults->setOkButton("Save all");
            genResults->setCancelButton("Cancel");

	    QApplication::setOverrideCursor(Qt::waitCursor);
            for (unsigned int i=0; i<selected.size(); ++i) {
                string res;
                CProcessManager::instance()->generateFile(selected[i], res);
                string fileName = current->getGeneratorFileName(selected[i]);
                final[fileName] = res;

                QMultiLineEdit* edit = new QMultiLineEdit(genResults);
                edits[edit] = fileName;
                edit->setFont(QFont("Courier", 10));
                genResults->addTab(edit, selected[i].c_str());
                edit->setText(res.c_str());
            }
	    QApplication::restoreOverrideCursor();
            genResults->resize(800, 600);

                if (genResults->exec() != QDialog::Rejected) {
                    map<QMultiLineEdit*, string>::iterator pos;
                    for (pos = edits.begin(); pos != edits.end(); ++pos) {
                        QString tmp = pos->first->text();
                        final[pos->second] = tmp.latin1();
                    }
		    delete genResults;
                } else {
		    delete genResults;
		    if (QMessageBox::information(0, "Generate process files",
			"Save files without edits?", "Yes", "No") == 1) return;
		}
	} else {
	    QApplication::setOverrideCursor(Qt::waitCursor);
            for (unsigned int i=0; i<selected.size(); ++i) {
                string res;
                CProcessManager::instance()->generateFile(selected[i], res);
                string fileName = current->getGeneratorFileName(selected[i]);
                final[fileName] = res;
            }
	    QApplication::restoreOverrideCursor();
	}

            string savePath = dlg.selectedDirectory();
            if (savePath == "") savePath = ".";
            savePath += "/" + current->getName();

    QDir directory(savePath.c_str());
    if (!directory.exists()) {
	QString message = "The directory '";
        message += savePath.c_str();
        message += "' does not exist.\n"
                   "Would you like me to create it?";
        if (QMessageBox::information(0, "Directory does not exist",
	    message, "Yes", "No") == 1) return;
	directory.mkdir(directory.absPath());
    }
    if (!directory.exists()) {
	QString message = "Cannot create directory '";
        message += savePath.c_str();
        message += "'\nGenerate aborted!";
        QMessageBox::critical(0, "Error!", message, "OK");
	return;
    }
	    QApplication::setOverrideCursor(Qt::waitCursor);
            map<string, string>::iterator pos;
            for (pos = final.begin(); pos != final.end(); ++pos)
                saveGeneratedFile(savePath, pos->first, pos->second);

            if (dlg.mustGenerateProcessList())
                dlg.generateProcessList();
	    QApplication::restoreOverrideCursor();
        }
    }
}

//============================================================================
//! Switches the active process.
void CAppMainWindow::onClickedProcessMenu(int menuID)
{
    CProcessManager* processMan = CProcessManager::instance();
    QString processName = m_processMenu->text(menuID);

    CProcess* process = processMan->getProcess(processName.latin1());

    if (process != processMan->currentProcess())
        processMan->activateProcess(process);

    updateTitleBar();
}

//============================================================================
//! Updates title bar and status indicator the display the current active process.
void CAppMainWindow::updateTitleBar()
{
    CProcess* process = CProcessManager::instance()->currentProcess();

    if (process != 0) {
        QString titleBarTxt;
        titleBarTxt += "spock - ";
        titleBarTxt += process->getName().c_str();
     // titleBarTxt += " - ";
     // titleBarTxt += process->getFileName().c_str();
        setCaption(titleBarTxt);
        m_processIndicator->onChangeText(process->getName().c_str());
    } else {
     // setCaption("No current process");
        setCaption("spock");
        m_processIndicator->onChangeText("No process");
    }
}

//============================================================================
//! If the current focus widget is a spreadsheet widget, it adds a new row.
void CAppMainWindow::onEditBarAddRow()
{
    QWidget* focus = focusWidget();

    CSpreadSheetView* sheet = dynamic_cast<CSpreadSheetView*>(focus);
    if (sheet != 0)
        sheet->addNewRow();
}

//============================================================================
//! If the current focus widget is a spreadsheet widget, it deletes the current row.
void CAppMainWindow::onEditBarDelRow()
{
    QWidget* focus = focusWidget();

    CSpreadSheetView* sheet = dynamic_cast<CSpreadSheetView*>(focus);
    if (sheet != 0)
        sheet->deleteRow(sheet->currentItem());
}

//============================================================================
//! If the current focus widget is a spreadsheet widget, inserts a row above.
void CAppMainWindow::onEditBarInsAboveRow()
{
    QWidget* focus = focusWidget();

    CSpreadSheetView* sheet = dynamic_cast<CSpreadSheetView*>(focus);
    if (sheet != 0)
        sheet->insertItemAbove(sheet->currentItem());
}

//============================================================================
//! If the current focus widget is a spreadsheet widget, inserts a row below.
void CAppMainWindow::onEditBarInsBelowRow()
{
    QWidget* focus = focusWidget();

    CSpreadSheetView* sheet = dynamic_cast<CSpreadSheetView*>(focus);
    if (sheet != 0)
        sheet->insertItemBelow(sheet->currentItem());
}

//============================================================================
//! If the current focus widget is a spreadsheet widget, moves this row up.
void CAppMainWindow::onEditBarMoveRowUp()
{
    QWidget* focus = focusWidget();

    CSpreadSheetView* sheet = dynamic_cast<CSpreadSheetView*>(focus);
    if (sheet != 0)
        sheet->moveRowUp(sheet->currentItem());
}

//============================================================================
//! If the current focus widget is a spreadsheet widget, moves this row down.
void CAppMainWindow::onEditBarMoveRowDown()
{
    QWidget* focus = focusWidget();

    CSpreadSheetView* sheet = dynamic_cast<CSpreadSheetView*>(focus);
    if (sheet != 0)
        sheet->moveRowDown(sheet->currentItem());
}

//============================================================================
//! Displays the about box.
void CAppMainWindow::onHelpAbout()
{
//  CAboutDlg dlg(0);
//  dlg.exec();
    QMessageBox::about(0, "About SPOCK",
	"<qt><center><h3>SPOCK</h3>Version 1.3</center><br>"
	"Technology file generator for SPACE<br>"
	"See <tt>http://www.space.tudelft.nl</font></tt><br>"
	"Programming and design by Xander Burgerhout<br>"
	// "Patient coaching and guiding by N.P. van der Meijs<br>"
	"This program uses the Qt toolkit</qt>");
}

//============================================================================
//! Displays some information about Qt.
void CAppMainWindow::onHelpAboutQt()
{
    QMessageBox::aboutQt(0);
}

//============================================================================
//! Saves a generated technology file
void CAppMainWindow::saveGeneratedFile(const string& dir, const string& name, const string& contents)
{
    QDir directory(dir.c_str());
    string fileName = directory.absPath().latin1();
    fileName += "/" + name;
    fprintf(stderr, "Writing %s\n", fileName.c_str());

    QFile file(fileName.c_str());

    if (file.open(IO_WriteOnly)) {
        QTextStream ts(&file);
        ts << contents.c_str();
        file.close();
    } else {
        QString message;
        message.sprintf("Could not open the file %s for writing.\nMake sure you have write access to\n%s",
                        fileName.c_str(), directory.absPath().latin1());
        QMessageBox::critical(0, "Error!", message, "OK");
    }
}
