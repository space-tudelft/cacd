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
#include "src/spock/src/widgets/SectionWidget/SectionWidget.h"

// Qt includes
#include <qlabel.h>
#include <qmultilineedit.h>

using namespace std;

//============================================================================
//! Constructor
/*!
    The constructor set the frame's size policy.

    \param parent   This widgets parent.
    \param name     The internal QObject name
    \param g        Needed by Qt, leave at defaults.
 */
CScrollFrame::CScrollFrame(QWidget* parent, const char* name, WFlags g)
    : QFrame(parent, name, g)
{
    QSizePolicy policy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    setSizePolicy(policy);
}

//============================================================================
//! Reimplemented.
/*!
    Reimplemented to emit contentsSizeChanged() signals.
 */
void CScrollFrame::resizeEvent(QResizeEvent* e)
{
    QFrame::resizeEvent(e);

    emit contentsSizeChanged();
}

//============================================================================
//! Constructor.
/*!
    The constructor creates the expand/collapse button and other performes
    other internal widget setup.

    \param label    The bar's text
    \param pParent  This widget's parent.
    \param name     The internal QObject name
 */
CSectionBar::CSectionBar(const char* label, QWidget* pParent, const char* name /* = 0 */)
: QFrame(pParent, name)
{
    QString buttTxt = "+";
    m_layout = new QHBoxLayout(this, 6, 4);
    m_pixmap = 0;

    m_toggleButton = new QPushButton(buttTxt, this);
    m_toggleButton->setToggleButton(TRUE);
    m_toggleButton->setMaximumSize(18,18);
    m_layout->addWidget(m_toggleButton, 0);

    m_pixmapLabel = new QLabel(this);
    m_layout->addWidget(m_pixmapLabel);

    m_sectionLabel = new QLabel(label, this);
    m_layout->addWidget(m_sectionLabel, 1);

    connect(m_toggleButton, SIGNAL(clicked()), this, SIGNAL(toggleButtonPressed()));
    connect(m_toggleButton, SIGNAL(clicked()), this, SLOT(onToggleButtonPressed()));

    //    setMidLineWidth(2);
    setFrameStyle(QFrame::Box | QFrame::Raised);
    setPalette( QPalette( QColor(20, 150, 200) ) );

    QSizePolicy policy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setSizePolicy(policy);
}

//============================================================================
//! Sets the pixmap shown on the sectionbar.
/*!
    \param fileName     Name and location of the pixmap file.

    If loading the pixmap fails, nothing happens and the sectionbar will remain
    as it was. If the pixmap could be loaded the pixmap is shown between the
    button and the title.
 */
void CSectionBar::setPixmap(const char* fileName)
{
    if (fileName == 0)
        return;

    if (m_pixmap == 0)
        m_pixmap = new QPixmap(fileName);
    else
        m_pixmap->load(fileName);

    if (!m_pixmap->isNull())
        m_pixmapLabel->setPixmap(*m_pixmap);
    else
        warning("Failed to load/set pixmap `%s'", fileName);

    updateGeometry();
}

//============================================================================
//! Called when the toggle button is pressed.
/*!
    The buttons text and state is updated.
 */
void CSectionBar::onToggleButtonPressed()
{
    if (m_toggleButton->isOn())
        m_toggleButton->setText("-");
    else
        m_toggleButton->setText("+");
}

//============================================================================
//! Constructor.
/*!
    \param pParent  This widgets parent widget.
    \param name     The intenal QObject name you want this widget to have.

    Some initialization is performed.
 */
CSectionWidget::CSectionWidget(QWidget* pParent, const char* name /* = 0 */)
: QFrame(pParent, name)
{
    m_layout = new QVBoxLayout(this);

    m_sbar = new CSectionBar("", this);
    m_layout->addWidget(m_sbar, 0);

    m_content = 0;

    setFrameStyle(QFrame::Panel | QFrame::Plain);

    connect(m_sbar, SIGNAL(toggleButtonPressed()), this, SLOT(onToggleButtonPressed()));
}

//============================================================================
//! Called when the sectionbar's toggle button is pressed.
/*!
    The widgets work area is expanded or collapsed, depending on the buttons state.

    \sa CSectionBar
 */
