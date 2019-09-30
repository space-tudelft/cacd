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
#ifndef __SERIALIZERVISITOR_H__
#define __SERIALIZERVISITOR_H__

// Project includes
#include "src/spock/src/datastruct/Visitors.h"

// STL includes
#include <vector>
#include <string>

// Forward declarations
class CGUITree;
class CComponent;
class QTextStream;
class QFile;

//! Serializes a process.
/*!
    The information entered into the application can be saved for later use.
    This means the processes entered by the user must be serialized to disk.
    This class takes care of the serialization process.

    \sa CProcess

    \author Xander Burgerhout
*/
class CSerializerVisitor : public CComponentVisitor
{
    private:
	CGUITree*           m_tree;
	string              m_processName;
	string              m_processDesc;
	bool                m_isSaving;
	string              m_fileName;
	QTextStream*        m_str;
	QFile*              m_file;
	bool                m_isValid;

	int                 m_fileVerMajor;
	int                 m_fileVerMinor;

	bool                m_calledFromSerialize;

	void                readConversion(string& identifier, vector<string>& values);
	void                writeConversion(string& identifier, vector<string>& values);

	vector<string>      getValues(CComponent* comp);
	void                setValues(CComponent* comp, const vector<string>& values);

	vector<string>      getSpreadSheetValues(CComponent* sheet, CComponent* comp);
	void                setSpreadSheetValues(CComponent* sheet, CComponent* comp,
						 const vector<string>& values);

	CComponent*         guessCorrectComponent(const string& identifier);
	void                doFileInit();
	void                load();

    public:
	CSerializerVisitor(CGUITree* tree, const string& fileName, const string& processName = "",
			   const string& processDesc = "",bool isSaving = false);

	bool                isValid();
	bool                isSaving();
	void                visitComponent(CComponent* comp);

	string              getProcessName();
	string              getProcessDesc();
	void                serialize();
};

#endif // __SERIALIZERVISITOR_H__
