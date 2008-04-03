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

#include "WidgetPositioner.h"

namespace Lancelot
{

WidgetPositioner::WidgetPositioner(QWidget * widget, QGraphicsView * view, QGraphicsItem * parent)
    : Widget("WidgetPositioner", parent), m_widget(widget), m_view(view)
{
}

WidgetPositioner::~WidgetPositioner()
{
}

void WidgetPositioner::setGeometry(const QRectF & geometry)
{
    m_geometry = geometry;
    if (m_widget) {
        QPoint pos = m_view->mapFromScene(mapToScene(m_geometry.topLeft()));
        m_widget->setGeometry(QRect(pos, geometry.size().toSize()));
        //m_widget->setGeometry(sceneBoundingRect().toRect());
    }
}

QRectF WidgetPositioner::geometry() const
{
    return m_geometry;
}

QSizeF WidgetPositioner::sizeHint() const
{
    if (m_widget) {
        return m_widget->sizeHint();
    }
    return QSizeF();
}


}
