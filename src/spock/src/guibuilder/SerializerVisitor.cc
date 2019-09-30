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
#include "src/spock/src/guibuilder/SerializerVisitor.h"
#include "src/spock/src/guibuilder/GUITree.h"
#include "src/spock/src/datastruct/Component.h"
#include "src/spock/src/datastruct/ComponentTree.h"
#include "src/spock/src/parser/Parser.h"
#include "src/spock/src/guibuilder/Process.h"
#include "src/spock/src/widgets/SpreadSheet/SpreadSheet.h"
#include "src/spock/src/helper/log/Helper.h"

#include <stdlib.h>

// Qt includes
#include <qapplication.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qmessagebox.h>
#include <qprogressdialog.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qlistview.h>
#include <qregexp.h>

using namespace std;

//============================================================================
//! Default constructor
/*!
    \param tree         The gui tree containing the current entered values.
    \param fileName     The name of the file we are serializing.
    \param processName  The name of the process.
    \param processDesc  The description of the process.
    \param isSaving     Specifies if we are storing (saving) or retrieving (loading)

    The constructor opens the file and calls doFileInit(), which will prepare the
    file for saving or loading.
*/
CSerializerVisitor::CSerializerVisitor(CGUITree* tree, const string& fileName,
        const string& processName, const string& processDesc, bool isSaving)
{
    ASSERT(tree != 0);
    m_fileVerMajor = CParser::instance()->getMajorVersion();
    m_fileVerMinor = CParser::instance()->getMinorVersion();
    m_tree = tree;
    m_processName = processName;
    m_processDesc = processDesc;
    m_fileName = fileName;
    m_isSaving = isSaving;

    m_file = new QFile(fileName.c_str());

    m_isValid = false;
    if (isSaving)
        m_isValid = m_file->open(IO_WriteOnly);
    else
        m_isValid = m_file->open(IO_ReadOnly);

    if (!m_isValid)
        QMessageBox::critical(0, "Error opening file!", "Could not open file.");
    else
        m_str = new QTextStream(m_file);

    doFileInit();
}

//============================================================================
//! Returns true of the file was correctly initialized.
/*!
    \return True of the file was correctly initialized, false otherwise.
 */
bool CSerializerVisitor::isValid()
{
    return m_isValid;
}

//============================================================================
//! Returns true if we are saving (storing) the file.
/*!
    \return True if we are saving (storing) the file. False otherwise.
*/
bool CSerializerVisitor::isSaving()
{
    return m_isSaving;
}

//============================================================================
//! Initializes the file.
/*!
    This means the file header is read or written, depending of the serialization
    direction.

    \sa isSaving(), isValid()
 */
void CSerializerVisitor::doFileInit()
{
    if (!isValid()) return;
    if (isSaving()) {
	*m_str << m_fileVerMajor << " " << m_fileVerMinor << endl;
	*m_str << m_processName.c_str() << endl;
	*m_str << m_processDesc.c_str() << endl;
	return;
    } else {
	char buf[8];
	m_str->readRawBytes(buf, 4);
	if (buf[3] == '\r') m_str->readRawBytes(buf+3, 1);
	if (buf[0] < '1' || buf[0] > '1' || buf[1] != ' ' ||
	    buf[2] < '0' || buf[2] > '1' || buf[3] != '\n') {
	    QMessageBox::critical(0, "Error opening file!",
		"Incorrect input file format.\nMaybe an old type input file!");
	    goto err;
	}
	m_fileVerMajor = (buf[0] - '0');
	m_fileVerMinor = (buf[2] - '0');
	QString s = m_str->readLine();
	QRegExp expr("^[a-zA-Z_][a-zA-Z_0-9]*$");
	if (expr.search(s, 0) != 0) {
	    QMessageBox::critical(0, "Error opening file!",
		"Line 2: Incorrect process name.\nPlease correct this line first!");
	    goto err;
	}
	m_processName = s.latin1();
	s = m_str->readLine().simplifyWhiteSpace();
	m_processDesc = s.latin1();
	return;
    }
err:
    m_isValid = false;
    delete m_str;
    m_file->close();
}

