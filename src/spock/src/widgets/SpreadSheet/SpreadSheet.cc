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
#include "src/spock/src/widgets/SpreadSheet/SpreadSheet.h"
#include "src/spock/src/widgets/SpreadSheet/Cell.h"

// Qt includes
#include <qpalette.h>
#include <qheader.h>
#include <qpopupmenu.h>

extern string oldValue;
extern string newValue;
static bool rowUpDown = false;

using namespace std;

//============================================================================
//! Default constructor.
/*!
    The constructor initializes the spreadsheet widget to its defaults.

    \param pParent  The parent widget
    \param name     The QObject name for this widget

    The constructor creates the right click pop-up menu and connects it to
    the onRightButtonClicked() slot.
 */
CSpreadSheetView::CSpreadSheetView(QWidget* pParent, const char* name)
    : QListView(pParent, name)
{
    m_currFocusCol = 0;
    m_inEditMode = 0;
    m_popupMenu = 0;

    setAllColumnsShowFocus(true);
    setSelectionMode(Multi);
    setSorting(-1); // disable sorting
//  setShowSortIndicator(true);

    m_popupMenu = new QPopupMenu;
    m_popupMenu->insertItem("&Remove row", POPUP_REMOVE);
    m_popupMenu->insertItem("&Add new row", POPUP_ADD);
    m_popupMenu->insertItem("Insert row &above", POPUP_INSERTABOVE);
    m_popupMenu->insertItem("Insert row &below", POPUP_INSERTBELOW);
    m_popupMenu->insertSeparator();
    m_popupMenu->insertItem("Move row &up", POPUP_MOVEUP);
    m_popupMenu->insertItem("Move row &down", POPUP_MOVEDOWN);

    connect(this, SIGNAL(rightButtonClicked(QListViewItem*, const QPoint&, int)),
            this, SLOT(onRightButtonClicked(QListViewItem*, const QPoint&, int)));

    connect(header(), SIGNAL(sizeChange(int, int, int)), this, SLOT(onHeaderSizeChange()));
    header()->setMovingEnabled(false);  // Make available in next version

    setMinimumHeight(25);
    updateGeometry();
}

//============================================================================
//! Returns the column number found at x-coordinate \a x.
/*!
    Calculates the column at \a x. Returns 0 if the column number of \a x
    could not be determined. The x-coordinate is assummed to be from the left
    side of the spreadsheet.
 */
int CSpreadSheetView::columnAt(int x)
{
    int left = 0;
    for (int i=0; i<columns(); ++i) {
        left += columnWidth(i);
        if (x<=left)
            return i;
    }
    return 0;
}

//============================================================================
//! Reimplemented from QListView.
/*!
    Moves focus to the next or previous cell.

    \param next If true focus moves right. If false focus moves left.

    If the focus reaches the end of a row, focus moves to the beginning of the
    next row. The edit mode of the spreadsheet is honored. This means that if
    the current cell is in edit mode, the next cell will aso be in edit mode.
    If the current cell is in edit mode and the current cell is also the last
    cell in the spreadsheet, a new row is added.

    Do not call this method directly. Focus is handled by Qt.
 */
bool CSpreadSheetView::focusNextPrevChild(bool next)
{
    QListViewItem* nextItem = currentItem();
    if (!nextItem) return QListView::focusNextPrevChild(next);

    clearSelection();
    bool restoreEdit = false;
    if (m_inEditMode != 0) {
        endEdit();
        restoreEdit = true;
    }

    if (next) { // To the right within row
	if (m_currFocusCol<columns()-1 && !rowUpDown) {
	    m_currFocusCol++;
	    // repaintItem(nextItem);
	    // if (restoreEdit) startEdit();
	    // return false;
	}
	else { // To the right, next row
	    if (nextItem->nextSibling() != 0) {
		if (!rowUpDown) m_currFocusCol = 0;
		nextItem = nextItem->nextSibling();
	    } else if (restoreEdit) { // No next, create it if we were editing.
		nextItem = new CMultiTypeItem(this, nextItem);
		if (!rowUpDown) m_currFocusCol = 0;
	    }
	}
    }
    else { // To the left within row
	if (m_currFocusCol>0 && !rowUpDown) {
	    m_currFocusCol--;
	    // repaintItem(nextItem);
	    // if (restoreEdit) startEdit();
	    // return false;
	}
	else {
	    QListViewItem* find = 0;
	    QListViewItem* prev = 0;
	    find = firstChild();
	    while (find != 0 && find != nextItem) {
		prev = find;
		find = find->nextSibling();
	    }
	    if (find != 0 && find != firstChild()) {
		if (!rowUpDown) m_currFocusCol = columns() - 1;
		nextItem = prev;
	    }
	}
    }

    setCurrentItem(nextItem);
    repaintItem(nextItem);
    if (restoreEdit) startEdit();
    return false;
}

