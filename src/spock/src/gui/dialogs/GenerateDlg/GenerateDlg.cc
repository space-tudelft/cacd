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
#include <qframe.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

#include <qfiledialog.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qmessagebox.h>
#include <qcombobox.h>
#include <qcheckbox.h>

#include <stdlib.h>

// Project includes
#include "src/spock/src/gui/dialogs/GenerateDlg/GenerateDlg.h"
#include "src/spock/src/helper/log/Helper.h"
#include "src/spock/src/gui/dialogs/GenerateDlg/NewProcListEntryDlg.h"

// STL includes
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

//============================================================================
//! Constructor
/*!
    Does some dialog box initialization.
 */
CGenerateDlg::CGenerateDlg(QWidget* parent, const string& procName, const string& procDesc)
    : GenerateDlgInternal( parent, 0, true )
{
    m_newProcName = procName;
    m_newProcDesc = procDesc;

    QString capt = "Generate files for the ";
    capt += procName.c_str();
    capt += " process...";
    setCaption(capt);

    m_layoutFileFrame = new QVBoxLayout(m_frameFiles);
    m_layoutFileFrame->setAutoAdd(true);
}

//============================================================================
//! Does some initialization that should not be done in the constructor
/*!
    Fills the process number combobox and sets the destination directory.
 */
void CGenerateDlg::polish()
{
    fillProcessNrCombo();
    setGenToDir();
}

//============================================================================
//! Destructor
/*!
    Destroys the object and frees any allocated resources
 */
CGenerateDlg::~CGenerateDlg()
{
    // no need to delete child widgets, Qt does it all for us
}

//============================================================================
//! Returns true if the generation results should be inspected.
/*!
    \return True if the generation results should be inspected.
            False if the generation results should be saved immediately.
 */
bool CGenerateDlg::wantExamineResults()
{
    if (m_checkExamine->isChecked())
        return true;
    return false;
}

//============================================================================
//! Returns true if the processlist file needs to be regenerated.
/*!
    \return True if the processlist file needs to be regenerated. False if not.
 */
bool CGenerateDlg::mustGenerateProcessList()
{
    if (m_radioGenerateSpace->isChecked())
        return true;
    return false;
}

//============================================================================
//! Returns the selected destination directory
/*!
    \return The selected destination directory.
 */
string CGenerateDlg::selectedDirectory()
{
    if (mustGenerateProcessList()) {
        char* icdpath = getenv("ICDPATH");
        if (icdpath == 0)
            return "";
        string tmp = icdpath;
        tmp += "/share/lib/process";
        return tmp;
    }

    if (m_editDirectory->text().isNull())
        return "";
    else
        return m_editDirectory->text().latin1();
}

//============================================================================
//! Switches between the ``generate to process tree'' and ``generate to directory''.
void CGenerateDlg::setGenToDir(bool val)
{
    m_genToDir = val;

    m_radioGenerateSpace->setChecked(!val);
    m_comboProcessNr->setEnabled(!val);
    m_labelProcessNr->setEnabled(!val);
    m_buttonNewProcessNr->setEnabled(!val);

    m_radioGenerateDir->setChecked(val);
    m_editDirectory->setEnabled(val);
    m_buttonDirBrowse->setEnabled(val);
    m_labelDir->setEnabled(val);
}

//============================================================================
//! Sets the files that can be generated.
/*!
    Each file will have a checkbox, allowing the user to select the files
    that must be generated.
 */
void CGenerateDlg::setFileList(const vector<string>& fileList)
{
    m_fileList = fileList;
    rebuildFileFrame();
}

//============================================================================
//! Returns the list of files the user wants to generate.
/*!
    \return The list of files the user wants to generate.
 */
vector<string> CGenerateDlg::selectedFiles()
{
    vector<string> ret;
    for (unsigned int i=0; i<m_fileChecks.size(); ++i) {
        if (m_fileChecks[i]->isChecked())
            ret.push_back(m_fileChecks[i]->text().latin1());
    }
    return ret;
}

//============================================================================
//! Generates the processlist file in the process tree.
void CGenerateDlg::generateProcessList()
{
    string fileName = selectedDirectory() + "/processlist";
    QFile file(fileName.c_str());

    if (!file.open(IO_WriteOnly)) {
        QString msg;
        msg.sprintf("Could not open '%s' for writing.\n"
                    "Make sure your ICDPATH is set and that you\n"
                    "have write access to this file.", fileName.c_str());
        QMessageBox::critical(0, "Error writing process list!", msg, "OK");
        return;
    }

    QTextStream ts(&file);

    ts << "# Generated by SPOCK on ";
    ts << QTime::currentTime().toString();
    ts << " " << QDate::currentDate().toString();
    ts << "\n# process list\n# id\tname\t# desc\n";

    for (unsigned int i=0; i<m_processDesc.size(); ++i) {
        ts << m_processDesc[i].nr() << "\t" << m_processDesc[i].name().c_str();
        ts << "\t# " << m_processDesc[i].desc().c_str() << "\n";
    }
    file.close();
}