void CSectionWidget::onToggleButtonPressed()
{
    if (m_content != 0) {
        setUpdatesEnabled(false);
        //int oldWidth = width();
        if (m_content->isVisible()) {
            //debug("   Hiding...");
            m_content->hide();
        } else {
            //debug("   Showing...");
            m_content->show();
        }

        setUpdatesEnabled(true);
    }
//    updateGeometry();
}

//============================================================================
//! Sets the contents of the widgets work area.
/*!
    \param pWidget The widget to use as the new work area.

    If a content widget was already present, it is deleted.
    The new widget is reparented and becomes an integral part of this widget.
 */
void CSectionWidget::setContentWidget(QWidget* pWidget)
{
    if (m_content != 0) {
        delete m_content;
        m_content = 0;
    }

    QPoint p(0,0);
    pWidget->reparent(this, QPoint(0,0)); //, FALSE);

    pWidget->hide();
    m_content = pWidget;
    m_layout->addWidget(pWidget);
}

//============================================================================
//! Constructor
CMultiSectionWidget::CMultiSectionWidget(QWidget* pParent, const char* name /* = 0 */)
    : QScrollView(pParent, name)
{
    setVScrollBarMode(QScrollView::Auto);
    setHScrollBarMode(QScrollView::Auto);

    m_frame = new CScrollFrame(viewport());
    m_layout = new QVBoxLayout(m_frame);

    connect(m_frame, SIGNAL(contentsSizeChanged()), this, SLOT(onContentsSizeChanged()));

    QSizePolicy policyScroller(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setSizePolicy(policyScroller);

    addChild(m_frame);
}

//============================================================================
//! Adds a widget
/*!
    \param pWidget      The widget to add.

    The widget added is reparented. Widgets are layed out vertically.
 */
void CMultiSectionWidget::addWidget(QWidget* pWidget)
{
//    ASSERT(pWidget != 0);
    if (pWidget != 0) {
        pWidget->reparent(m_frame, QPoint(0,0));
        m_layout->addWidget(pWidget);
    }
}

//============================================================================
//! Reimplemented.
/*!
    Called when the contents change size.
    The contents are updated with the current visible width and height.
 */
void CMultiSectionWidget::onContentsSizeChanged()
{
    updateContents(0, 0, visibleWidth(), visibleHeight());
}

/*
void CMultiSectionWidget::resizeEvent(QResizeEvent* pEvent)
{
    QScrollView::resizeEvent(pEvent);
    debug("A paintEvent should occur now.");
}
*/

//============================================================================
//! Reimplemented.
/*!
    Fixes the frame width.
 */
void CMultiSectionWidget::paintEvent(QPaintEvent* pEvent)
{
    QScrollView::paintEvent(pEvent);
    //debug("A paintEvent!!.");
    fixFrameWidth();
}

//============================================================================
//! Updates the size of the frame containing the widgets.
/*!
    The widget attempts to keep the contents the same width as the widget so
    no horizontal scroll bar is displayed and no ugly empty space is displayed.

    Vertically, nothing is done.
 */
void CMultiSectionWidget::fixFrameWidth()
{
    setUpdatesEnabled(false);

    QSize hint = m_frame->sizeHint();
    QSize viewSize = viewportSize(hint.width(), hint.height());

//    debug("Hint wants to be: %dx%d", hint.width(), hint.height());
//    debug("Frame is now    : %dx%d", m_frame->width(), m_frame->height());
//    debug("Viewport is now : %dx%d", viewSize.width(), viewSize.height());

    if (viewSize.width() > hint.width()) {
//        debug(">>> resizing from CMultiSectionWidget::doFixFrameWidth()");
//        debug("    new size to be    %3dx%3d", viewSize.width(), m_frame->height());
        m_frame->setFixedWidth(viewSize.width());
        m_frame->resize(viewSize.width(), m_frame->height());
//        debug("    new size reported %3dx%3d", m_frame->width(), m_frame->height());
//        debug("<<< done");
    }
    setUpdatesEnabled(true);
}
