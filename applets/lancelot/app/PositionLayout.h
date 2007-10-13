/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free 
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef POSITIONLAYOUT_H_
#define POSITIONLAYOUT_H_

#include <plasma/widgets/layout.h>
#include <cmath>
#include <QMap>

namespace Lancelot
{

class PositionLayout : public Plasma::Layout
{
public:
    class Position {
    public:
        Position(float x1 = 0, float y1 = 0, float x2 = 0, float y2 = 0)
          : m_x1(x1), m_y1(y1), m_x2(x2), m_y2(y2) {}

    protected:
        float m_x1, m_y1;
        float m_x2, m_y2;
        QRectF calculateRectangle(QRectF geometry);
        friend class PositionLayout;
    };
    
    // reimplemented
    virtual Qt::Orientations expandingDirections() const;
    
    explicit PositionLayout(LayoutItem * parent = 0);
    virtual ~PositionLayout();
    
    virtual QRectF geometry() const;
    void setGeometry(const QRectF& geometry);

    QSizeF sizeHint() const;
    
    void addItem (Plasma::LayoutItem * item);
    void addItem (Plasma::LayoutItem * item, Position position);
    
    void removeItem (Plasma::LayoutItem * item);

    virtual int count() const;
    virtual int indexOf(Plasma::LayoutItem * item) const;
    virtual Plasma::LayoutItem * itemAt(int i) const;
    virtual Plasma::LayoutItem * takeAt(int i);
    

private:
    QMap< Plasma::LayoutItem * , Position > m_itemPositions;
    QList < Plasma::LayoutItem * > m_items;
    QRectF m_geometry;
};

}

#endif /*POSITIONLAYOUT_H_*/