//============================================================================
//! Lets the user select a directory.
void CGenerateDlg::onBrowseDirectory()
{
    QString dir = QFileDialog::getExistingDirectory();
    m_editDirectory->setText(dir);
}

//============================================================================
//! Lets the user enter a new process number.
/*!
    The new process number is added to the combobox containing the processes.
 */
void CGenerateDlg::onNewProcessNr()
{
    const char *dn = "New...";
    CNewProcListEntryDlg dlg(0);

    if (dlg.exec() == QDialog::Accepted) {
	int f = 0;
        int nr = dlg.getNr();
	if (nr <= 0) return;
    for (unsigned int i=0; i<m_processDesc.size(); ++i) {
	if (m_processDesc[i].nr() == nr) f += 1;
	if (m_processDesc[i].name() == m_newProcName) f += 2;
	if (f == 1) {
	    QMessageBox::information(0, dn, "New process number already found!", "OK");
	    return;
	}
	if (f == 2) {
	    QMessageBox::information(0, dn, "Process name already found!", "OK");
	    return;
	}
	if (f == 3) {
	    if (m_processDesc[i].desc() == m_newProcDesc) {
		QMessageBox::information(0, dn, "Process entry already found!", "OK");
		return;
	    }
	    if (QMessageBox::information(0, dn,
		"Process entry found!\nDo you want to replace the description?",
		"Yes", "No") == 1) return;
	    break;
	}
    }
	int    proc_id   = nr;
	string proc_name = m_newProcName;
	string proc_desc = m_newProcDesc;
	string tmp;
        QString item;
    m_comboProcessNr->clear();
    for (unsigned int i=0; i<m_processDesc.size(); ++i) {
	if (!f && proc_id <  m_processDesc[i].nr()) {
	    nr  = m_processDesc[i].nr();
	    m_processDesc[i].setNr(proc_id); proc_id = nr;
	    tmp = m_processDesc[i].name();
	    m_processDesc[i].setName(proc_name); proc_name = tmp;
	    tmp = m_processDesc[i].desc();
	    m_processDesc[i].setDesc(proc_desc); proc_desc = tmp;
	}
	else if (f == 3 && proc_id == m_processDesc[i].nr()) {
	    m_processDesc[i].setDesc(proc_desc); f = 1;
	}
	item.sprintf("%d %s  - %s", m_processDesc[i].nr(),
                                    m_processDesc[i].name().c_str(),
                                    m_processDesc[i].desc().c_str());
        m_comboProcessNr->insertItem(item);
    }
	if (f) return;
        item.sprintf("%d %s  - %s", proc_id, proc_name.c_str(), proc_desc.c_str());
        m_comboProcessNr->insertItem(item);
        CProcessListEntry newEntry;
        newEntry.setNr(proc_id);
        newEntry.setName(proc_name);
        newEntry.setDesc(proc_desc);
        m_processDesc.push_back(newEntry);
    }
}

//============================================================================
//! Sets generation to a user specified directory.
void CGenerateDlg::onClickedRadioDir()
{
    setGenToDir(true);
}

void CGenerateDlg::onClickedRadioProcessNr()
//============================================================================
//! Sets generation to the SPACE process tree.
{
    setGenToDir(false);
}

//============================================================================
//! Rebuilds the frame containing the files that can be generated.
void CGenerateDlg::rebuildFileFrame()
{
    for (unsigned int i=0; i<m_fileChecks.size(); ++i) {
        if (m_fileChecks[i] != 0)
            delete m_fileChecks[i];
    }

    m_fileChecks.clear();

    for (unsigned int i=0; i<m_fileList.size(); ++i) {
        QCheckBox* check = new QCheckBox(m_fileList[i].c_str(), m_frameFiles);
        check->setChecked(true);
        m_fileChecks.push_back(check);
    }
}

//============================================================================
//! Fills the combobox containing the processes in the SPACE proces tree.
void CGenerateDlg::fillProcessNrCombo()
{
    vector<string> procs = readProcesses();

    m_processDesc.clear();
    for (unsigned int i=0; i<procs.size(); ++i) {
        CProcessListEntry entry;
        entry.buildFromString(procs[i]);
        if (entry.isValid())
            m_processDesc.push_back(entry);
    }

    m_comboProcessNr->clear();
    for (unsigned int i=0; i<m_processDesc.size(); ++i) {
        QString item;
        item.sprintf("%d %s  - %s", m_processDesc[i].nr(),
                                    m_processDesc[i].name().c_str(),
                                    m_processDesc[i].desc().c_str());
        m_comboProcessNr->insertItem(item);
    }
}

