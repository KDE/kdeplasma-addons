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
    QalculateGraphicsWidget(QGraphicsItem* parent = 0, Qt::WindowFlags wFlags = 0);
    
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
