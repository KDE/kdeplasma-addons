/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser/Library General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser/Library General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser/Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef LANCELOT_FLIPLAYOUT_H_
#define LANCELOT_FLIPLAYOUT_H_

#include <plasma/widgets/layout.h>
#include <plasma/widgets/widget.h>
#include <cmath>
#include <QMap>

#include <KDebug>

namespace Lancelot
{

class FlipLayoutGlobal {
public:
    enum Flip {
        No = 0,
        Vertical = 1,
        Horizontal = 2,
        Both = 3
    };
    
    static void setFlip(Flip directions) {
        if (m_flip == directions) return;
        m_flip = directions;
        //SuperLayout::update();
    }
    
    static Flip flip() {
        return m_flip;
    }
    
private:
    static Flip m_flip;

};

template <typename SuperLayout>
class FlipLayout : public SuperLayout { //Plasma::Layout
public:
    
    void setGeometry(const QRectF & geometry)
    {
        if (
            !geometry.isValid() ||
            this->geometry() == geometry
        ) return;

        kDebug() << "Master" << geometry;
        SuperLayout::setGeometry(geometry);
        
        if (FlipLayoutGlobal::flip() == FlipLayoutGlobal::No) return;
        
        QRectF childGeometry;
        for (int i = 0; i < SuperLayout::count(); i++) {
            kDebug() << "Item " << i;            
            Plasma::LayoutItem * item = SuperLayout::itemAt(i);
            
            if (!item) continue;
            
            childGeometry = item->geometry();
            kDebug() << "Pre flip" << childGeometry;
            if (FlipLayoutGlobal::flip() & FlipLayoutGlobal::Horizontal) {
                // 2 * geometry.left() - twice because we already have one
                // value of geometry.left() inside the childGeometry.left()
                childGeometry.moveLeft(
                    2 * geometry.left()       + geometry.width()
                      - childGeometry.left()  - childGeometry.width()
                );
            }
            if (FlipLayoutGlobal::flip() & FlipLayoutGlobal::Vertical) {
                // 2 * geometry.top() - same reason as aforemontioned
                childGeometry.moveTop(
                    2 * geometry.top()      + geometry.height()
                      - childGeometry.top() - childGeometry.height()
                );
            }
            kDebug() << "Post flip" << childGeometry;
            item->setGeometry(childGeometry);
        }
    }
    
};

}

#endif /*FlipLayout_H_*/
