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

#ifndef QALCULATE_GRAPHICSWIDGET_H
#define QALCULATE_GRAPHICSWIDGET_H

#include <QtGui/QGraphicsWidget>

//! Qalculate! applet
/*! This is the main class of the applet.
*/
class QalculateGraphicsWidget : public QGraphicsWidget
{
    Q_OBJECT
    
public:
    explicit QalculateGraphicsWidget(QGraphicsItem* parent = 0, Qt::WindowFlags wFlags = 0);
    
protected:
    void keyPressEvent(QKeyEvent * event);
    void focusInEvent(QFocusEvent* event);
    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    
signals:
    void previousHistory();
    void nextHistory();
    void giveFocus();
};

#endif // QALCULATE_GRAPHICSWIDGET_H
