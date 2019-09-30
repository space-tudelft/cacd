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
#ifndef __PROCESSMANAGER_H__
#define __PROCESSMANAGER_H__

// Qt includes
#include <qobject.h>

// STL
#include <map>
#include <vector>
#include <string>

using namespace std;

// Forward declarations
class QWidget;
class QWidgetStack;
class QTabWidget;
class CGUITree;
class CGeneratorComp;
class CProcess;

//! Manages the processes available in the application.
/*!
    The process manager manages the processes available in the application.
    It controls the processes and the integrates the user interfaces associated
    with these processes into the main application framework.

    \author Xander Burgerhout
*/
class CProcessManager : public QObject
{
    Q_OBJECT

    private:
	// User interface related
	map<CProcess*, QTabWidget*>         m_processTabs;
	QWidgetStack*                       m_stack;
	int                                 m_lastTabWidgetID;

	static CProcessManager*             m_singleton;

	void                    runParser();
	void                    setupWidgets();
	QTabWidget*             createTabWidget(CProcess* proc);
	void                    createTabPages(CProcess* proc);

    protected:
	CProcessManager();

    public:
	~CProcessManager() {}

	// Misc
	static CProcessManager* instance();
	QWidgetStack*           getWidgetStack();


	// process related
	CProcess*               currentProcess();
	CProcess*               newProcess(const string& name = "Unnamed");
	void                    activateProcess(CProcess* proc);
	vector<string>          getProcessNames();
	CProcess*               getProcess(const string& name);
	void                    removeProcess(CProcess* proc);
	bool                    isLoaded(const string& name);

	void                    saveProcess(CProcess* proc);
	void                    loadProcess(const string& name);

	// generation
	void                    generateFile(const string& generatorTitle, string& result);
	void                    generateAllFiles(CProcess* proc);
};

#endif //__PROCESSMANAGER_H__
