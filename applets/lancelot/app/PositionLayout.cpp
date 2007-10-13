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

#include "PositionLayout.h"
#include <KDebug>

namespace Lancelot
{


PositionLayout::PositionLayout(LayoutItem * parent) 
  : Plasma::Layout(parent)
{
}

PositionLayout::~PositionLayout()
{}

Qt::Orientations PositionLayout::expandingDirections() const
{
    return Qt::Horizontal | Qt::Vertical;
}

QRectF PositionLayout::geometry() const {
	return m_geometry;
}

void PositionLayout::setGeometry(const QRectF& geometry)
{
    if (!geometry.isValid() || geometry.isEmpty()) {
        kDebug() << "Invalid Geometry " << geometry;
        return;
    }
    
    for (int i = 0 ; i < count() ; i++) {
        LayoutItem *l = itemAt(i);
        if (!m_itemPositions.contains(l)) continue;
        l->setGeometry(m_itemPositions[l].calculateRectangle(geometry));
    }

    m_geometry = geometry;
}

QSizeF PositionLayout::sizeHint() const
{
    qreal hintHeight = 0.0;
    qreal hintWidth = 0.0;

    return QSizeF(hintWidth, hintHeight);
}

void PositionLayout::addItem (Plasma::LayoutItem * item) {
    PositionLayout::addItem (item, Position(0, 0, 0.2, 0.2));
}

void PositionLayout::addItem (Plasma::LayoutItem * item, Position position) {
    m_itemPositions[item] = position;
    m_items.append(item);
}

void PositionLayout::removeItem (Plasma::LayoutItem * item) {
    m_itemPositions.remove(item);
    m_items.removeAll(item);
}

QRectF PositionLayout::Position::calculateRectangle(QRectF geometry) {
    QRectF rect;

    // X1 coordinate
    float absv = fabs(m_x1);
    if (absv <= 1.0) absv = geometry.width() * absv;         // In percents
    rect.setLeft(m_x1 >= 0 ? absv : geometry.width() - absv);

    // Y1 coordinate
    absv = fabs(m_y1);
    if (absv <= 1.0) absv = geometry.height() * absv;        // In percents
    rect.setTop(m_y1 >= 0 ? absv : geometry.height() - absv);

    // X2 coordinate
    absv = fabs(m_x2);
    if (absv <= 1.0) absv = geometry.width() * absv;         // In percents
    rect.setWidth((m_x2 >= 0 ? absv : geometry.width() - absv) - rect.left());

    // Y2 coordinate
    absv = fabs(m_y2);
    if (absv <= 1.0) absv = geometry.height() * absv;        // In percents
    rect.setHeight((m_y2 >= 0 ? absv : geometry.height() - absv) - rect.top());
    
    rect.moveTopLeft(rect.topLeft() + geometry.topLeft());
    
    return rect;
}

int PositionLayout::count() const {
	return m_items.size();
}

int PositionLayout::indexOf(Plasma::LayoutItem * item) const {
	return m_items.indexOf(item);
}

Plasma::LayoutItem * PositionLayout::itemAt(int i) const {
	return m_items[i];
}

Plasma::LayoutItem * PositionLayout::takeAt(int i) {
	Plasma::LayoutItem * item = itemAt(i);
	removeItem(item);
	return item;
}


}