//============================================================================
//! Reimplemented from QListView
/*!
    Called when the spreadsheet is about to lose focus. In this case, the
    current selection is cleared. This makes sure that only one spreadsheetview
    has a selection at any given time.
 */
void CSpreadSheetView::focusOutEvent(QFocusEvent* e)
{
    QListView::focusOutEvent(e);
    clearSelection();
}

//============================================================================
//! Reimplemented.
/*!
    This method is called whenever a key is pressed. The spreadsheet reacts to
    the cursor keys to move the focus rectangle.
    It also detects insert and delete, used to insert or delete a row.
    Edit mode os started/ended by pressing return, enter or F2.
 */
void CSpreadSheetView::keyPressEvent(QKeyEvent* e)
{
    //debug("CSpreadSheetView::keyPressEvent(%d)", e->key());

    clearSelection();

    bool passOn = true;

    switch (e->key()) {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        case Qt::Key_F2:
            if (m_inEditMode == 0)
                startEdit();
            else
                if (e->key() != Qt::Key_F2)
                    endEdit();
            passOn = false;
            break;
        case Qt::Key_Left:
        case Qt::Key_F3:
            passOn = false;
            focusNextPrevChild(false);
            break;
        case Qt::Key_Right:
        case Qt::Key_F4:
            passOn = false;
            focusNextPrevChild(true);
            break;
        case Qt::Key_Down:
            if (e->state() == ControlButton) {
                moveRowDown(currentItem());
                if (currentItem() != 0)
                    setCurrentItem(currentItem()->itemAbove());
            } else {
		passOn = false;
		rowUpDown = true;
		focusNextPrevChild(true);
		rowUpDown = false;
            }
            break;
        case Qt::Key_Up:
            if (e->state() == ControlButton) {
                moveRowUp(currentItem());
                if (currentItem() != 0)
                    setCurrentItem(currentItem()->itemBelow());
            } else {
		passOn = false;
		rowUpDown = true;
		focusNextPrevChild(false);
		rowUpDown = false;
            }
            break;
        case Qt::Key_Insert:
            if (m_inEditMode == 0)
                insertItemBelow(currentItem());
            break;
        case Qt::Key_Delete:
            if (m_inEditMode == 0)
                deleteRow(currentItem());
            break;
    }
    if (passOn == true)
        QListView::keyPressEvent(e);
    else
        debug("Keypress accepted");
}

//============================================================================
//! Reimplemented.
/*!
    Detects mouse clicks and moves the focus rectangle. Moving the focus
    rectangle ends edit mode.
 */
void CSpreadSheetView::contentsMousePressEvent(QMouseEvent* e)
{
    QPoint p = e->pos();
    p.setX(p.x()+contentsX());
    p.setY(p.y()+contentsY());

    QListViewItem* item = itemAt(p);

    if (item != 0) {
        endEdit();
        setCurrentItem(item);
        m_currFocusCol = columnAt(e->pos().x());
        setFocus();
        repaintItem(item);
    }

    QListView::contentsMousePressEvent(e);
}

//============================================================================
//! Reimplemented
/*!
    Called in case of a double mouse click. In this case, the cell that was
    double clicked on switches to edit mode.
 */
void CSpreadSheetView::contentsMouseDoubleClickEvent(QMouseEvent* e)
{
    QPoint p = e->pos();
    p.setX(p.x()+contentsX());
    p.setY(p.y()+contentsY());

    QListViewItem* item = itemAt(p);
    if (item != 0) {
        setCurrentItem(item);
        m_currFocusCol = columnAt(e->pos().x());
        repaintItem(item);
        startEdit();
    }

    QListView::contentsMouseDoubleClickEvent(e);
}

