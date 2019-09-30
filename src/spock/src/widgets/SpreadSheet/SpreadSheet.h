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
#ifndef __SPREADSHEET_H__
#define __SPREADSHEET_H__

// Qt includes
#include <qlistview.h>

// STL include
#include <vector>

using namespace std;

// Forward declarations.
class CCell;
class CMultiTypeItem;
class QPopupMenu;

//! A widget that is mix between a spreadsheet and a QListView.
/*!
    For a more elaborate description of the spreadsheet widget, refer to the
    authors graduation report.

    The spreadsheetview is derived from a QListView. At the time development
    started, Qt 2.2.x was not yet available. Meanwhile, Qt 2.2.x has become
    available and it includes a spreadsheet widget. This spreadsheet widget
    probably provides a better base for this class. Future version of the
    application might change to a Qt spreadsheet based implementation.

    The rows in the spreadsheet are CMultiTypeItem objects.

    \image html spreadsheet.jpg
    \image latex spreadsheet.eps "A CSpreadSheetView widget in action" width=10cm

    \sa CMultiTypeItem, CCell

    \author Xander Burgerhout
*/
class CSpreadSheetView : public QListView
{
    Q_OBJECT

    private:
	int             m_currFocusCol;
	CCell*          m_inEditMode;
	QPopupMenu*     m_popupMenu;

	int             columnAt(int x);

	enum {POPUP_REMOVE = 0, POPUP_ADD, POPUP_INSERTABOVE, POPUP_INSERTBELOW,
	      POPUP_MOVEUP, POPUP_MOVEDOWN};

    protected:
	virtual bool    focusNextPrevChild(bool next);
	virtual void    focusOutEvent(QFocusEvent* e);
	virtual void    keyPressEvent(QKeyEvent* e);
	virtual void    contentsMousePressEvent(QMouseEvent* e);
	virtual void    contentsMouseDoubleClickEvent(QMouseEvent* e) ;

    protected slots:
	virtual void    onHeaderSizeChange();
	virtual void    onValueChanged(int col);
	virtual void    onRightButtonClicked(QListViewItem *item, const QPoint &p, int c);

    public:
	CSpreadSheetView(QWidget* pParent = 0, const char* name = 0);

	int             currentFocusCol();

	virtual int     addColumn(const QString& label, int width = -1);
	virtual int     addColumn(const QIconSet& iconset, const QString& label, int width = -1);
	virtual void    deleteColumn(int column);
	// removeColumn() is not virtual, so we cannot overload it.
	// According to the Qt source for QListView, it will be virtual in the
	// next major release.
	// Care should be taken to use deleteColumn in stead of removeColumn()!
	virtual int     findColumn(const QString& label);

	void            startEdit();
	void            endEdit();

	void            moveRowUp(QListViewItem* item);
	void            moveRowDown(QListViewItem* item);
	void            insertItemBelow(QListViewItem* item);
	void            insertItemAbove(QListViewItem* item);
	void            addNewRow();
	void            deleteRow(QListViewItem* item);

	virtual QString getValue(CMultiTypeItem* item, int col);
	virtual bool    setValue(CMultiTypeItem* item, int col, const QString& value);
	virtual int     getIntValue(CMultiTypeItem* item, int col);
	virtual void    setIntValue(CMultiTypeItem* item, int col, int newVal);

	virtual QSizePolicy sizePolicy() const;
	virtual QSize   sizeHint() const;

	void            emitPleaseCreateCell(CMultiTypeItem*, int column);

    signals:
	void            pleaseCreateCell(CMultiTypeItem*, int);
	void            valueChanged(CSpreadSheetView*, int); // source sheet, column changed
};

//! Implements a spreadsheet row.
/*!
    Like the CSpreadSheetView is derived from the QListView, the CMultiTypeItem
    is derived from the QListViewItem. The CMultiTypeItem allows all sorts of
    cells with different edit modes and displays. The implementation is open,
    so new cells can easily be added and used.

    \sa CCell

    \author Xander Burgerhout
*/
class CMultiTypeItem : public QObject, public QListViewItem
{
    Q_OBJECT

    private:
	vector<CCell*>      m_cells;

	void                init(QListView* parent);

    public:
	CMultiTypeItem(QListView* parent);
	CMultiTypeItem(QListView* parent, QListViewItem* after);
	~CMultiTypeItem();

	virtual void        addColumn();
	virtual void        deleteColumn(int index);
	virtual int         findColumn(CCell* cell);

	virtual void        paintCell(QPainter *p, const QColorGroup& cg,
					int column, int width, int align);
	virtual void        paintFocus(QPainter *p, const QColorGroup& cg, const QRect& r);

	virtual CCell*      getCell(int column);
	virtual void        setCell(int column, CCell* pCell);

	virtual CCell*      startEdit(int column);
};

#endif