//============================================================================
//! Converts from an older version
/*!
    In future versions, compatibility with older files might be broken. In this
    case this routine can be altered to restore the compatibility.

    The first paramter specifies the identifier that needs to be converted.
    This could mean renaming the identifier, if required.

    The second parameter contains the values. They too can be converted if
    necessary.

    \sa writeConversion()
 */
void CSerializerVisitor::readConversion(string& /*identifier*/, vector<string>& /*values*/)
{
    // Conversions necessary to read older files can be put here.

    /* An example:

       We are reading an old file. The old file contains:
         identifier = maskdata_tab.maskdata_sheet.ID
         values = "0", "1", "2", "3"

      In the current version, ID has been renamed to layerID.
      Therefore, we must change 'identifier':

        if (identifier == "maskdata_tab.maskdata_sheet.ID")
            identifier = "maskdata_tab.maskdata_sheet.layerID";

      Also, assume the values of these ID's must be at least 100.
      One solution could be to add 100 to each of the values.
      */
}

//============================================================================
//! Converts to an older version
/*!
    To be able to write older version files, this method can be changed.
    As in the readConversion() method, the first parameter is the identifier and
    the second the values.

    \sa readConversion()
 */
void CSerializerVisitor::writeConversion(string& /*identifier*/, vector<string>& /*values*/)
{
    // Conversions necessary to write older files can be put here.
    // See the readConversion() function for an example.
    // Of course, write conversion should be the inverse of the read conversion!
}

//============================================================================
//! Calls CGUITree::getValues()
/*!
    \returns The values returned by CGUITree::getValues()

    \sa CGUITree::getValues(), setValues(), CGUITree::setValues()
 */
vector<string> CSerializerVisitor::getValues(CComponent* comp)
{
    return m_tree->getValues(comp);
}

//============================================================================
//! Calls CGUITree::setValues()
/*!
    \sa CGUITree::setValues(), getValues(), CGUITree::getValues()
 */
void CSerializerVisitor::setValues(CComponent* comp, const vector<string>& values)
{
    m_tree->setValues(comp, values);
}

//============================================================================
//! Calls CGUITree::getSpreadSheetValues()
vector<string> CSerializerVisitor::getSpreadSheetValues(CComponent* sheet, CComponent* comp)
{
    return m_tree->getSpreadSheetValues(sheet, comp);
}

//============================================================================
//! Calls CGUITree::setSpreadSheetValues()
void CSerializerVisitor::setSpreadSheetValues(CComponent* sheet, CComponent* comp, const vector<string>& values)
{
    m_tree->setSpreadSheetValues(sheet, comp, values);
}

//============================================================================
//! Tries to make an educated guess as to where \a identifier refers to.
/*!
    If the load() method cannot immediatly determine the component belongng to
    \a identifier it is possible the \a identifier has been moved in the
    configuration file. It then tries to find the best match wich is probably
    the new location.

    The guess is made by removing all scope from the identifier, leaving only
    the simple component name. Then, scope is gradually added until there are
    no more hist by the search operation. The best guess so far is then
    returned.

    \return The best guess or 0 if there is not even one good candidate.
 */
CComponent* CSerializerVisitor::guessCorrectComponent(const string& identifier)
{
    debug("Making educated guess for %s.", identifier.c_str());
    debug("-----------------------------------------------");

    vector<string> parts = splitString(identifier, ".");
    if (parts.size() == 0)
        parts.push_back("##notfound##");

    string guess = parts.back();
    CComponent* bestguess = 0;

    while (parts.size() != 0) {
        debug("Considering [%s]", guess.c_str());

        CComponentTree* compTree = m_tree->getComponentTree();
        CFindComponentVisitor query = compTree->search(guess);

        debug("Found %d candidates", query.hitCount());
        if (query.hitCount()>0) {
            bestguess = query.getHit();
            debug("Best guess is now [%s]", bestguess->getFullContextName().c_str());
        }

        parts.pop_back();
        if (parts.size() != 0)
            guess = parts.back() + "." + guess;
    }
    debug("Done guessing.\n");
    return bestguess;
}

