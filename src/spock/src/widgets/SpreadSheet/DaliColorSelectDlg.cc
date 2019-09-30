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

// Qt includes
#include <qpushbutton.h>
#include <qlayout.h>
#include <qlabel.h>

// Project includes
#include "src/spock/src/widgets/SpreadSheet/DaliColorSelectDlg.h"

using namespace std;

//============================================================================
//! Default constructor
/*!
    Creates a pushbutton with a pixmap in the specified dalistyle as its label.

    \param style    The requested dalistyle.
    \param parent   The widgets parent
    \param name     The QObject internal name of the widget.
 */
CDaliPushButton::CDaliPushButton(CDaliStyle& style, QWidget* parent, const char* name)
    : QPushButton(parent, name)
{
    connect(this, SIGNAL(clicked()), this, SLOT(onClicked()));

    int color = style.GetColor();
    int fill =  style.GetFill();
    m_pm = new CDaliPixmap(32, 16, color, fill);

    setPixmap(*m_pm);
}

//============================================================================
//! Destructor
/*!
    Deletes the pixmap.
 */
CDaliPushButton::~CDaliPushButton()
{
    if (m_pm != 0)
        delete m_pm;
}

//============================================================================
//! Emits a clicked() signal with the dalistyle of the button.
/*!
    This slot is connected with its own clicked() signal. This adds another
    clicked() signal to the availabe signals. The emitted clicked signal can
    communicate the dalistyle of the button.
 */
void CDaliPushButton::onClicked()
{
    CDaliStyle style;
    style.SetFill(m_pm->GetFill());
    style.SetColor(m_pm->GetColor());

    emit clicked(style);
}

//============================================================================
//! Constructor
/*!
    Constructs the CDaliColorDlg dialog box. The buttons are created from the
    available dali patterns and colors.
 */
CDaliColorDlg::CDaliColorDlg(QWidget* pParent, const char* name /* = 0 */)
    : QDialog(pParent, name, true)
{
    setCaption ("Select dali color");

    m_layout = new QGridLayout(this, 0, 0, 11, 6);

    QLabel* label = new QLabel("Please select a style:", this);
    m_layout->addMultiCellWidget(label, 0, 0, 0, CDaliStyle::NumFills-1);

    CDaliColorTable* ct = CDaliColorTable::Instance();

    for (int col = 0; col<ct->NumColors(); ++col) {
        for (int fill = 0; fill<CDaliStyle::NumFills; ++fill) {
            CDaliStyle style(col, fill);
            CDaliPushButton* but = new CDaliPushButton(style, this);
            connect(but, SIGNAL(clicked(const CDaliStyle&)),
                    this, SLOT(onClicked(const CDaliStyle&)));
            m_layout->addWidget(but, col+1, fill);
        }
    }

    QPushButton* cancel = new QPushButton("Cancel", this);
    m_layout->addMultiCellWidget(cancel, ct->NumColors()+1, ct->NumColors()+1,
                                    CDaliStyle::NumFills-2, CDaliStyle::NumFills-1);
    connect(cancel, SIGNAL(clicked()), this, SLOT(onCancel()));
}

//============================================================================
//! Retrieves the selected style
/*!
    \return The selected style
 */
CDaliStyle CDaliColorDlg::getStyle()
{
    return m_style;
}

//============================================================================
//! Called when the user clicks a dalistyle button.
/*!
    The clicked() signal emitted by the dalistyle button communicates his
    dalistyle. That style is set as the selected style and the dialog box is
    closed.

    \sa onCancel()
 */
void CDaliColorDlg::onClicked(const CDaliStyle& style)
{
    m_style = style;
    accept();
}

//============================================================================
//! Cancels the operation.
/*!
    \sa onClicked()
*/
void CDaliColorDlg::onCancel()
{
    reject();
}