//============================================================================
//! Changing the header size ends edit mode.
/*!
    This is to prevent strange effects, like an in-place edit that is floating
    above the wrong cell.

    It is also possible to move the edit widget to the new location, but that
    is a lot of extra work. Would be nice in a future version.
 */
void CSpreadSheetView::onHeaderSizeChange()
{
    endEdit();
}

//============================================================================
//! Called when a cell changes value.
/*!
    If a cell changes value this method will emit the valueChanged() signal.
    This signal also commincates the spreadsview that was changed as well as
    the column number.
 */
void CSpreadSheetView::onValueChanged(int col)
{
    emit valueChanged(this, col);
}

//============================================================================
//! Reimplemented
/*!
    Called if the user clicks the right mouse over a spreadsheet.

    \param item Is the item that was clicked on.
    \param p    The location of the click.

    The spreadsheet responds by displaying a right click pop-up menu.
    The menu offers actions to delete the row, insert a row above or below
    the clicked item, move the row up or down or just adds a row.
 */
void CSpreadSheetView::onRightButtonClicked(QListViewItem* item, const QPoint& p, int /*c*/)
{
    if (item == 0)
        m_popupMenu->setItemEnabled(POPUP_REMOVE, false);
    int result = m_popupMenu->exec(p);

    switch (result) {
        case POPUP_REMOVE:
            deleteRow(item);
            break;
        case POPUP_INSERTABOVE:
            insertItemAbove(item);
            break;
        case POPUP_INSERTBELOW:
            insertItemBelow(item);
            break;
        case POPUP_ADD:
            addNewRow();
            break;
        case POPUP_MOVEUP:
            moveRowUp(item);
            break;
        case POPUP_MOVEDOWN:
            moveRowDown(item);
            break;
    }

    m_popupMenu->setItemEnabled(POPUP_REMOVE, true);
}

//============================================================================
//! Returns the column number of the cell with focus
/*!
    \return The column number of the cell with focus
 */
int CSpreadSheetView::currentFocusCol()
{
    return m_currFocusCol;
}

//============================================================================
//! Adds a column to the spreadsheet.
/*!
    \param label The title for the new column.
    \param width The initial width for the new column.

    Columns are added to the right of the last column in the spreadsheet.

    \return The new columns index.
 */
int CSpreadSheetView::addColumn(const QString& label, int width)
{
    int ret = QListView::addColumn(label, width);

    QListViewItem* item = firstChild();

    while(item !=0) {
        CMultiTypeItem* multi = dynamic_cast<CMultiTypeItem*>(item);
        if (multi != 0)
            multi->addColumn();
        item = item->nextSibling();
    }
    return ret;
}

//============================================================================
//! Adds a column to the spreadsheet.
/*!
    \param iconset  The iconset used in the title.
    \param label    The title for the new column.
    \param width    The initial width for the new column.

    Columns are added to the right of the last column in the spreadsheet.

    \return The new columns index.
 */
int CSpreadSheetView::addColumn(const QIconSet& iconset, const QString& label, int width)
{
    int ret = QListView::addColumn(iconset, label, width);

    QListViewItem* item = firstChild();

    while(item !=0) {
        CMultiTypeItem* multi = dynamic_cast<CMultiTypeItem*>(item);
        if (multi != 0)
            multi->addColumn();
        item = item->nextSibling();
    }
    return ret;
}

//============================================================================
//! Removes a column from the spreadsheet.
/*!
    The QListView class offers a removeColumn() method. However, this method
    was not declared virtual and cannot be overridden. This is a bug in Qt
    that will be fixed in Qt 3.0. We need a way to safely remove columns so
    use deleteColumn instead of removeColumn.

    \param column   The index of the column to remove.
 */
void CSpreadSheetView::deleteColumn(int column)
{
    QListViewItem* item = firstChild();

    while(item != 0) {
        CMultiTypeItem* multi = dynamic_cast<CMultiTypeItem*>(item);
        if (multi != 0)
            multi->deleteColumn(column);
        item = item->nextSibling();
    }

    QListView::removeColumn(column);
}

