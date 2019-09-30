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
#include "src/spock/src/gui/dialogs/ProcessDescDlg/ProcessDescDlg.h"
#include "src/spock/src/gui/validators/IdentValidator.h"

// Qt includes
#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qwidget.h>
#include <qvalidator.h>

using namespace std;

//============================================================================
//! Constructor
/*!
    \param The widgets parent.
    \param The internallu used QObject name for this instance.
 */
CProcessDescDlg::CProcessDescDlg(QWidget* parent, const char* name)
    : QDialog(parent, name, true)
{
    m_layout = new QVBoxLayout(this, 8, 8);
    m_layout->setAutoAdd(true);

    new QLabel("Please enter an identifier for this process:", this);
    m_processName = new QLineEdit(this);
//    QIntValidator* val = new QIntValidator(m_processName);
//    m_processName->setValidator(val);
    CIdentValidator* val = new CIdentValidator(m_processName);
    m_processName->setValidator(val);

    new QLabel("Please enter a short description for this process:", this);
    m_processDesc = new QLineEdit(this);

    QWidget* buttons = new QWidget(this);
    QHBoxLayout* buttLay = new QHBoxLayout(buttons);
    buttLay->setAutoAdd(true);

    buttLay->addStretch(1);
    QPushButton* okBut = new QPushButton("OK", buttons);
    okBut->setAutoDefault(true);
    QPushButton* cancelBut = new QPushButton("Cancel", buttons);
    buttLay->addStretch(1);

    connect(okBut, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancelBut, SIGNAL(clicked()), this, SLOT(reject()));
}

//============================================================================
//! Sets the process name field
void CProcessDescDlg::setProcessName(const string& name)
{
    m_processName->setText(name.c_str());
}

//============================================================================
//! Sets the process description field.
void CProcessDescDlg::setProcessDesc(const string& desc)
{
    m_processDesc->setText(desc.c_str());
}

//============================================================================
//! Returns the process name
/*!
    \return The process name
 */
string CProcessDescDlg::getProcessName()
{
    if (!m_processName->text().isNull())
        return m_processName->text().latin1();
    return "";
}

//============================================================================
//! Returns the process description
/*!
    \return The process description
 */
string CProcessDescDlg::getProcessDesc()
{
    if (!m_processDesc->text().isNull())
        return m_processDesc->text().latin1();
    return "";
}
