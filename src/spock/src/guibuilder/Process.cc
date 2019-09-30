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
#include "src/spock/src/guibuilder/Process.h"
#include "src/spock/src/guibuilder/GUITree.h"
#include "src/spock/src/guibuilder/SerializerVisitor.h"
#include "src/spock/src/parser/Parser.h"
#include "src/spock/src/datastruct/Generator.h"
#include "src/spock/src/widgets/SpreadSheet/SpreadSheet.h"
#include "src/spock/src/datastruct/Keywords.h"
#include "src/spock/src/helper/log/Helper.h"

// STL includes
#include <algorithm>
#include <sstream>
#include <iostream>

// Qt includes
#include <qdatetime.h>
#include <qprogressdialog.h>
#include <qapplication.h>

using namespace std;

//============================================================================
//! Default constructor
/*!
    Sets some defaults and retrieves the generators from the parser.
    \sa CParser.
 */
CProcess::CProcess()
{
    m_tree = 0;
    m_hasChanged = 0;

    m_generators = CParser::instance()->getGenerators();
}

//============================================================================
//! Returns the associated CGUITree object containing the user interface.
/*!
    \returns The CGUITree object describing the user interface belonging to
    this process. The widgets in the tree contain the values currently entered.

    \sa CGUITree, setGUITree()
 */
CGUITree* CProcess::guiTree()
{
    return m_tree;
}

//============================================================================
//! Associates a CGUITree with this process.
/*!
    \param tree The CGUITree object.
 */
void CProcess::setGUITree(CGUITree* tree)
{
    m_tree = tree;
}

//============================================================================
//! Returns the name of this process
/*!
    \return The name of this process
    \sa setName(), setDesc(), getDesc()
 */
string CProcess::getName()
{
    return m_processName;
}

//============================================================================
//! Sets the name of this process
/*!
    \param name The new name of this process
    \sa getName(), setDesc(), getDesc()
 */
void CProcess::setName(const string& name)
{
    m_processName = name;
}

//============================================================================
//! Returns the description of this process
/*!
    \return The description of this process
    \sa setDesc(), getName(), setName()
 */
string CProcess::getDesc()
{
    return m_processDesc;
}

//============================================================================
//! Sets the description of this process
/*!
    \param descr The description of this process
    \sa getDesc(), setName(), getName()
 */
void CProcess::setDesc(const string& descr)
{
    vector<string> split = splitString(descr, " ");
    string work = "";
    for (unsigned int i = 0; i < split.size(); ++i) {
	if (split[i] != "") {
	    if (work != "") work += " ";
	    work += split[i];
	}
    }
    m_processDesc = work;
}

//============================================================================
//! Sets the filename the process is stored in.
/*!
    \param name The name of the file.
    \sa getFileName()
 */
void CProcess::setFileName(const string& name)
{
    m_fileName = name;
}

//============================================================================
//! Get the filename the process is stored in.
/*!
    \return The name of the file.
    \sa setFileName()
*/
string CProcess::getFileName()
{
    return m_fileName;
}

//============================================================================
//! Flag the generateFile process.
void CProcess::toggleFileGen()
{
    if (m_tree)
	m_tree->toggleFileGen();
}

//============================================================================
//! Saves this process
/*!
    Constructs a CSerializerVisitor object and calls the CSerializerVisitor::serialize()
    method.

    \sa CSerializerVisitor, CSerializerVisitor::serialize()
 */
void CProcess::save()
{
    ASSERT(m_tree != 0);
    ASSERT(m_fileName != "");

    CSerializerVisitor* serializer = new CSerializerVisitor(m_tree, m_fileName, m_processName, m_processDesc, true);
    serializer->serialize();
    delete serializer;
}

//============================================================================
//! Loads into this process
/*!
    Constructs a CSerializerVisitor object and calls the CSerializerVisitor::serialize()
    method.

    The dataconnections are disabled to prevent the continous updating of
    values in the user interface during loading. Once the file is loaded
    the connections are enabled again. Enabling them results in a massive
    update of the user interface.

    \sa CSerializerVisitor, CSerializerVisitor::serialize()
 */