//============================================================================
//! Returns the index of the column named \a label.
/*!
    \param label The name of the column.
    \return The index of the column named \a label.
 */
int CSpreadSheetView::findColumn(const QString& label)
{
    QHeader* hdr = header();
    for (int i=0; i<hdr->count(); ++i) {
        if (label == hdr->label(i))
            return i;
    }
    return -1;
}

//============================================================================
//! Starts edit mode on the current cell.
/*!
    The current cell is the cell that has focus.
 */
void CSpreadSheetView::startEdit()
{
    endEdit();
    clearSelection();
    CMultiTypeItem* multi = dynamic_cast<CMultiTypeItem*>(currentItem());
    if (multi != 0)
        m_inEditMode = multi->startEdit(currentFocusCol());
}

//============================================================================
//! Ends edit mode.
void CSpreadSheetView::endEdit()
{
    if (m_inEditMode != 0) {
        m_inEditMode->endEdit();
        m_inEditMode = 0;
    }
}

//============================================================================
//! Moves the current row one row up.
/*!
    \sa moveRowDown().
 */
void CSpreadSheetView::moveRowUp(QListViewItem* item)
{
    if (item != 0) {
        QListViewItem* above = item->itemAbove();
        QListViewItem* after = 0;
        if (above != 0) {
            after = above->itemAbove();
            if (after != 0)
                item->moveItem(after);
            else
                moveRowDown(above);
        }
    }
}

//============================================================================
//! Moves the current row one row down.
/*!
    \sa moveRowUp().
 */
void CSpreadSheetView::moveRowDown(QListViewItem* item)
{
    if (item != 0)
        item->moveItem(item->nextSibling());
}

//============================================================================
//! Inserts a row below the current row.
/*!
    \sa insertItemAbove()
 */
void CSpreadSheetView::insertItemBelow(QListViewItem* item)
{
    if (item != 0)
        new CMultiTypeItem(this, item);
    else
        new CMultiTypeItem(this);
}

//============================================================================
//! Inserts a row above the current row.
/*!
    \sa insertItemBelow()
 */
void CSpreadSheetView::insertItemAbove(QListViewItem* item)
{
    if (item != 0) {
        QListViewItem* above = item->itemAbove();
        new CMultiTypeItem(this, above);
    } else
        insertItemBelow(item);
}

//============================================================================
//! Adds a new row
/*!
    The new row is added at end, below all other rows.
    Emits the valueChanged() signal for each column.
 */
void CSpreadSheetView::addNewRow()
{
    QListViewItem* a;
    QListViewItem* after = firstChild();
    if (after)
	while ((a = after->itemBelow())) after = a;
    new CMultiTypeItem(this, after);
}

//============================================================================
//! Deletes a row.
/*!
    \param item The row to delete

    Emits the valueChanged() signal for all columns.
 */
void CSpreadSheetView::deleteRow(QListViewItem* item)
{
    if (item != 0) {
	vector<string> oldv;
	string old;
	newValue = "";
	int nr = columns();
	for (int i=0; i<nr; ++i) {
	    if (item->text(i)) old = item->text(i).latin1();
	    else old = "";
	    oldv.push_back(old);
	}
        takeItem(item);
        delete item;
	for (int i=0; i<nr; ++i) {
	    oldValue = oldv[i];
	    if (oldValue != "") emit valueChanged(this, i);
	}
    }
}

//============================================================================
//! Retrieves the value in a cell.
/*!
    \param item The row coordinate of the cell
    \param col  The column index of the cell

    \returns The value of the call in \a item at \a col.

    \sa setValue(), getIntValue(), setIntValue()
 */
QString CSpreadSheetView::getValue(CMultiTypeItem* item, int col)
{
    if (item != 0) {
        CCell* cell = item->getCell(col);
        if (cell != 0)
            return cell->getValue();
    }
    return "";
}

//============================================================================
//! Sets the value in a cell.
/*!
    \param item The row coordinate of the cell
    \param col  The column index of the cell
    \param value The new cell value

    \sa getValue(), getIntValue(), setIntValue()
 */
