/*  ------------------------------------------------------
              __   _____  ____  _
              \ \ / / _ \|  _ \| | ____ _
               \ V / | | | |_) | |/ / _` |
                | || |_| |  __/|   < (_| |
                |_| \__\_\_|   |_|\_\__, |
                                    |___/
    ------------------------------------------------------

    Project:  YQPkg Package Selector
    Copyright (c) 2024 SUSE LLC
    License:  GPL V2 - See file LICENSE for details.

    Textdomain "qt-pkg"
 */


#include "Logger.h"
#include "Exception.h"
#include "WizardPage.h"


//
// This can also serve a very simple example how to use Qt Designer .ui forms.
//


WizardPage::WizardPage( const QString & name, QWidget * parent )
    : QWidget( parent )
    , _ui( new Ui::WizardPage ) // Use the Qt designer .ui form (XML)
{
    CHECK_NEW( _ui );
    _ui->setupUi( this ); // Actually create the widgets from the .ui form

    // See ui_wizard-page.h for the widget names.
    //
    // That header is generated by Qt's uic (user interface compiler)
    // from the XML .ui file created with Qt designer.
    //
    // Take care in Qt designer to give each widget a meaningful name in the
    // widget tree at the top right: They are also the member variable names
    // for the _ui object.

    _ui->pageContentLabel->setText( name );

    QObject::connect( _ui->actionBack,  SIGNAL( triggered() ),
                      this,             SIGNAL( back()      ) );

    QObject::connect( _ui->actionNext,  SIGNAL( triggered() ),
                      this,             SIGNAL( next()      ) );

    // The buttons are already connected to the actions in the .ui form
    // with the "action editor" in Qt designer.
    // Check tue ui_xxx.h header to see the generated code.
}


WizardPage::~WizardPage()
{
    delete _ui;
}


void WizardPage::connect( QObject * receiver )
{
    // This works here only because we are blindly assuming that the receiver
    // has slots named back() and next(); because the SIGNAL and SLOT macros do
    // some fancy text subsitution that results in moc (Qt Meta Object
    // Compiler) files being used for introspection and looking up string
    // tables of those signals and slots.
    //
    // Observe the log file for complaints at runtime.
    // That's the downside: It is not checked by the compiler at build time.

    QObject::connect( this,     SIGNAL( back() ),
                      receiver, SLOT  ( back() ) );

    QObject::connect( this,     SIGNAL( next() ),
                      receiver, SLOT  ( next() ) );
}