bool CProcess::load()
{
    ASSERT(m_tree != 0);
    ASSERT(m_fileName != "");

    CSerializerVisitor* serializer = new CSerializerVisitor(m_tree, m_fileName);
    if (serializer->isValid()) {
	m_tree->disableDataConnectionUpdates();
	serializer->serialize();
	m_processName = serializer->getProcessName();
	m_processDesc = serializer->getProcessDesc();
	delete serializer;
	m_tree->enableDataConnectionUpdates();
    }
    else {
	delete serializer;
	return false;
    }
    return true;
}

//============================================================================
//! Retrieves the descriptions of the generators
/*!
    The descriptions of the gnerators are usuallu a short description of the
    technology file names, for example ``Element definitions'' for the
    space.def.s file.

    \sa getGeneratorFileName()
 */
vector<string> CProcess::getGeneratorTitles()
{
    vector<string> names;

    for (int i=0; i<m_generators->numGenerators(); ++i)
        names.push_back(m_generators->getGenerator(i)->getProperty(PROP_TITLE));
    return names;
}

//============================================================================
//! Returns the filename belonging to a generator description
/*!
    \return The filename belonging to a generator description.
 */
string CProcess::getGeneratorFileName(const string& generatorTitle)
{
    for (int i=0; i<m_generators->numGenerators(); ++i)  {
        if (m_generators->getGenerator(i)->getProperty(PROP_TITLE) == generatorTitle)
            return m_generators->getGenerator(i)->getProperty(PROP_FILENAME);
    }
    return "";
}

//============================================================================
//! Obsolete.
map<string, string> CProcess::generateAllFiles()
{
    map<string, string> retval;

    QProgressDialog progress( "Generating technology files...", "Stop", m_generators->numGenerators(),
                              0, "progress", true );
    progress.setMinimumDuration(0);

    for (int i=0; i<m_generators->numGenerators(); ++i) {
        progress.setProgress(i);
        qApp->processEvents();

        string title = m_generators->getGenerator(i)->getProperty(PROP_TITLE);
        string result;
        generateFile(title, result);
        retval[title] = result;

        if (progress.wasCancelled())
            i = m_generators->numGenerators();
     }
     progress.setProgress(m_generators->numGenerators());

    return retval;
}

//============================================================================
//! Generates a single technology file.
/*!
    \param title    The description of the technology file
    \param result   The contents of the generated file are put into this parameter.

    This method starts the recursive generate().

    \sa generate()
 */
void CProcess::generateFile(const string& title, string& result)
{
    CGeneratorComp* generator = 0;
    for (int i=0; i<m_generators->numGenerators(); ++i) {
        if (m_generators->getGenerator(i)->getProperty(PROP_TITLE) == title)
            generator = m_generators->getGenerator(i);
    }

    toggleFileGen();
    result = "";
    if (generator != 0) {
        for (int i=0; i<generator->numChildren(); ++i)
            generate(generator, generator->getChild(i), result);
    }
    toggleFileGen();
}

//============================================================================
//! Generates a node of the generator expression tree.
/*!
    The generation process works by the evaluation of the process tree.

    This method evaluates a node of this expression tree. It calls the correct
    evaluation methods for each type of node.

    \param generator    The generator we are evaluating.
    \param node         The current node in the expression tree
    \param result       The result so far.
 */
void CProcess::generate(CGeneratorComp* generator, CComponent* node, string& result)
{
    ASSERT(generator != 0);
    ASSERT(node != 0);

    string type = node->getType();

    if (type == GEN_APPLICATION)
        result += getApplicationValue(node->getProperty(GEN_FIELD));

    if (type == GEN_LITERAL || type == GEN_REAL || type == GEN_INTEGER)
        result += node->getProperty(GEN_VALUE);

    if (type == "+" || type == "-")
        generateBinaryNode(node, result);

    if (type == GEN_IDENTIFIER)
        result += generateIdentifier(generator, node);

    if (type == GEN_FOREACH)
        generateForEach(generator, node, result);

    if (type == GEN_IF)
        generateIf(generator, node, result);
}