bool CSpreadSheetView::setValue(CMultiTypeItem* item, int col, const QString& value)
{
    if (item != 0) {
        CCell* cell = item->getCell(col);
	if (cell != 0) {
	    cell->setValue(value);
	    return true;
	}
    }
    return false;
}

//============================================================================
//! Gets the integer value of a cell
/*!
    Retrieves the value of a cell as an integer.

    Can be reimplemented if special behaviour is desired.

    \sa getValue(), setValue(), setIntValue()

    \return The integer value of a cell.
 */
int CSpreadSheetView::getIntValue(CMultiTypeItem* item, int col)
{
    if (item != 0) {
        CCell* cell = item->getCell(col);
        if (cell != 0)
            return cell->getIntValue();
    }
    return 0;
}

//============================================================================
//! Sets the integer value of a cell
/*!
    Sets the value of a cell using an integer.

    Can be reimplemented if special behaviour is desired.

    \sa getValue(), setValue(), getIntValue()
 */
void CSpreadSheetView::setIntValue(CMultiTypeItem* item, int col, int value)
{
    if (item != 0) {
        CCell* cell = item->getCell(col);
        if (cell != 0)
            cell->setIntValue(value);
    }
}

//============================================================================
//! Can be called if a cell needs to be created.
/*!
    Emits both the pleaseCreateCell() and valueChanged() signal if
    \a item is not 0.

    \param item   Item to create a cell for
    \param column Index of the column
 */
void CSpreadSheetView::emitPleaseCreateCell(CMultiTypeItem* item, int column)
{
    if (item != 0) {
        emit pleaseCreateCell(item, column);
    }
}

//============================================================================
//! Reimplemented
/*!
    \returns The preferred size of the spreadsheet.
 */
QSize CSpreadSheetView::sizeHint() const
{
    int w = header()->sizeHint().width();
    int h = header()->height();

    if (verticalScrollBar() != 0)
        w += verticalScrollBar()->width();

    if (horizontalScrollBar() != 0)
        h += horizontalScrollBar()->height();

    QListViewItem* item = firstChild();
    while (item != 0) {
        h += item->height();
        item = item->nextSibling();
    }

    QSize size(w, h);
    // debug("Spreadsheet recommended size = %d x %d", w, h);
    return size;
}

//============================================================================
//! Reimplemented
/*!
    \return The spreadsheets size policy.
 */
