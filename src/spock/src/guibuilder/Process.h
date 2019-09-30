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
#ifndef __A_PROCESS_H__
#define __A_PROCESS_H__

// STL includes
#include <string>
#include <map>
#include <vector>

using namespace std;

// Forward declarations
class CGUITree;
class CGenerators;
class CComponent;
class CGeneratorComp;

//! A small class containing the state of a foreach loop.
/*!
    \author Xander Burgerhout
*/
class CIteratedState
{
    public:
	CIteratedState();

	CComponent*     m_component;
	CComponent*     m_foreachNode;

	int             m_index;
	string          m_iterator;
	vector<string>  m_values;
};

//! Contains a process.
/*!
    The class name does not fully cover the functionality of this class.
    Perhaps this class should be split in a future version.

    The CProcess class contains methods to set and get the process name
    and description. Also included are some methods to start the serialization
    process of this CProcess object.

    The actual file generation code is also inlcuded in this class, and this
    should really move to it's own class in a future version.

    \author Xander Burgerhout
*/
class CProcess
{
    private:
	CGUITree*       m_tree;
	CGenerators*    m_generators;

	string          m_processName;
	string          m_processDesc;
	string          m_fileName;
	bool            m_hasChanged;

	// Generation...
	vector<CIteratedState*> m_iterators;
	bool            isBeingIterated(CComponent* comp);
	string          getIteratedValue(CComponent* comp, CComponent* node, const string& field = "");

	void            generate(CGeneratorComp* generator, CComponent* node, string& result);
	string          generateIdentifier(CGeneratorComp* generator, CComponent* node);
	void            generateForEach(CGeneratorComp* generator, CComponent* node, string& result);
	void            generateBinaryNode(CComponent* node, string& result);
	void            generateIf(CGeneratorComp* generator, CComponent* node, string& result);
	string          determineNumberType(CComponent* node);
	double          determineNumberValue(CComponent* node);
	string          getApplicationValue(const string& field);

    public:
	CProcess();

	CGUITree*       guiTree();
	void            setGUITree(CGUITree* tree);
	string          getName();
	void            setName(const string& name);
	string          getDesc();
	void            setDesc(const string& name);
	string          getFileName();
	void            setFileName(const string& name);
	void            toggleFileGen();

	void            save();
	bool            load();

	void            generateFile(const string& generatorTitle, string& result);
	vector<string>  getGeneratorTitles();
	string          getGeneratorFileName(const string& generatorTitle);
	map<string, string>  generateAllFiles();
};

#endif // __A_PROCESS_H__
