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
#ifndef __SECTIONWIDGET_H__
#define __SECTIONWIDGET_H__

// Qt includes
#include <qwidget.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qscrollview.h>

// STL includes
#include <vector>

//! A small QFrame derived widget that emits a special signal when resized.
/*!
    Should be renamed to something like CScrollFrameInternal to allow renaming
    of CMultiSectionWidget to something more appropriate like CScrollFrame.

    \sa CSectionBar, CSectionWidget, CMultiSectionWidget

    \author Xander Burgerhout
*/
class CScrollFrame : public QFrame
{
    Q_OBJECT

    public:
	CScrollFrame(QWidget *parent=0, const char *name=0, WFlags g=0);
//        ~CFrameLayout();
	virtual void    resizeEvent(QResizeEvent* pEvent);

    signals:
	void            contentsSizeChanged();
};

//! The title bar used in the CSectionWidget.
/*!
    The bar has a button, a pixmap and a label. They can be set
    using the setLabel() or setPixmap() method.

    \sa CSectionWidget, CMultiSectionWidget

    \author Xander Burgerhout
*/
class CSectionBar : public QFrame
{
    Q_OBJECT

    private:
	QBoxLayout*     m_layout;
	QPushButton*    m_toggleButton;
	QLabel*         m_sectionLabel;
	QLabel*         m_pixmapLabel;
	QPixmap*        m_pixmap;

    private slots:
	void            onToggleButtonPressed();

    public:
	CSectionBar(const char* label, QWidget* pParent, const char* name = 0);

	void            setLabel(const char* newLabel) { m_sectionLabel->setText(newLabel);}
	void            setPixmap(const char* fileName);

    signals:
	void            toggleButtonPressed();
};

//! A widget that has a title bar and a work area that can expanded and collapsed.
/*!
    The title bar is a CSectionBar. The work area just a QFrame. The contents of
    the work area can be set with the setContentWidget() method.

    \image html sections.jpg
    \image latex sections.eps "Some collapsed section widgets" width=7cm

    \sa CSectionBar, CMultiSectionWidget

    \author Xander Burgerhout
*/
class CSectionWidget : public QFrame
{
    Q_OBJECT

    private:
	QBoxLayout*     m_layout;
	CSectionBar*    m_sbar;
	QWidget*        m_content;

    private slots:
	void            onToggleButtonPressed();

    public:
	CSectionWidget(QWidget* pParent, const char* name = 0);

	void            setContentWidget(QWidget* pWidget);
	void            setLabel(const char* newLabel) { m_sbar->setLabel(newLabel);}
	void            setPixmap(const char* fileName) { m_sbar->setPixmap(fileName);}
};

//! A vertically scrollable frame.
/*!
    If possible, the width of widgets inside this QScrollView derived class
    are matched to the width of this widget. If a fit is not possible,
    a horizontal scrollbar is shown.

    Vertically, scroll bars are added and removed when necessary.

    This class should be renamed in a future version.

    \sa CSectionBar, CSectionWidget.

    \author Xander Burgerhout
*/
class CMultiSectionWidget : public QScrollView
{
    Q_OBJECT

    private:
	CScrollFrame*   m_frame;
	QBoxLayout*     m_layout;

	void            fixFrameWidth();

    public:
	CMultiSectionWidget(QWidget* pParent, const char* name = 0);

	void            addWidget(QWidget* pWdgt);

    protected:
//        void            resizeEvent(QResizeEvent* e);
	void            paintEvent(QPaintEvent* e);

    private slots:
	void            onContentsSizeChanged();
};

#endif // __SECTIONWIDGET_H__
