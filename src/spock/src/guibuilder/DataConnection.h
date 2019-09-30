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
#ifndef __DATACONNECTION_H__
#define __DATACONNECTION_H_

// Qt includes
#include <qobject.h>

// STL includes
#include <string>
#include <vector>

using namespace std;

// Forward declarations
class CComponent;
class CSpreadSheetView;
class CGUITree;
class QComboBox;

//////////////////////////////////////////////////////////////////////////////
// Base classes for datasource, datatarget and a dataconnection
//////////////////////////////////////////////////////////////////////////////

//! Models a data source
/*!
    Currently, only one type of datasource is supported.
    Only spreadsheet columns can act as a data source.

    If new data source types are required, this class should probably change
    to an abstract base class and the spreadsheet dependent implementation
    should move to a derived class. This is the way it is done in the
    CDataTarget and derived classes.

    \author Xander Burgerhout

    \sa CDataConnection, CDataTarget
*/
class CDataSource : public QObject
{
    Q_OBJECT

    protected:
	CGUITree*               m_tree;

	CComponent*             m_sourceComp;
	CSpreadSheetView*       m_sourceSheet;
	int                     m_sourceColumn;

	CGUITree*               guiTree();

    protected slots:
	virtual void    onValueChanged(CSpreadSheetView* sheet, int col);

    public:
	CDataSource(CGUITree* tree, CComponent* comp);

	virtual vector<string>  getValues(bool add = false);

    signals:
	void                    valueChanged();
};

//! Models a data target
/*!
    This is an abstract base class.

    If new data source types are required, inherit from this class and implement
    an updateValues() method.

    There are already some derived classes available, like CColumnDataTarget,
    CConditionListDataTarget and CComboDataTarget.

    \author Xander Burgerhout

    \sa CDataConnection, CDataSource
*/
class CDataTarget
{
    private:
	CGUITree*               m_tree;
	CComponent*             m_targetComp;

    protected:
	CGUITree*               guiTree();

    public:
	CDataTarget(CGUITree* tree, CComponent* targetComp);
	virtual ~CDataTarget() {};

	virtual void            updateValues(const vector<CDataSource*>& sources, int s) = 0;
	CComponent*             component();
};

//! Models a data connection.
/*!
    A data connection consists of the source(s) the and the target conected by
    a link. The link is implemented by this class, the source(s) with the
    CDataSource class and the target with the CDataTarget class.

    The connection controls the synchronization of the source with the target.
    The synchronization can be turned on and off with the enable() and disable()
    methods.

    \sa CDataTarget, CDataSource

    \author Xander Burgerhout
 */
class CDataConnection : public QObject
{
    Q_OBJECT

    private:
	vector<CDataSource*>    m_sources;
	CDataTarget*            m_target;
	bool                    m_enabled;

    public:
	CDataConnection(CDataTarget* target);
	void                    disable();
	void                    enable();
	void                    addDataSource(CDataSource* source);
	bool                    isTarget(CComponent* comp);

    public slots:
	void                    onSynchronize();
	void                    onSynchronize2();
};

//! Implements a data target that is a spreadsheet column
/*!
    The updateValues() method is implemented in a way that allows updates of
    spread sheet column cells.

    \author Xander Burgerhout
 */
class CColumnDataTarget : public CDataTarget
{
    private:
	CSpreadSheetView*       m_targetSheetWidget;
	int                     m_targetColumn;

    public:
	CColumnDataTarget(CGUITree* tree, CComponent* targetComp);

	virtual void            updateValues(const vector<CDataSource*>& sources, int s);
};

//! Implements a data target that is a combobox or dropdown
/*!
    \author Xander Burgerhout
*/
class CComboDataTarget : public CDataTarget
{
    private:
	QComboBox*              m_targetCombo;

    public:
	CComboDataTarget(CGUITree* tree, CComponent* targetComp);

	virtual void            updateValues(const vector<CDataSource*>& sources, int s);
};

//! Implements a data target that is a condition list.
/*!
    The condition list data target can have one or two data sources.
    The first data source is used for the layer names. The second (optional)
    data source is used for the layer comments.

    \author Xander Burgerhout
*/
class CConditionListDataTarget : public CDataTarget
{
    private:
	CSpreadSheetView*       m_targetSheetWidget;
	int                     m_targetColumn;

    public:
	CConditionListDataTarget(CGUITree* tree, CComponent* targetComp);

	virtual void    updateValues(const vector<CDataSource*>& sources, int s);
};

#endif // __DATACONNECTION_H_
