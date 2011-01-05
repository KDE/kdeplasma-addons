/*
 *   Copyright 2009 Matteo Agostinelli <agostinelli@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   TYou should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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