//============================================================================
//! Serializes each node in the component tree.
/*!
    This method can of course only save an existing tree.

    \param comp The component to serialize.

    \sa load()
 */
void CSerializerVisitor::visitComponent(CComponent* comp)
{
    if (isValid() && isSaving()) {
	vector<string> values = getValues(comp);
	int nr = values.size();
	while (nr > 0 && values[nr-1] == "") --nr;
	if (nr > 0) {
            string fullName = comp->getFullContextName();

	    vector<string> parts = splitString(fullName, ".");
	    int i = parts.size();
	    ASSERT(i >= 2);
	    fullName = parts[i-2] + "." + parts[i-1];

///////     writeConversion(fullName, values);

	    *m_str << fullName.c_str() << " " << nr << endl;
	    for (i=0; i<nr; ++i)
		*m_str << values[i].c_str() << endl;
	}
    }
}

//============================================================================
//! Returns the process name
/*!
    \return The process name
 */
string CSerializerVisitor::getProcessName()
{
    return m_processName;
}

//============================================================================
//! Returns the process description
/*!
    \return The process description
 */
string CSerializerVisitor::getProcessDesc()
{
    return m_processDesc;
}

//============================================================================
//! Starts the serilization process.
void CSerializerVisitor::serialize()
{
    debug("Serializing...");
    if (isValid()) {
        if (isSaving()) {
            m_tree->getComponentTree()->accept(*this);
        } else {
            m_calledFromSerialize = true;
            load();
            m_calledFromSerialize = false;
        }
	delete m_str;
	m_file->close();
	m_isValid = false;
    }
    else
        debug("ERROR: CSerializerVisitor::serialize() not a valid serializer!");
}

//============================================================================
//! Does a load.
/*!
    The visitComponent() method cannot perform a load since the component tree
    in the file can differ from the one present.

    The load process reads each component full-context name and values stored
    in the file and then tries to find this full-context identifier name in
    the current tree. If found, the values are set to the ones loaded.
 */
void CSerializerVisitor::load()
{
    ASSERT(m_calledFromSerialize == true);

    vector<string> values;
    string identifier;
    bool giveWarning = false;

    debug("Starting the load process...");
    while (!m_str->atEnd()) {
	QString s = m_str->readLine();
	int size = s.find(' ');
	if (size > 0) {
	    identifier = s.left(size).latin1();
	    size = atoi(s.right(s.length() - size).latin1());
	}
	if (size <= 0) {
	    QMessageBox::critical(0, "Error reading file!",
		"Incorrect input file.\nNumber of values <= 0!");
	    break;
	}
    //  string debug_string = "";
        for (int i=0; i<size; ++i) {
	    s = m_str->readLine().simplifyWhiteSpace();
            values.push_back(s.latin1());
    //      debug_string +=  s.latin1();
    //      debug_string += " ";
        }
    //  debug("%s\t= %s", identifier.c_str(), debug_string.c_str());
/////// readConversion(identifier, values);
	CComponent* comp = m_tree->findComponent(identifier);
	if (comp != 0)
	    setValues(comp, values);
	else {
	//  comp = guessCorrectComponent(identifier);
	//  if (comp != 0)
	//      setValues(comp, values);
	//  else
		fprintf(stderr, "name '%s' not found!\n", identifier.c_str());
		giveWarning = true;
	}
	values.clear();
    }

    if (giveWarning)
        QMessageBox::warning( 0, "Warning",
                "Some values saved in this file could not be placed\n"
                "at their correct location in the user interface.\n"
                "This means that the user interface specification is\n"
                "not completely compatible with this saved file.\n"
                "Some values could be lost or changed, please check\n"
                "the data in this file and save it to update the format.",
                "OK");
}
