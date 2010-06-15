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

#ifndef KOR_PLASMAAPPLET_H
#define KOR_PLASMAAPPLET_H

#include <plasma/applet.h>
#include <plasma/containment.h>
#include <plasma/corona.h>
#include <qgraphicsview.h>

namespace Kor
{

// based on plasma's plasmoidviewer
class PlasmaApplet
    : public QGraphicsView
    {
    Q_OBJECT
    public:
        PlasmaApplet( QWidget* parent );
        void init();
    protected:
        virtual void resizeEvent( QResizeEvent* event );
    private slots:
        void appletRemoved();
    private:
        Plasma::Corona corona;
        Plasma::Containment* containment;
        Plasma::Applet* applet;
    };

} // namespace

#endif