/********************************************************************
 Copyright 2010 Lubos Lunak <l.lunak@kde.org>

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

#include "minicli.h"

#include <assert.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kauthorized.h>
#include <klocale.h>

#include <kdebug.h>

#include "miniclidialog.h"
#include "miniclihandler.h"

namespace Kor
{

static Minicli* minicli_ptr = NULL;

Minicli::Minicli( QObject* parent )
    : QObject( parent )
    {
    assert( minicli_ptr == NULL );
    minicli_ptr = this;
    dialog = new MinicliDialog( this );
    KActionCollection* actions = new KActionCollection( this );
    KAction* action = actions->addAction( "showruncommand" );
    action->setText( i18n( "Show run command dialog" ));
    action->setGlobalShortcut( KShortcut( Qt::ALT + Qt::Key_F5 ));
    connect( action, SIGNAL( triggered( bool )), this, SLOT( showDialog()));

    connect( &updateTimer, SIGNAL( timeout()), this, SLOT( doUpdate()));
    updateTimer.setSingleShot( true );

    // Command handlers, highest priority first. This could be made even
    // more flexible by introducing plugins, if needed.    
    if( KAuthorized::authorizeKAction( "run_command" )) // TODO this authorize action for specials?
        handlers.append( new MinicliHandlerSpecials );
    handlers.append( new MinicliHandlerCalculator );
    if( KAuthorized::authorizeKAction( "run_command" ))
        handlers.append( new MinicliHandlerCommandUrl );
    }

Minicli::~Minicli()
    {
    qDeleteAll( handlers );
    minicli_ptr = NULL;
    }

Minicli* Minicli::self()
    {
    assert( minicli_ptr != NULL );
    return minicli_ptr;
    }

void Minicli::showDialog()
    {
    dialog->activate();
    }

bool Minicli::runCommand( const QString& cmd, QString* result )
    {
    updateTimer.stop();
    QString command = cmd.trimmed();
    result->clear();
    if( command.isEmpty())
        return false;
    foreach( MinicliHandler* handler, handlers )
        {
        MinicliHandler::HandledState handled = handler->run( command, dialog, result );
        if( handled != MinicliHandler::NotHandled )
            return handled == MinicliHandler::HandledOk;
        }
    *result = i18n( "Could not run the specified command" );
    return false;
    }

void Minicli::commandChanged( const QString& command )
    {
    if( config.disableProgressParsing())
        return;
    updateText = command;
    updateTimer.start( 200 );
    }

void Minicli::doUpdate()
    {
    QString command = updateText.trimmed();
    QString iconName;
    if( !command.isEmpty())
        {
        foreach( MinicliHandler* handler, handlers )
            {
            MinicliHandler::HandledState handled = handler->update( command, &iconName );
            if( handled != MinicliHandler::NotHandled )
                break;
            }
        }
    dialog->updateIcon( iconName );
    }

QStringList Minicli::finalURIFilters() const
    {
    static QStringList filters = makeURIFilters( config.removeFinalURIFilters());
    return filters;
    }

QStringList Minicli::progressURIFilters() const
    { // TODO thread-unsafe with default KDE build flags
    static QStringList filters = makeURIFilters( config.removeProgressURIFilters());
    return filters;
    }

QStringList Minicli::makeURIFilters( const QStringList& remove )
    {
    if( remove.count() == 1 && remove.first() == "all" )
        return QStringList();
    QStringList filters = KUriFilter::self()->pluginNames();
    // remove everything not wanted
    foreach( QString filter, remove )
        filters.removeAll( filter );
    return filters;
    }

} // namespace

#include "minicli.moc"