//============================================================================
//! Generates an identifer type node.
/*!
    The identifier is replaced with its current value. Checks are made to see
    if it is an iterated value and if so the iterated value is used.
 */
// node is the current generator component, describing what to generate
string CProcess::generateIdentifier(CGeneratorComp* generator, CComponent* node)
{
    ASSERT(generator != 0);
    ASSERT(node != 0);

    string result = "";

    // Retrieve the component the identifier 'identfies'.
    CComponent* comp = m_tree->findComponent(node->getName());
    if (comp != 0) {
        if (isBeingIterated(comp)) {
            result = getIteratedValue(comp, node, node->getProperty(GEN_FIELD));
        } else {
            vector<string> values = m_tree->getValues(comp, node->getProperty(GEN_FIELD));
            if (values.size()>0)
                result = values[0];
        }
        //debug("result = [%s], define field = %s", result.c_str(), comp->getProperty(PROP_DEFINE).c_str());
        result = generator->getMapped(comp->getProperty(PROP_DEFINE), result);
    }
    debug("generateIdentifier() result = [%s]", result.c_str());
    return result;
}

//============================================================================
//! Generates a foreach loop node.
/*!
    Starts an iteration.
 */
void CProcess::generateForEach(CGeneratorComp* generator, CComponent* node, string& result)
{
    ASSERT(node != 0);
    ASSERT(generator != 0);

    CIteratedState* newIterator = new CIteratedState;
    newIterator->m_foreachNode = node;

    CComponent* comp = m_tree->findComponent(node->getProperty(GEN_CONTEXT));
    if (comp == 0 || comp->getType() != KEY_SPREADSHEET) {
	fprintf(stderr,"--foreach: spreadsheet context '%s' not found!\n",
		node->getProperty(GEN_CONTEXT).c_str());
	return;
    }

    string c_id = node->getProperty(GEN_ITERATOR);

    if (c_id == "row") {
        newIterator->m_component = comp;
        newIterator->m_iterator = "row";
        m_iterators.push_back(newIterator);

        debug("Generating for component %s, iterating rows", comp->getName().c_str());

        CSpreadSheetView* sheet = dynamic_cast<CSpreadSheetView*>(m_tree->getWidget(comp));
        if (sheet != 0) {
            debug("Sheet has %d columns, <node> has %d children.", sheet->childCount(), node->numChildren());
            for (newIterator->m_index = 0; newIterator->m_index<sheet->childCount(); newIterator->m_index++) {
                for (int j=0; j<node->numChildren(); ++j)
                    generate(generator, node->getChild(j), result);
            }
        }
    } else {
        comp = comp->getChild(c_id);
        if (comp == 0) {
	    fprintf(stderr,"--foreach: spreadsheet '%s' column '%s' not found!\n",
		node->getProperty(GEN_CONTEXT).c_str(), c_id.c_str());
	    return;
	}

        vector<string> values = m_tree->getValues(comp, node->getProperty(GEN_FIELD));
        sort(values.begin(), values.end());

        vector<string> valToIterate;
        for (unsigned int i=0; i<values.size(); ++i) {
            if (find(valToIterate.begin(), valToIterate.end(), values[i])==valToIterate.end())
                valToIterate.push_back(values[i]);
        }
        newIterator->m_component = comp;
        newIterator->m_iterator = "val";
        newIterator->m_values = valToIterate;
        m_iterators.push_back(newIterator);

        for (unsigned int i=0; i<valToIterate.size(); ++i) {
            debug("Foreach iterating over values, m_index = %d", i);
            newIterator->m_index = i;
            for (int j=0; j<node->numChildren(); ++j)
                generate(generator, node->getChild(j), result);
        }
    }

    vector<CIteratedState*>::iterator pos;
    pos = find(m_iterators.begin(), m_iterators.end(), newIterator);
    ASSERT (pos != m_iterators.end());
    m_iterators.erase(pos);

    delete newIterator;
}