//============================================================================
//! Parses the processlist file in the SPACE process tree.
/*!
    \return A vector containing the relevant lines in the processlist file.
 */
vector<string> CGenerateDlg::readProcesses()
{
    vector<string> ret;

    string env = "ICDPATH";
    char* tmp = 0;
    tmp = getenv(env.c_str());
    string fileName = "";
    if (tmp == 0)
        fileName = "/ICDPATH_not_set";
    else
        fileName = tmp;
    fileName += "/share/lib/process/processlist";

    QFile file(fileName.c_str());

    QString message = "The file ";
    message += fileName.c_str();
    message += " does not exist.\n"
               "Make sure ICDPATH is set to the correct\n"
               "location and that SPACE has been installed correctly.\n"
               "Ask the system administrator for more information.";

    if (!file.exists()) {
        QMessageBox::critical(0, "File does not exist!", message, "OK");
        setGenToDir();
        m_radioGenerateSpace->setEnabled(false);
        return ret;
    }

    file.open(IO_ReadOnly);

    if (file.status() != IO_Ok) {
        message = "An unidentified error occured trying to open this file:\n";
        message += fileName.c_str();
        message += "\nSorry, cannot help you with this.";
        switch (file.status()) {
            case IO_ReadError:
            case IO_OpenError:
                message = "Could not read or open the file ";
                message += fileName.c_str();
                message += "\nMake sure you have read access to this file\n"
                           "and that ICDPATH is set correctly.";
                break;
            case IO_FatalError:
            case IO_AbortError:
            case IO_TimeOutError:
                message = "The operation was aborted, timed out or an \n"
                          "unrecoverable error occured trying to read or open:\n";
                message += fileName.c_str();
                message += "\nMake sure you have read access to this file\n"
                           "and that ICDPATH is set correctly.";
                break;
        }
        QMessageBox::critical(0, "Error reading file!", message, "OK");
        setGenToDir();
        m_radioGenerateSpace->setEnabled(false);
        return ret;
    }

    int result = -1;
    string procLine = "";
    do {
        QString tmp;
        result = file.readLine(tmp, 100);
        if (!tmp.isNull()) {
            procLine = tmp.latin1();
            procLine.resize(procLine.size()-1); // strip newline
            ret.push_back(procLine);
        }
    } while (result != -1);

    return ret;
}

//============================================================================
//! Constructor
CProcessListEntry::CProcessListEntry()
{
    m_nr = -1;
}

//============================================================================
//! Copy constructor
CProcessListEntry::CProcessListEntry(const CProcessListEntry& src)
{
    *this = src;
}

//============================================================================
//! Assignment operator
/*!
    This object will be made equal to \a src.
 */
CProcessListEntry& CProcessListEntry::operator=(const CProcessListEntry& src)
{
    m_nr = src.m_nr;
    m_name = src.m_name;
    m_desc = src.m_desc;
    return *this;
}

//============================================================================
//! Returns the process number
/*!
    \return The process number
 */
int CProcessListEntry::nr()
{
    return m_nr;
}

//============================================================================
//! Sets the process number
void CProcessListEntry::setNr(int nr)
{
    m_nr = nr;
}

//============================================================================
//! Returns the process name
/*!
    \return The process name
 */
string CProcessListEntry::name()
{
    return m_name;
}

//============================================================================
//! Sets the process name
void CProcessListEntry::setName(const string& name)
{
    m_name = name;
}

//============================================================================
//! Returns the process description
/*!
    \return The process description
 */
string CProcessListEntry::desc()
{
    return m_desc;
}

//============================================================================
//! Sets the process description
void CProcessListEntry::setDesc(const string& desc)
{
    m_desc = desc;
}

//============================================================================
//! Builds a CProcessListEntry object from a string.
/*!
    The string must be in the format used in the processlist file found in the
    SPACE process tree ( a number, whitespace, process name whitespace, process description).
 */
void CProcessListEntry::buildFromString(const string& src)
{
    string::size_type i = 0;
    m_nr = -1;

    string work = src;
    vector<string> split = splitString(work, "#");

    if (split.size() > 1) {
	work = split.back();
	while (work.find(" ",i) == i) ++i;
	m_desc = work.substr(i);
        work = split[0];
    }

    i = 0;
    while ((i = work.find("\t",i)) != string::npos) work.replace(i,1," ");
    split = splitString(work, " ");
    vector<string>::iterator pos;
    pos = remove(split.begin(), split.end(), "");
    split.erase(pos, split.end());

    if (split.size() != 2) {
        m_nr = -1;
        return;
    }
    m_nr = atoi(split[0].c_str());
    m_name = split[1];
}

//============================================================================
//! Returns true if this is a valid processlist entry object.
bool CProcessListEntry::isValid()
{
    if (m_nr == -1)
        return false;
    return true;
}
