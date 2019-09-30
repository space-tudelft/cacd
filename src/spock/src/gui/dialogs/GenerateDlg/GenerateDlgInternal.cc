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
** Form implementation generated from reading ui file 'GenerateDlg.ui'
**
** Created: Mon Dec 11 13:02:08 2000
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "src/spock/src/gui/dialogs/GenerateDlg/GenerateDlgInternal.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

using namespace std;

/*
 *  Constructs a GenerateDlgInternal which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
GenerateDlgInternal::GenerateDlgInternal( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "GenerateDlgInternal" );
    resize( 576, 265 );
    setCaption( tr( "Generate..." ) );
    setSizeGripEnabled( TRUE );
    vbox = new QVBoxLayout( this );
    vbox->setSpacing( 6 );
    vbox->setMargin( 11 );

    m_groupFiles = new QGroupBox( this, "m_groupFiles" );
    m_groupFiles->setTitle( tr( "What files would you like to generate?" ) );
    m_groupFiles->setFrameShadow( QGroupBox::Sunken );
    m_groupFiles->setFrameShape( QGroupBox::Box );
    m_groupFiles->setColumnLayout(0, Qt::Vertical );
    m_groupFiles->layout()->setSpacing( 0 );
    m_groupFiles->layout()->setMargin( 0 );
    vbox_2 = new QVBoxLayout( m_groupFiles->layout() );
    vbox_2->setAlignment( Qt::AlignTop );
    vbox_2->setSpacing( 6 );
    vbox_2->setMargin( 11 );

    m_frameFiles = new QFrame( m_groupFiles, "m_frameFiles" );
    m_frameFiles->setFrameShadow( QFrame::Raised );
    m_frameFiles->setFrameShape( QFrame::NoFrame );
    vbox_2->addWidget( m_frameFiles );
    vbox->addWidget( m_groupFiles );

    m_groupTarget = new QGroupBox( this, "m_groupTarget" );
    m_groupTarget->setTitle( tr( "Where would you like to generate to?" ) );
    m_groupTarget->setColumnLayout(0, Qt::Vertical );
    m_groupTarget->layout()->setSpacing( 0 );
    m_groupTarget->layout()->setMargin( 0 );
    vbox_3 = new QVBoxLayout( m_groupTarget->layout() );
    vbox_3->setAlignment( Qt::AlignTop );
    vbox_3->setSpacing( 6 );
    vbox_3->setMargin( 11 );

    m_radioGenerateSpace = new QRadioButton( m_groupTarget, "m_radioGenerateSpace" );
    m_radioGenerateSpace->setText( tr( "Generate into the SPACE process tree"  ) );
    vbox_3->addWidget( m_radioGenerateSpace );

    hbox = new QHBoxLayout;
    hbox->setSpacing( 6 );
    hbox->setMargin( 0 );

    m_labelProcessNr = new QLabel( m_groupTarget, "m_labelProcessNr" );
    m_labelProcessNr->setText( tr( "     Process number:"  ) );
    m_labelProcessNr->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)1 ) );
    hbox->addWidget( m_labelProcessNr );

    m_comboProcessNr = new QComboBox( FALSE, m_groupTarget, "m_comboProcessNr" );
    m_comboProcessNr->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0 ) );
    hbox->addWidget( m_comboProcessNr );

    m_buttonNewProcessNr = new QPushButton( m_groupTarget, "m_buttonNewProcessNr" );
    m_buttonNewProcessNr->setText( tr( "New..."  ) );
    hbox->addWidget( m_buttonNewProcessNr );
    vbox_3->addLayout( hbox );

    m_radioGenerateDir = new QRadioButton( m_groupTarget, "m_radioGenerateDir" );
    m_radioGenerateDir->setText( tr( "Generate into a directory:"  ) );
    vbox_3->addWidget( m_radioGenerateDir );

    hbox_2 = new QHBoxLayout;
    hbox_2->setSpacing( 6 );
    hbox_2->setMargin( 0 );

    m_labelDir = new QLabel( m_groupTarget, "m_labelDir" );
    m_labelDir->setText( tr( "     Directory:"  ) );
    hbox_2->addWidget( m_labelDir );

    m_editDirectory = new QLineEdit( m_groupTarget, "m_editDirectory" );
    hbox_2->addWidget( m_editDirectory );

    m_buttonDirBrowse = new QPushButton( m_groupTarget, "m_buttonDirBrowse" );
    m_buttonDirBrowse->setText( tr( "Browse..."  ) );
    hbox_2->addWidget( m_buttonDirBrowse );
    vbox_3->addLayout( hbox_2 );
    vbox->addWidget( m_groupTarget );
    QSpacerItem* spacer = new QSpacerItem( 20, 1, QSizePolicy::Fixed, QSizePolicy::Expanding );
    vbox->addItem( spacer );

    hbox_3 = new QHBoxLayout;
    hbox_3->setSpacing( 6 );
    hbox_3->setMargin( 0 );

    m_checkExamine = new QCheckBox( this, "m_checkExamine" );
    m_checkExamine->setText( tr( "Examine and  edit results before saving" ) );
    hbox_3->addWidget( m_checkExamine );
    QSpacerItem* spacer_2 = new QSpacerItem( 78, 20, QSizePolicy::Expanding, QSizePolicy::Fixed );
    hbox_3->addItem( spacer_2 );

    m_buttonGenerate = new QPushButton( this, "m_buttonGenerate" );
    m_buttonGenerate->setText( tr( "Generate!" ) );
    m_buttonGenerate->setAutoDefault( TRUE );
    m_buttonGenerate->setDefault( TRUE );
    hbox_3->addWidget( m_buttonGenerate );

    m_buttonCancel = new QPushButton( this, "m_buttonCancel" );
    m_buttonCancel->setText( tr( "Cancel" ) );
    m_buttonCancel->setAutoDefault( TRUE );
    hbox_3->addWidget( m_buttonCancel );
    vbox->addLayout( hbox_3 );

    // signals and slots connections
    connect( m_buttonGenerate, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( m_buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( m_buttonDirBrowse, SIGNAL( clicked() ), this, SLOT( onBrowseDirectory() ) );
    connect( m_radioGenerateSpace, SIGNAL( clicked() ), this, SLOT( onClickedRadioProcessNr() ) );
    connect( m_radioGenerateDir, SIGNAL( clicked() ), this, SLOT( onClickedRadioDir() ) );
    connect( m_buttonNewProcessNr, SIGNAL( clicked() ), this, SLOT( onNewProcessNr() ) );

    // tab order
    setTabOrder( m_radioGenerateSpace, m_radioGenerateDir );
    setTabOrder( m_radioGenerateDir, m_buttonGenerate );
    setTabOrder( m_buttonGenerate, m_buttonCancel );
    setTabOrder( m_buttonCancel, m_editDirectory );
    setTabOrder( m_editDirectory, m_buttonDirBrowse );
}

/*
 *  Destroys the object and frees any allocated resources
 */
GenerateDlgInternal::~GenerateDlgInternal()
{
    // no need to delete child widgets, Qt does it all for us
}

void GenerateDlgInternal::onBrowseDirectory()
{
    qWarning( "GenerateDlgInternal::onBrowseDirectory(): Not implemented yet!" );
}

void GenerateDlgInternal::onClickedRadioDir()
{
    qWarning( "GenerateDlgInternal::onClickedRadioDir(): Not implemented yet!" );
}

void GenerateDlgInternal::onClickedRadioProcessNr()
{
    qWarning( "GenerateDlgInternal::onClickedRadioProcessNr(): Not implemented yet!" );
}

void GenerateDlgInternal::onNewProcessNr()
{
    qWarning( "GenerateDlgInternal::onNewProcessNr(): Not implemented yet!" );
}