//============================================================================
//! Generates an if-statement node.
/*!
    Unfortunately, due to current parser constraints an else is not supported.

    Selectively traverses one of two branches starting from this node.
 */
void CProcess::generateIf(CGeneratorComp* generator, CComponent* node, string& result)
{
    ASSERT(generator != 0);
    ASSERT(node != 0);
    ASSERT(node->getType() == GEN_IF);

    CComponent* conditionComp = node->getChild(0);
    if (conditionComp != 0) {
        CComponent* l = 0;
        CComponent* r = 0;
        ASSERT(conditionComp->getType() == KEY_IFCONDITION);
        CComponent* left  = conditionComp->getChild(0);
        CComponent* right = conditionComp->getChild(1);
        string cond;

        if (left->getType() == KEY_IFCONDITION) {
	    ASSERT(right->getType() == KEY_IFCONDITION);
	    l = left;
	    r = right;
	    left  = l->getChild(0);
	    right = l->getChild(1);
	    cond  = l->getName();
	}
	else
	    cond = conditionComp->getName();
again:
        string leftval = "";
        string rightval = "";
        debug(">>>> Getting left value");
        generate(generator, left, leftval);
        debug(">>>> Getting right value");
        generate(generator, right, rightval);
        debug("--------------------");
        debug("leftval: [%s] rightval: [%s]", leftval.c_str(), rightval.c_str());
        bool doChildren = false;

        if (cond == "==") {
            if (leftval == rightval)
                doChildren = true;
        }
        else if (cond == "!=") {
            if (leftval != rightval)
                doChildren = true;
        }
	else
	    fprintf(stderr,"--if: unknown condition!\n");

	if (l) {
	    cond = conditionComp->getName();
	    if ((doChildren && cond == "&&") || (!doChildren && cond == "||")) {
		left  = r->getChild(0);
		right = r->getChild(1);
		cond  = r->getName();
		l = 0;
		goto again;
	    }
	}

        if (doChildren) {
            debug("Generating children of IF");
            for (int i=1; i<node->numChildren(); ++i)
                generate(generator, node->getChild(i), result);
        }
	else if ((node = conditionComp->getChild(2))) {
	    debug("Generating children of ELSE");
	    for (int i=0; i<node->numChildren(); ++i)
		generate(generator, node->getChild(i), result);
	}
    }
}

//============================================================================
//! Returns true if the current component is subject to iteration.
bool CProcess::isBeingIterated(CComponent* comp)
{
    for (unsigned int i=0; i<m_iterators.size(); ++i) {
        if (m_iterators[i]->m_iterator == "row")
            comp = comp->getParent();
        if (m_iterators[i]->m_component == comp)
            return true;
    }
    return false;
}

//============================================================================
//! Retrieves the correct value for a component that is being iterated.
string CProcess::getIteratedValue(CComponent* comp, CComponent* node, const string& field)
{
    ASSERT(comp != 0);
    ASSERT(node != 0);

    //comp->dump();
    //node->dump();

    CIteratedState* state = 0;
    string iterIdent = node->getProperty(PROP_ITERIDENT);
    if (iterIdent != "") {
        debug("---------------------------------------------");
        debug(">> iterIdent (from comp) = [%s]", iterIdent.c_str());
        for (unsigned int i=0; i<m_iterators.size(); ++i) {
            string foreachIter = m_iterators[i]->m_foreachNode->getProperty(PROP_ITERIDENT);
            debug(">> foreachIter (from foreach) = [%s]", foreachIter.c_str());
            if ( foreachIter == iterIdent) {
                debug("   setting correct state");
                state = m_iterators[i];
            }
        }
    }

    if (state == 0) {
        CComponent* compare = 0;
        for (unsigned int i=0; i<m_iterators.size(); ++i) {
            if (m_iterators[i]->m_iterator == "row")
                compare = comp->getParent();
            else
                compare = comp;
            if (m_iterators[i]->m_component == compare)
                state = m_iterators[i];
        }
    }

    ASSERT(state != 0);

    if (state->m_iterator == "row") {
        debug("Generating identifier value of [%s] for foreach over rows.", comp->getName().c_str());
        vector<string> values = m_tree->getValues(comp, field);
        //debug("Size of 'values' = %d", values.size());
        debug("Current m_index = %d", state->m_index);
        if (state->m_index < (int)values.size() && state->m_index >= 0) {
            debug("index: %d\tvalue: %s", state->m_index, values[state->m_index].c_str());
            return values[state->m_index];
        }
        debug("   Out of bounds!");
        ASSERT(true);
    }
    if (state->m_iterator == "val") {
        debug("Generating identifier value of [%s] for foreach over values.", comp->getName().c_str());
        debug("  index = %d", state->m_index);
        if (state->m_index < (int)state->m_values.size() && state->m_index >= 0)
            return state->m_values[state->m_index];
    }
    return "";
}

