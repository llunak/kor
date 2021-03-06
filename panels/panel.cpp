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

#include <assert.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kicon.h>
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
    setupWindow();
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
    QColor color( cfg.readEntry( "BackgroundColor" )); // QColor ctor rather than KConfig which can't handle 'green'
    QPalette pal = window()->palette();
    if( color.isValid())
        {
        pal.setColor( QPalette::Window, color );
        window()->setAutoFillBackground( true );
#if QT_VERSION >= 0x040500
        window()->setAttribute( Qt::WA_TranslucentBackground, color.alpha() != 255 );
#endif
        }
    else
        {
        pal.setColor( QPalette::Window, QPalette().color( QPalette::Window )); // default color
        window()->setAutoFillBackground( false );
#if QT_VERSION >= 0x040500
        window()->setAttribute( Qt::WA_TranslucentBackground, false );
#endif
        }
    window()->setPalette( pal );
    setupWindow();
    updatePosition();
    window()->update();
    }

void Panel::setupWindow()
    { // needs to be done after changing WA_TranslucentBackground, as that recreates the X window
    window()->setWindowRole( id.toLower());
    window()->setWindowIcon( KIcon( "user-desktop" ));
    KWindowSystem::setType( window()->winId(), NET::Dock );
    KWindowSystem::setOnAllDesktops( window()->winId(), true );
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
            pos = QPoint( center.x() - width / 2, screenGeom.bottom() + 1 - height );
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
            pos = QPoint( screenGeom.right() + 1 - width, screenGeom.top());
            break;
        case PositionBottomLeft:
            pos = QPoint( screenGeom.left(), screenGeom.bottom() + 1 - height );
            break;
        case PositionBottomRight:
            pos = QPoint( screenGeom.right() + 1 - width, screenGeom.bottom() + 1 - height );
            break;
        }
    window()->move( pos );
    window()->setFixedSize( width, height );
    switch( mainEdge())
        {
        case MainEdgeTop:
            {
            int strut = pos.y() - qApp->desktop()->y() + height; // struts are from total screen area, not monitor area
            KWindowSystem::setExtendedStrut( window()->winId(), 0, 0, 0, 0, 0, 0,
                strut, pos.x(), pos.x() + width - 1, 0, 0, 0 );
            break;
            }
        case MainEdgeBottom:
            {
            int strut = qApp->desktop()->geometry().bottom() + 1 - pos.y() - height + height;
            KWindowSystem::setExtendedStrut( window()->winId(), 0, 0, 0, 0, 0, 0,
                0, 0, 0, strut, pos.x(), pos.x() + width - 1 );
            break;
            }
        case MainEdgeLeft:
            {
            int strut = pos.x() - qApp->desktop()->x() + width;
            KWindowSystem::setExtendedStrut( window()->winId(), strut, pos.y(), pos.y() + height - 1, 0, 0, 0,
                0, 0, 0, 0, 0, 0 );
            break;
            }
        case MainEdgeRight:
            {
            int strut = qApp->desktop()->geometry().right() + 1 - pos.x() - width + width;
            KWindowSystem::setExtendedStrut( window()->winId(), 0, 0, 0, strut, pos.y(), pos.y() + height - 1,
                0, 0, 0, 0, 0, 0 );
            break;
            }
        }
    if( window()->layout() != NULL
        && !( horizontal() == ( static_cast< QBoxLayout* >( window()->layout())->direction() == QBoxLayout::LeftToRight )))
        { // delete old layout if it exists and is not of the right orientation
        delete window()->layout();
        }
    if( window()->layout() == NULL )
        new QBoxLayout( horizontal() ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom, window());
    static_cast< QBoxLayout* >( window()->layout())->setContentsMargins( 0, 0, 0, 0 );
    static_cast< QBoxLayout* >( window()->layout())->setSpacing( 0 );
    // TODO add already existing widgets?
    }

Panel::MainEdge Panel::mainEdge() const
    {
    if( horizontal() && ( position() & PositionTop ))
        return MainEdgeTop;
    else if( horizontal() && ( position() & PositionBottom ))
        return MainEdgeBottom;
    else if( !horizontal() && ( position() & PositionLeft ))
        return MainEdgeLeft;
    else if( !horizontal() && ( position() & PositionRight ))
        return MainEdgeRight;
    else
        abort();
    }

} // namespace

#include "panel.moc"
