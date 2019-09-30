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
#include "src/spock/src/gui/dialogs/AboutDlg/AboutDlg.h"

// Qt includes
#include <qlayout.h>
#include <qtextview.h>
#include <qpushbutton.h>
#include <qwidget.h>

using namespace std;

//============================================================================
//! Constructor
/*!
    Constructs the dialog. Uses a QTextView to show some information about the application.
    Obsolete. (SdeG)
 */
CAboutDlg::CAboutDlg(QWidget* parent, const char* name)
    : QDialog(parent, name, true)
{
    m_layout = new QVBoxLayout(this, 8, 8);
    m_layout->setAutoAdd(true);

    QTextView* label = new QTextView(
	"<qt><center><font face=\"helvetica\"><h3>SPOCK</h3></font><br/>"
	"Version 1.0"
	"<hr/>Technology file generator for SPACE<br/>"
	"Visit the SPACE homepage at http://www.space.tudelft.nl<hr/>"
	"Programming and design by Xander Burgerhout<br/>"
	"Patient coaching and guiding by N.P. van der Meijs<hr/>"
	"This program uses the Qt toolkit"
	"</center></qt>",
	QString::null, this);

    QPushButton* okBut = new QPushButton("OK", this);
    okBut->setAutoDefault(true);
    connect(okBut, SIGNAL(clicked()), this, SLOT(accept()));
    resize(420, 300);
}