//============================================================================
//! Generates a binary operator type node.
/*!
    Currently only the + and - operator are supported.
 */
void CProcess::generateBinaryNode(CComponent* node, string& result)
{
    string type = node->getType();
//  ASSERT(type == "+" || type == "-");

    CComponent* left  = node->getChild(0);
    CComponent* right = node->getChild(1);
    ASSERT(left != 0 && right != 0); // Must have left/right arguments

    string leftType  = determineNumberType(left);
    string rightType = determineNumberType(right);

    double leftval  = determineNumberValue(left);
    double rightval = determineNumberValue(right);
    double res = 0;

    if (type == "+")
        res = leftval + rightval;
    if (type == "-")
        res = leftval - rightval;

    ostringstream tmp;
    if (leftType == GEN_INTEGER && rightType == GEN_INTEGER)
        tmp << (int)res << ends;
    else
        tmp << res << ends;
    result += tmp.str();
}

//============================================================================
//! Determines of a number is an integer or a real.
string CProcess::determineNumberType(CComponent* node)
{
    string type = node->getType();
    if (type == GEN_IDENTIFIER) {
        CComponent* comp = m_tree->findComponent(node->getName());
        if (comp != 0) {
	    type = comp->getType();
	    if (type == KEY_SPREADSHEET &&
		node->getProperty(GEN_FIELD) == "numrows") type = GEN_INTEGER;
	}
    }
    return type;
}

//============================================================================
//! Converts the component \node value to a double.
double CProcess::determineNumberValue(CComponent* node)
{
    string type = node->getType();

    if (type == GEN_REAL || type == GEN_INTEGER)
        return atof(node->getProperty(GEN_VALUE).c_str());

    if (type == GEN_IDENTIFIER) {
        CComponent* comp = m_tree->findComponent(node->getName());
        if (comp != 0) {
	    type = comp->getType();
	    if (type == KEY_SPREADSHEET) {
		if (node->getProperty(GEN_FIELD) == "numrows") {
		    vector<string> values = m_tree->getValues(comp, "numrows");
		    if (values.size() > 0) return atof(values[0].c_str());
		}
	    }
            else if (type == GEN_REAL || type == GEN_INTEGER) {
                vector<string> values = m_tree->getValues(comp);
                if (values.size() > 0) return atof(values[0].c_str());
            }
        }
    }
    return 0;
}

//============================================================================
//! Retrieves a value fom the application
/*!
    Currently supported are the process name, the process description and the
    date and time.
 */
string CProcess::getApplicationValue(const string& field)
{
//    CProcessManager* thePM = CProcessManager::instance();

    if (field == FIELD_PROCESSNAME)
        return getName();

    if (field == FIELD_PROCESSDESC)
        return getDesc();

    if (field == FIELD_DATE) {
        QString now = QDate::currentDate().toString();
        if (!now.isNull())
            return now.latin1();
        return "";
    }

    if (field == FIELD_TIME) {
        QString now = QTime::currentTime().toString();
        if (!now.isNull())
            return now.latin1();
    }
    return "";
}

//============================================================================
//! Default constructor.
CIteratedState::CIteratedState()
{
    m_component = 0;
    m_foreachNode = 0;
    m_index = 0;
    m_iterator = "";
}
