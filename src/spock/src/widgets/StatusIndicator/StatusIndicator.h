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
#ifndef __STATUSINDICATOR_H__
#define __STATUSINDICATOR_H__

//============================================================================
//! A status bar text indicator.
/*! \file
    The indicator is added to the statusbar. Thet text of the indicator can be
    changed whenever necessary by connecting the appropriate signal to the
    ChangeText() slot.

    This file declares the class.

    \author Xander Burgerhout
    \date   May 15, 2000    First version.
    \date   June 19, 2000   Renamed file & class from ProcessIndicator to the
                            more general StatusIndicator. The name of the slot
                            to change the text was also renamed.
*/
//============================================================================

// Qt includes
#include <qlabel.h>

//! An indicator that shows the current selected process.
/*!
    This class provides a slot that handles process changes.
    Signals that indicate that the user switched processes can connect to this
    slot.

    \author Xander Burgerhout
*/
class CStatusIndicator : public QLabel
{
    Q_OBJECT

    public:
	CStatusIndicator(QWidget* pParent, const char* name = 0);

    public slots:
	void            onChangeText(QString newText);
	void            onChangeText(const char* newText);
};

#endif //__STATUSINDICATOR_H__
