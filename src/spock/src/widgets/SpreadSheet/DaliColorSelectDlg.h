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
#ifndef __DALICOLORSELECT_H__
#define __DALICOLORSELECT_H__

// Project includes
#include "src/spock/src/helper/DaliStyles/DaliStyles.h"

// STL includes
#include <vector>

// Qt includes
#include <qpushbutton.h>
#include <qdialog.h>
#include <qlayout.h>

//! Pushbutton with a dalistyle as its label.
/*!
    The CDaliPushButton is used in the CDaliColorDlg. The pushbutton is derived
    from the QPushButton class and has a pixmap with a dali style as its label.

    \sa CDaliColorDlg, CDaliStyle

    \author Xander Burgerhout
*/
class CDaliPushButton : public QPushButton
{
    Q_OBJECT

    private:
	CDaliPixmap*        m_pm;

    public:
	CDaliPushButton(CDaliStyle& style, QWidget* parent, const char* name = 0);
       ~CDaliPushButton();

    private slots:
	void            onClicked();

    signals:
	void            clicked(const CDaliStyle&);
};

//! A dali style selection dialog box.
/*!
    This dialog box can be used to select a dali style.

    \image html dalidlg.jpg
    \image latex dalidlg.eps "Dalistyle selection" width=10cm

    \author Xander Burgerhout
*/
class CDaliColorDlg : public QDialog
{
    Q_OBJECT

    private:
	QGridLayout*                m_layout;
	CDaliStyle                  m_style;

    public:
	CDaliColorDlg(QWidget* pParent, const char* name = 0);

	CDaliStyle                  getStyle();
    // signals & slots
    public slots:
	void            onClicked(const CDaliStyle& style);
	void            onCancel();
};

#endif // __DALICOLORSELECT_H__
