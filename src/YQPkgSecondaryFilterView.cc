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


#include <QVBoxLayout>
#include <QSplitter>

#include "QY2ComboTabWidget.h"
#include "QY2LayoutUtils.h"
#include "YQPkgSearchFilterView.h"
#include "YQPkgStatusFilterView.h"
#include "YQi18n.h"

#include "Logger.h"
#include "Exception.h"

#include "YQPkgSecondaryFilterView.h"


YQPkgSecondaryFilterView::YQPkgSecondaryFilterView( QWidget * parent )
    : QWidget( parent )
{
}

void YQPkgSecondaryFilterView::init(QWidget * primary_widget)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    CHECK_NEW( layout );
    layout->setContentsMargins(0,0,0,0);

    QSplitter * splitter = new QSplitter( Qt::Vertical, this );
    CHECK_NEW( splitter );

    layout->addWidget( splitter );
    splitter->addWidget(primary_widget);

    primary_widget->setSizePolicy( QSizePolicy( QSizePolicy::Ignored, QSizePolicy::Expanding ) );// hor/vert

    // Directly propagate signals filterStart() and filterFinished()
    // from primary filter to the outside
    connect( primary_widget,	SIGNAL( filterStart() ),
	     this,		SIGNAL( filterStart() ) );

    connect( primary_widget,	SIGNAL( filterFinished() ),
	     this, 		SIGNAL( filterFinished() ) );

    // Redirect filterMatch() and filterNearMatch() signals to secondary filter
    connect( primary_widget,	SIGNAL( filterMatch		( ZyppSel, ZyppPkg ) ),
	     this,		SLOT  ( primaryFilterMatch	( ZyppSel, ZyppPkg ) ) );

    connect( primary_widget,	SIGNAL( filterNearMatch		( ZyppSel, ZyppPkg ) ),
	     this,		SLOT  ( primaryFilterNearMatch	( ZyppSel, ZyppPkg ) ) );

    layoutSecondaryFilters( splitter, primary_widget );

    splitter->setStretchFactor(0, 5);
    splitter->setStretchFactor(1, 1);
    splitter->setStretchFactor(2, 3);
}

YQPkgSecondaryFilterView::~YQPkgSecondaryFilterView()
{
    // NOP
}

QWidget *
YQPkgSecondaryFilterView::layoutSecondaryFilters( QWidget * parent, QWidget * primary_widget )
{
    QWidget *vbox = new QWidget( parent );
    CHECK_NEW( vbox );

    QVBoxLayout *layout = new QVBoxLayout();
    CHECK_NEW( layout );

    vbox->setLayout( layout );
    layout->setContentsMargins( 0, 0, 0, 0 );

    // Translators: This is a combo box where the user can apply a secondary filter
    // in addition to the primary filter by repository - one of
    // "All packages", "RPM groups", "search", "summary"
    //
    // And yes, the colon really belongs there since this is one of the very
    // few cases where a combo box label is left to the combo box rather than
    // above it.
    _secondaryFilters = new QY2ComboTabWidget( _( "&Secondary Filter:" ));
    CHECK_NEW( _secondaryFilters );
    layout->addWidget(_secondaryFilters);

    //
    // All Packages
    //
    _allPackages = new QWidget( this );
    CHECK_NEW( _allPackages );
    _secondaryFilters->addPage( _( "All Packages" ), _allPackages );


    // Unmaintained packages: Packages that are not provided in any of
    // the configured repositories
    _unmaintainedPackages = new QWidget( this );
    CHECK_NEW( _unmaintainedPackages );
    _secondaryFilters->addPage( _( "Unmaintained Packages" ), _unmaintainedPackages );

    //
    // Package search view
    //

    _searchFilterView = new YQPkgSearchFilterView( this );
    CHECK_NEW( _searchFilterView );
    _secondaryFilters->addPage( _( "Search" ), _searchFilterView );

    connect( _searchFilterView,	SIGNAL( filterStart() ),
	     primary_widget,	SLOT  ( filter()      ) );

    connect( _secondaryFilters, &QY2ComboTabWidget::currentChanged,
             this,              &YQPkgSecondaryFilterView::filter );

    //
    // Status change view
    //
    _statusFilterView = new YQPkgStatusFilterView( parent );
    CHECK_NEW( _statusFilterView );
    _secondaryFilters->addPage( _( "Installation Summary" ), _statusFilterView );

    connect( _statusFilterView,	SIGNAL( filterStart() ),
	     primary_widget,	    SLOT  ( filter() ) );

    return _secondaryFilters;
}

void YQPkgSecondaryFilterView::filter()
{
    primaryFilter();
}

void YQPkgSecondaryFilterView::filterIfVisible()
{
    primaryFilterIfVisible();
}

void YQPkgSecondaryFilterView::primaryFilterMatch( ZyppSel 	selectable,
						 ZyppPkg 	pkg )
{
    if ( secondaryFilterMatch( selectable, pkg ) )
	   emit filterMatch( selectable, pkg );
}

void YQPkgSecondaryFilterView::primaryFilterNearMatch( ZyppSel	selectable,
						     ZyppPkg 	pkg )
{
    if ( secondaryFilterMatch( selectable, pkg ) )
	   emit filterNearMatch( selectable, pkg );
}

bool
YQPkgSecondaryFilterView::secondaryFilterMatch( ZyppSel	selectable,
		      ZyppPkg 	pkg )
{
    if ( _allPackages->isVisible() )
    {
	    return true;
    }
    else if ( _unmaintainedPackages->isVisible() )
    {
        return ( selectable->availableSize() == 0 );
    }
    else if ( _searchFilterView->isVisible() )
    {
	    return _searchFilterView->check( selectable, pkg );
    }
    else if ( _statusFilterView->isVisible() )
    {
	    return _statusFilterView->check( selectable, pkg );
    }

	return true;
}

