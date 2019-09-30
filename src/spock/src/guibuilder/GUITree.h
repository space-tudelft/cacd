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
#ifndef __GUITREE_H__
#define __GUITREE_H__

// Project includes
#include "src/spock/src/datastruct/Visitors.h"

// Qt includes
#include <qobject.h>

// STL includes
#include <map>
#include <vector>
#include <string>

using namespace std;

// Forward declarations
class CComponent;
class CComponentTree;
class QWidget;
class CSpreadSheetView;
class CMultiTypeItem;
class CCell;
class QValidator;
class CDataConnection;
class QDataStream;
class QFile;
class CProcess;

//! A class containing the widgets associated with the components in the component tree.
/*!
    The CGUITree object is created for each process open in the GUI
    framework (each ``file). The CGUITree is not really implemented as a tree
    but rather as a mapping between the components in the component tree and
    the widgets created by the CGUIBuilderVisitor. However, the widgets created
    do form a tree, although it is managed by Qt. This class is the interface to
    access that tree and its relationship to the component tree, therefore the
    name CGUITree is quite acceptable.

    This class manages the connections between the widgets and the components
    in the component tree. This means there are methods available related to
    the widget <-> component mapping and the data connections.

    \sa CDataConnection, CComponentTree, CGUIBuilderVisitor.

    \author Xander Burgerhout
*/
class CGUITree : public QObject
{
    Q_OBJECT

    private:
	CComponentTree*             m_componentTree;
	map<CComponent*, QWidget*>  m_mapping;
	vector<CDataConnection*>    m_dataConnections;
	bool m_fileGen;

	void buildEditCell (CMultiTypeItem* item, int col, CComponent* comp);
	void buildComboCell(CMultiTypeItem* item, int col, CComponent* comp);
	void buildDaliCell (CMultiTypeItem* item, int col, CComponent* comp);
	void buildColorCell(CMultiTypeItem* item, int col, CComponent* comp);
	void buildConditionListCell(CMultiTypeItem* item, int col, CComponent* comp);

	CDataConnection*    isADataTarget(CComponent* comp);

    public:
	CGUITree(CComponentTree* tree);
	void                makeMapping(CComponent* component, QWidget* widget);
	void                addDataConnection(CDataConnection*);

	CComponent*         findComponent(const string& name);
	CComponent*         getComponent(QWidget* wdgt);
	QWidget*            getWidget(CComponent* component);
	vector<QWidget*>    getTabPages();
	CComponentTree*     getComponentTree();

	vector<string>      getValues(CComponent* comp, const string& field = "");
	void                setValues(CComponent* comp, const vector<string>& values);
	vector<string>      getSpreadSheetValues(CComponent* sheet, CComponent* comp, const string& field = "");
	void                setSpreadSheetValues(CComponent* sheet, CComponent* comp, const vector<string>& values);

	QValidator*         buildValidator(QWidget* widget, const string& type, CComponent* comp);
	void                buildDataConnections();
	void                disableDataConnectionUpdates();
	void                enableDataConnectionUpdates();

	bool isFileGen();
	void toggleFileGen();

    public slots:
	void                onPleaseCreateCell(CMultiTypeItem* item, int column);
};

#endif // __GUITREE_H__
