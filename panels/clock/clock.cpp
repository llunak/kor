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

#include "clock.h"

#include <kconfiggroup.h>
#include <kglobal.h>
#include <kicon.h>
#include <klocale.h>
#include <ksharedconfig.h>
#include <kwindowsystem.h>
#include <qevent.h>
#include <qlayout.h>

#include "clockconfig.h"
#include "panel.h"

namespace Kor
{

ClockApplet::ClockApplet( Kor::Panel* panel )
    : QWidget( panel->window())
    , Applet( panel )
    , datePicker( NULL )
    , timeLabel( NULL )
    , dateLabel( NULL )
    {
    connect( &timer, SIGNAL( timeout()), this, SLOT( updateClock()));
    }

void ClockApplet::load( const QString& id )
    {
    ClockAppletConfig config( id );
    showSeconds = config.showSeconds();
    showDate = config.showDate();
    setupLayout();
    updateClock();
    timer.start( 1000 ); // TODO longer interval if not showing seconds
    }

void ClockApplet::setupLayout()
    { // TODO maybe optimize
    delete timeLabel;
    timeLabel = new QLabel( this );
    delete dateLabel;
    if( showDate == Config::DateHidden )
        dateLabel = NULL;
    else
        dateLabel = new QLabel( this );
    delete layout();
    QLayout* lay = NULL;
    if( showDate == Config::DateAside || ( showDate == Config::DateAutomatic && panel->horizontal() && height() < 32 ))
        lay = new QBoxLayout( QBoxLayout::LeftToRight, this );
    else
        lay = new QBoxLayout( QBoxLayout::TopToBottom, this );
    setLayout( lay );
    lay->addWidget( timeLabel );
    if( dateLabel != NULL )
        lay->addWidget( dateLabel );
    }

void ClockApplet::resizeEvent( QResizeEvent* event )
    {
    QWidget::resizeEvent( event );
// TODO crashes right now    setupLayout();
    }

void ClockApplet::updateClock()
    {
    QDateTime dt = QDateTime::currentDateTime();
    timeLabel->setText( timeString( dt.time()));
    if( dateLabel != NULL )
        dateLabel->setText( dateString( dt.date()));
    update();
    }

QString ClockApplet::timeString( const QTime& time ) const
    {
    return KGlobal::locale()->formatTime( time, showSeconds );
    }

QString ClockApplet::dateString( const QDate& date ) const
    {
    return KGlobal::locale()->formatDate( date, KLocale::ShortDate );
    }

void ClockApplet::mousePressEvent( QMouseEvent* event )
    {
    QWidget::mousePressEvent( event );
    if( event->button() != Qt::LeftButton )
        return;
    if( datePicker == NULL )
        {
        // TODO
        // handle things from kicker (close on escape, etc.)
        // global function for the window activation?
        datePicker = new DatePicker( this );
        // It is convenient to have the date picker kept on top by default, as that makes
        // it easy to focus something else and have the (temporary, anyway) date picker still visible.
        KWindowSystem::setState( datePicker->winId(), NET::KeepAbove );
        datePicker->setAttribute( Qt::WA_DeleteOnClose );
        connect( datePicker, SIGNAL( destroyed()), this, SLOT( datePickerDeleted()));
        datePicker->move( popupPosition( datePicker, this ));
        datePicker->show();
        // Activate the window. KWin will not activate the window automatically, because the panel is not active.
        // This call activates it normally (not forced), because the user activity is in the panel (user timestamp).
        // Other calls such as QWidget::activateWindow() are kind of broken for this.
        KWindowSystem::activateWindow( datePicker->winId());
        }
    else
        {
        delete datePicker;
        datePicker = NULL;
        }
    }

void ClockApplet::datePickerDeleted()
    {
    datePicker = NULL;
    }

ClockApplet::DatePicker::DatePicker( QWidget* parent )
    : KDatePicker( parent )
    {
    setWindowFlags( Qt::Tool ); // KDatePicker doesn't take Qt::WindowFlags in its ctor
    if( parent != NULL ) // no idea why this doesn't work automatically
        KWindowSystem::setMainWindow( this, parent->window()->winId());
    setWindowRole( "calendar" );
    setWindowIcon( KIcon( "view-calendar" ));
    setWindowTitle( i18n( "Calendar" ));
    }

void ClockApplet::DatePicker::keyPressEvent( QKeyEvent* event )
    {
    KDatePicker::keyPressEvent( event );
    if( event->key() == Qt::Key_Escape )
        close();
    }

} // namespace

#include "clock.moc"