QSizePolicy CSpreadSheetView::sizePolicy() const
{
    return QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

//============================================================================
//! Default constructor
/*!
    \param parent The spreadsheetview that owns this item.
 */
CMultiTypeItem::CMultiTypeItem(QListView* parent)
    : QListViewItem(parent)
{
    init(parent);
}

//============================================================================
//! Inserts itself into the parent spreadsheet
/*!
    \param item The parent spreadsheet that will own this item.
    \param after The item after which this item will be placed.
 */
CMultiTypeItem::CMultiTypeItem(QListView* parent, QListViewItem* after)
    : QListViewItem(parent, after)
{
    init(parent);
}

//============================================================================
//! Initializes this item.
/*!
    Emits the pleaseCreateCell() signal for all its columns.
    If a cell was not succesfully created the standard CCell class is used to
    create a cell object.
 */
void CMultiTypeItem::init(QListView* parent)
{
    CSpreadSheetView* sheet = dynamic_cast<CSpreadSheetView*>(parent);
    m_cells.resize(parent->columns());
    for (int i=0; i<parent->columns(); ++i) {
        m_cells[i] = 0;
        if (sheet != 0)
            sheet->emitPleaseCreateCell(this, i);
        if (m_cells[i] == 0)
            m_cells[i] = new CCell(this);
    }
    if (parent != 0)
        parent->updateGeometry();
}

//============================================================================
//! Destructor
/*!
    Frees the memory occupied by the cells.
 */
CMultiTypeItem::~CMultiTypeItem()
{
    for (unsigned int i=0; i<m_cells.size(); ++i) {
        if (m_cells[i] != 0) {
            delete m_cells[i];
            m_cells[i] = 0;
        }
    }
}

//============================================================================
//! Adds a column to this row.
void CMultiTypeItem::addColumn()
{
    CSpreadSheetView* sheet = dynamic_cast<CSpreadSheetView*>(listView());

    int newCol = listView()->columns()-1;
    m_cells.resize(listView()->columns());
    m_cells[newCol] = 0;

    if (sheet != 0)
        sheet->emitPleaseCreateCell(this, newCol);

    if (m_cells[newCol] == 0)
        m_cells[newCol] = new CCell(this);

    connect(m_cells[newCol], SIGNAL(valueChanged(int)), sheet, SLOT(onValueChanged(int)));
}

//============================================================================
//! Removes a column from this row.
/*!
    Memory allocated for the cells is freed.

    \param index The column to remove.
 */
void CMultiTypeItem::deleteColumn(int index)
{
    if (index>=0 && (unsigned)index < m_cells.size()) {
        if (m_cells[index] != 0)
            delete m_cells[index];
        vector<CCell*>::iterator pos = m_cells.begin() + index;
        m_cells.erase(pos);
    }
}

//============================================================================
//! Returns the column index of the \a cell argument.
/*!
    \param cell The cell to find.
    \return The index if the cell was found, or -1 if the cell could not be
    found.
 */
int CMultiTypeItem::findColumn(CCell* cell)
{
    for (unsigned int i=0; i<m_cells.size(); ++i)
        if (m_cells[i] == cell)
            return i;
    return -1;
}

//============================================================================
//! Draws a cell.
/*!
    Calls the paintCell() method of the cell in \a column.
 */
void CMultiTypeItem::paintCell(QPainter *pPainter, const QColorGroup& cg, int column, int width, int align)
{
    m_cells[column]->paintCell(pPainter, cg, column, width, align);
}

//============================================================================
//! Paints the focus rectangle
void CMultiTypeItem::paintFocus(QPainter* pPainter, const QColorGroup& cg, const QRect& r)
{
    CSpreadSheetView* sheet = dynamic_cast<CSpreadSheetView*>(listView());

    if (sheet == 0) {
        QListViewItem::paintFocus(pPainter, cg, r);
        return;
    }

    int left = 0;
    for (int i=0; i<sheet->currentFocusCol(); ++i)
        left += sheet->columnWidth(i);

    left -= listView()->contentsX();

    QRect n = r;
    n.setLeft(left);
    n.setRight(left+sheet->columnWidth(sheet->currentFocusCol())-1);
    n.setHeight(n.height()-1);

    pPainter->drawWinFocusRect(n);
}

//============================================================================
//! Retrieves the cell at column \a column.
/*!
    \return The cell at column \a column.
 */
CCell* CMultiTypeItem::getCell(int column)
{
    if (column>=0 && (unsigned)column<m_cells.size())
        return m_cells[column];
    return 0;
}

//============================================================================
//! Sets the cell at \a column.
/*!
    Changes the cell at \a column to the new cell (\a pCell).
    The old cell is deleted.

    \param column Column of the cell to replace.
    \param pCell  The new CCell object.
 */
void CMultiTypeItem::setCell(int column, CCell* pCell)
{
    ASSERT(pCell != 0);

    if (column>=0 && (unsigned)column<m_cells.size()) {
        if (m_cells[column] != 0 && m_cells[column] != pCell)
            delete m_cells[column];
        m_cells[column] = pCell;

        CSpreadSheetView* sheet = dynamic_cast<CSpreadSheetView*>(listView());
if (sheet)
	connect(m_cells[column], SIGNAL(valueChanged(int)), sheet, SLOT(onValueChanged(int)));
    }
}

//============================================================================
//! Starts a cell's edit mode.
/*!
    \param column The cell's column number.

    Edit mode is started for the cell at column \a column.

    \return A pointer to the cell is returned. It is used by the spreadsheet
            to keep track of edit mode.
 */
CCell* CMultiTypeItem::startEdit(int column)
{
    debug("Starting edit on column %d", column);
    if (column>=0 && (unsigned)column<m_cells.size()) {
        debug("   Calling m_cells[column]->startEdit()");
        return m_cells[column]->startEdit();
    }
    return 0;
}
