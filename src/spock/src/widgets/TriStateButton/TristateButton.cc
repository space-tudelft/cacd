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
#include "src/spock/src/widgets/TriStateButton/TristateButton.h"

// Qt includes
#include <qstyle.h>
#include <qpainter.h>

using namespace std;

//============================================================================
//! Constructor
/*!
    \param parent   The parent widget of this button
    \param name     The internal QObject name of this widget

    The constructor sets some defaults, such as the button size and type.
 */
CTristateButton::CTristateButton(QWidget* parent, const char* name)
    : QButton(parent, name)
{
    setMinimumHeight(15);
    setFixedWidth(15);
    setToggleButton(true);
    setToggleType(Tristate);
    setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
}

//============================================================================
//! Constructor
/*!
    \param state    The initial state of the button
    \param parent   The parent widget of this button
    \param name     The internal QObject name of this widget

    The constructor sets some defaults, such as the button size and type. The
    button is set to the state specified by \a state.
 */
CTristateButton::CTristateButton(ToggleState state, QWidget* parent, const char* name)
    : QButton(parent, name)
{
    setMinimumHeight(15);
    setMinimumWidth(15);
    setToggleButton(true);
    setToggleType(Tristate);
    setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    setState(state);
}

//============================================================================
//! Reimplemented
/*!
    This method has been reimplemented and does nothing. It also does not call
    the base class method, so it really does nothing at all.

    The looks of the button are controlled by the class itself and are not open
    to change.
 */
void CTristateButton::setPixmap(const QPixmap&)
{
    // Does nothing: changing the pixmap is controlled
    // by this class
}

//============================================================================
//! Reimplemented
/*!
    This method has been reimplemented and does nothing. It also does not call
    the base class method, so it really does nothing at all.

    The looks of the button are controlled by the class itself and are not open
    to change.
 */
void CTristateButton::setText(const QString&)
{
    // Does nothing: changing the text is controlled
    // by this class.
}

//============================================================================
//! Sets the new state to \a newState
/*!
    After the state has been set the button is repainted to update the changes.

 */
void CTristateButton::changeState(QButton::ToggleState newState)
{
    setState(newState);
    repaint();
}

//============================================================================
//! Draws the button
/*!
    Depending on the state of the button, it is colored white, red or green.
 */
void CTristateButton::drawButton(QPainter* painter)
{
    int x = rect().x();
    int y = rect().y();
    int w = rect().width();
    int h = rect().height();
    QStyle::SFlags sflags = QStyle::Style_Raised;
    if(isDown()) sflags = QStyle::Style_Down;
    style().drawControl(QStyle::CE_PushButton, painter, this, QRect(x, y, w, h), colorGroup(), sflags);
    QRect butRect = style().subRect(QStyle::SR_PushButtonContents, this);

    QColor color("white");
    if (state() == Off)
        color.setNamedColor("red");
    if (state() == On)
        color.setNamedColor("green");

    painter->fillRect(butRect, QBrush(color));
}

//============================================================================
//! Reimplemented
/*!
    This method has been reimplemented and does nothing. It also does not call
    the base class method, so it really does nothing at all.

    The looks of the button are controlled by the class itself and are not open
    to change.
 */
void CTristateButton::drawButtonLabel(QPainter*)
{
}
