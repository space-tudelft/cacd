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

using namespace std;

//============================================================================
//! An indicator that shows the current selected process.
/*! \file
    The process indicator shows wich process is currently being edited.
    The Qt signal/slot mechanism is used to track any changes in the current
    process.

    This file implements the class.

    \author Xander Burgerhout
    \par Remarks
    Please refer to the header file (StatusIndicator.h) for information
    on changes and updates.
*/
//============================================================================

// Project includes
#include "src/spock/src/widgets/StatusIndicator/StatusIndicator.h"

//============================================================================
/*! Constructor.
    The constructor sets the indicators text to the default and the framestyle
    to something that looks a bit better.

    \param  pParent     The widget this indicator belongs to.
    \param  name        Optional (Qt) internal name for this widget.
*/
CStatusIndicator::CStatusIndicator(QWidget* pParent, const char* name /* = 0*/)
    : QLabel(pParent, name)
{
    setText("No process");
    setFrameStyle(QFrame::Panel | QFrame::Sunken);
}

//============================================================================
/*! This slot changes the text of the indicator.

    \param  newText      a QString with the new indicator text.
*/
void CStatusIndicator::onChangeText(QString newText)
{
    if (newText.latin1() != 0)
        setText(newText);
    else
        setText("");
}

//============================================================================
/*! This slot changes the text of the indicator.

    \param  newText      a const char* with the new indicator text.
*/
void CStatusIndicator::onChangeText(const char* newText)
{
    QString tmp = newText;
    onChangeText(tmp);
}
