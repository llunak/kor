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

#include "panel.h"

#include <kconfiggroup.h>
#include <kdebug.h>
#include <kglobal.h>
#include <ksharedconfig.h>
#include <kwindowsystem.h>
#include <qboxlayout.h>

#include <kdeversion.h>
#include <qapplication.h>
#include <qdesktopwidget.h>

#include "applet.h"

namespace Kor
{

Panel::Panel( const QString& id, QObject* parent )
    : QObject( parent )
    , id( id )
    , win( new QWidget )
    {
    window()->setWindowRole( id.toLower());
    KWindowSystem::setType( window()->winId(), NET::Dock );
    KWindowSystem::setOnAllDesktops( window()->winId(), true );
    loadConfig();
    loadApplets();
    window()->show();
    }

void Panel::loadConfig()
    {
    KConfigGroup cfg( KGlobal::config(), id );
    QString tmp = cfg.readEntry( "Position", "0x02" ); // default is bottom
    bool ok = false;
    pos = Position( tmp.toInt( &ok, 0 )); // TODO check
    horiz = cfg.readEntry( "Horizontal", bool( position() & ( PositionTop | PositionBottom )));
    configuredWidth = cfg.readEntry( "Width", 24 ); // TODO
    configuredLength = cfg.readEntry( "Length", int( FullLength ));
    configuredScreen = cfg.readEntry( "Screen", qApp->desktop()->primaryScreen());
    updatePosition();
    }

void Panel::loadApplets()
    {
    KConfigGroup cfg( KGlobal::config(), id );
    foreach( const QString& appletid, cfg.readEntry( "Applets", QStringList()))
        {
        KConfigGroup appletcfg( KGlobal::config(), appletid );
        QString type = appletcfg.readEntry( "Type" );
        Applet* applet = Applet::create( type, this );
        if( applet == NULL )
            {
            kWarning() << "Cannot load applet type " << type;
            continue;
            }
        applet->load( appletid );
        if( QWidget* w = dynamic_cast< QWidget* >( applet ))
            window()->layout()->addWidget( w );
        else
            kWarning() << "Unknown layout item for applet";
        applets.append( applet );
        }
    }

void Panel::updatePosition()
    {
    int screen = qBound( 0, configuredScreen, qApp->desktop()->numScreens() - 1 );
    QRect screenGeom = qApp->desktop()->screenGeometry( screen );
    int width; // configuredWidth is thickness, but this is widthxheight
    int height;
    if( horizontal())
        {
        width = qMin( configuredLength, screenGeom.width());
        height = configuredWidth;
        }
    else
        {
        height = qMin( configuredLength, screenGeom.height());
        width = configuredWidth;
        }
    QPoint pos;
    // QRect::center() for 1280x960+0+0 gives 639,479 , so calculate center to give 640,480
    QPoint center( screenGeom.x() + screenGeom.width() / 2, screenGeom.y() + screenGeom.height() / 2 );
    switch( position())
        {
        case PositionTop:
            pos = QPoint( center.x() - width / 2, screenGeom.top());
            break;
        case PositionBottom:
            pos = QPoint( center.x() - width / 2, screenGeom.bottom() - height );
            break;
        case PositionLeft:
            pos = QPoint( screenGeom.left(), center.y() - height / 2 );
            break;
        case PositionRight:
            pos = QPoint( screenGeom.right() - width, center.y() - height / 2 );
            break;
        case PositionTopLeft:
            pos = screenGeom.topLeft();
            break;
        case PositionTopRight:
            pos = QPoint( screenGeom.right() - width, screenGeom.top());
            break;
        case PositionBottomLeft:
            pos = QPoint( screenGeom.left(), screenGeom.bottom() - height );
            break;
        case PositionBottomRight:
            pos = QPoint( screenGeom.right() - width, screenGeom.bottom() - height );
            break;
        }
    window()->move( pos );
    window()->setFixedSize( width, height );
    if( horizontal() && ( position() & PositionTop ))
        {
        int strut = pos.y() - qApp->desktop()->y() + height; // struts are from total screen area, not monitor area
        KWindowSystem::setExtendedStrut( window()->winId(), 0, 0, 0, 0, 0, 0,
            strut, pos.x(), pos.x() + width - 1, 0, 0, 0 );
        }
    else if( horizontal() && ( position() & PositionBottom ))
        {
        int strut = qApp->desktop()->geometry().bottom() - pos.y() - height + height;
        KWindowSystem::setExtendedStrut( window()->winId(), 0, 0, 0, 0, 0, 0,
            0, 0, 0, strut, pos.x(), pos.x() + width - 1 );
        }
    else if( !horizontal() && ( position() & PositionLeft ))
        {
        int strut = pos.x() - qApp->desktop()->x() + width;
        KWindowSystem::setExtendedStrut( window()->winId(), strut, pos.y(), pos.y() + height - 1, 0, 0, 0,
            0, 0, 0, 0, 0, 0 );
        }
    else if( !horizontal() && ( position() & PositionRight ))
        {
        int strut = qApp->desktop()->geometry().right() - pos.x() - width + width;
        KWindowSystem::setExtendedStrut( window()->winId(), 0, 0, 0, strut, pos.y(), pos.y() + height - 1,
            0, 0, 0, 0, 0, 0 );
        }
    else
        abort();
    if( window()->layout() != NULL
        && !( horizontal() == ( static_cast< QBoxLayout* >( window()->layout())->direction() == QBoxLayout::LeftToRight )))
        {
        delete window()->layout();
        }
    QBoxLayout* l = new QBoxLayout( horizontal() ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom, window());
    l->setContentsMargins( 0, 0, 0, 0 );
    l->setSpacing( 0 );
    // TODO add already existing widgets?
    }

} // namespace

#include "panel.moc"
