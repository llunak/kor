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

#ifndef KOR_HELLOAPPLET_H
#define KOR_HELLOAPPLET_H

#include <qlabel.h>

#include "applet.h"

namespace Kor
{

class HelloApplet
    : public QLabel, public Applet
    {
    Q_OBJECT
    public:
        HelloApplet( Kor::Panel* panel );
        virtual void load( const QString& id );
    };

} // namespace

#endif
