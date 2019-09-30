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
#ifndef GENERATEDLG_H
#define GENERATEDLG_H

// Project includes
#include "src/spock/src/gui/dialogs/GenerateDlg/GenerateDlgInternal.h"

// Qt includes
#include <qdialog.h>

// STL includes
#include <string>
#include <vector>

using namespace std;

// Forward declarations
class QCheckBox;
class QBoxLayout;

//! Represent an entry in the processlist file.
/*!
    This is a small utility class used for parsing the processlist file. It us
    used by the technology file generateion dialog box.

    \sa CGenerateDlg

    \author Xander Burgerhout
*/
class CProcessListEntry
{
    private:
	int     m_nr;
	string  m_name;
	string  m_desc;

    public:
	CProcessListEntry();
	CProcessListEntry(const CProcessListEntry& src);

	CProcessListEntry&  operator=(const CProcessListEntry& src);

	int                 nr();
	void                setNr(int nr);

	string              name();
	void                setName(const string& name);

	string              desc();
	void                setDesc(const string& desc);

	void                buildFromString(const string& src);
	bool                isValid();
};

//! A dialog box used to generate the technology files.
/*!
    This dialog box is used to generate the technology files.
    The technology files to generate can be selected in this dialog box. The
    generation destination directory can be either the SPACE process tree or
    a directory given by the user.

    \image html generatedlg.jpg
    \image latex generatedlg.eps "CGenerateDlg screenshot" width=10cm

    This dialog has been created with Qt Designer as an experiment.

    \author Xander Burgerhout
*/
class CGenerateDlg : public GenerateDlgInternal
{
    Q_OBJECT

    private:
	string                      m_processNr;
	string                      m_newProcName;
	string                      m_newProcDesc;

	bool                        m_genToDir;
	vector<CProcessListEntry>   m_processDesc;

	QBoxLayout*                 m_layoutFileFrame;
	vector<string>              m_fileList;
	vector<QCheckBox*>          m_fileChecks;

	void                rebuildFileFrame();
	void                fillProcessNrCombo();
	vector<string>      readProcesses();

    public:
	CGenerateDlg( QWidget* parent = 0, const string& procName = "", const string& procDesc = "");
	~CGenerateDlg();

	bool                wantExamineResults();
	bool                mustGenerateProcessList();
	string              selectedDirectory();
	void                setGenToDir(bool val = true);
	void                setFileList(const vector<string>& fileList);
	vector<string>      selectedFiles();
	void                generateProcessList();

    public slots:
	virtual void        onClickedRadioDir();
	virtual void        onClickedRadioProcessNr();
	virtual void        onBrowseDirectory();
	virtual void        onNewProcessNr();

	virtual void        polish();
};

#endif // GENERATEDLGL_H
