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
#include "Workflow.h"

#ifndef VERBOSE_WORKFLOW
#  define VERBOSE_WORKFLOW      0
#endif


Workflow::Workflow( const QList<WorkflowStep *> & steps )
{
    _steps = steps; // This takes ownership of the steps!

    if ( _steps.isEmpty() )
    {
        logError() << "Empty workflow!" << endl;
        return;
    }

    foreach ( WorkflowStep * step, _steps )
    {
        CHECK_PTR( step );
        step->setWorkflow( this );
    }

    restart();
}


Workflow::~Workflow()
{
    qDeleteAll( _steps ); // Call 'delete' for each list item
}


WorkflowStep *
Workflow::step( int id ) const
{
    foreach ( WorkflowStep * step, _steps )
    {
        if ( step->id() == id )
            return step;
    }

    logWarning() << "No workflow step with ID " << id << endl;
    return 0;
}


void Workflow::next()
{
    if ( _currentStep && _currentStep->next() >= 0 ) // Step has a nonstandard 'next'
    {
        activate( step( _currentStep->next() ) );
    }
    else  // No nonstandard next -> use the next step in the steps list
    {
        int index = _steps.indexOf( _currentStep );      // -1 if not found

        if ( index >= 0 && index + 1 < _steps.size() )  // in range?
            activate( _steps.at( index + 1 ) );
        else
            logError() << "No workflow step after ID " << _currentStep->id()
                       << " (#" << index << ")"
                       << endl;
    }
}


void Workflow::back()
{
    if ( _history.isEmpty() )
    {
        logWarning() << "Workflow steps history is empty - can't go back any further" << endl;
        return;
    }

    activate( popHistory(),
              false ); // goingForward
}


void Workflow::gotoStep( int id )
{
    activate( step( id ) );
}


void Workflow::activate( WorkflowStep * step, bool goingForward )
{
    if ( step )
    {
#if VERBOSE_WORKFLOW
        logDebug() << "Going " << QString( goingForward ? "forward" : "backward" )
                   << " to step ID " << step->id()
                   << endl;
#endif

        if ( _currentStep )
            _currentStep->deactivate( goingForward );

        if ( goingForward && _currentStep )
        {
            pushHistory( _currentStep );
        }

        _currentStep = step;
        step->activate( goingForward );
    }
}


void Workflow::restart()
{
    _history.clear();
    _currentStep = _steps.isEmpty() ? 0 : _steps.first();

    if ( _currentStep )
        _currentStep->activate( true ); // goingForward
}


bool Workflow::atLastStep() const
{
    if ( _steps.isEmpty() )
        return true;

    CHECK_PTR( _currentStep );
    int index = _steps.indexOf( _currentStep );

    if ( index < 0 )
    {
        logError() << "Can't find current step in the steps list" << endl;
        return true;
    }

    return index == _steps.size() - 1;
}


void Workflow::pushHistory( WorkflowStep * step )
{
    if ( step )
    {
#if VERBOSE_WORKFLOW
        logDebug() << "Saving ID " << step->id() << " to history" << endl;
#endif
        _history << step;
    }
    else
        logError() << "Refusing to push null pointer to history" << endl;
}


WorkflowStep *
Workflow::popHistory()
{
    if ( _history.isEmpty() )
    {
        logError() << "History is empty" << endl;
        return 0;
    }

    WorkflowStep * step = _history.takeLast();
    CHECK_PTR( step );

#if VERBOSE_WORKFLOW
    logDebug() << "Taking ID " << step->id() << " from history" << endl;
#endif

    return step;
}
