/*
    Copyright (C) 2009  Matteo Agostinelli <agostinelli@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "qalculate_graphicswidget.h"

#include <QGraphicsSceneMouseEvent>

QalculateGraphicsWidget::QalculateGraphicsWidget(QGraphicsItem* parent, Qt::WindowFlags wFlags): QGraphicsWidget(parent, wFlags)
{

}

void QalculateGraphicsWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Up) {
        emit previousHistory();
    }
    if (event->key() == Qt::Key_Down) {
        emit nextHistory();
    }
    
    QGraphicsWidget::keyPressEvent(event);
}

void QalculateGraphicsWidget::focusInEvent(QFocusEvent* event)
{
    if (event->gotFocus()) {
        emit giveFocus();
    }
    
    QGraphicsWidget::focusInEvent(event);
}

void QalculateGraphicsWidget::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        emit giveFocus();
    }
    
    QGraphicsItem::mousePressEvent(event);
}
