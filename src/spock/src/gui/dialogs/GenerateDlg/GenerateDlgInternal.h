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

/****************************************************************************
** Form interface generated from reading ui file 'GenerateDlg.ui'
**
** Created: Mon Dec 11 13:02:08 2000
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef GENERATEDLGINTERNAL_H
#define GENERATEDLGINTERNAL_H

#include <qdialog.h>
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QCheckBox;
class QComboBox;
class QFrame;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;

class GenerateDlgInternal : public QDialog
{
    Q_OBJECT

public:
    GenerateDlgInternal( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~GenerateDlgInternal();

    QGroupBox* m_groupFiles;
    QFrame* m_frameFiles;
    QGroupBox* m_groupTarget;
    QRadioButton* m_radioGenerateSpace;
    QLabel* m_labelProcessNr;
    QComboBox* m_comboProcessNr;
    QPushButton* m_buttonNewProcessNr;
    QRadioButton* m_radioGenerateDir;
    QLabel* m_labelDir;
    QLineEdit* m_editDirectory;
    QPushButton* m_buttonDirBrowse;
    QCheckBox* m_checkExamine;
    QPushButton* m_buttonGenerate;
    QPushButton* m_buttonCancel;

public slots:
    virtual void onBrowseDirectory();
    virtual void onClickedRadioDir();
    virtual void onClickedRadioProcessNr();
    virtual void onNewProcessNr();

protected:
    QHBoxLayout* hbox;
    QHBoxLayout* hbox_2;
    QHBoxLayout* hbox_3;
    QVBoxLayout* vbox;
    QVBoxLayout* vbox_2;
    QVBoxLayout* vbox_3;
};

#endif // GENERATEDLGINTERNAL_H
