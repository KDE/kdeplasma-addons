/***************************************************************************
 *   Copyright 2009 by Davide Bettio <davide.bettio@kdemail.net>           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef BLACKBOARDWIDGET_H
#define BLACKBOARDWIDGET_H

#include <QGraphicsWidget>
#include <QPainter>
#include <QPixmap>
#include <QPointF>

class QGraphicsSceneMouseEvent;
class QStyleOptionGraphicsItem;

namespace Plasma
{
    class Applet; 
}

class BlackBoardWidget : public QGraphicsWidget
{
    Q_OBJECT

    public:
        BlackBoardWidget(Plasma::Applet *parent);
        ~BlackBoardWidget();

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseMoveEvent ( QGraphicsSceneMouseEvent *event );
        void mouseReleaseEvent ( QGraphicsSceneMouseEvent *event );
        void resizeEvent(QGraphicsSceneResizeEvent *event);

        void setBrushColor(QColor color);

    protected:
        bool event(QEvent *event);

    public Q_SLOTS:
        void saveImage();
	void erase();
	
    private Q_SLOTS:
    	void loadImage();
	
    private:
        QString imagePath();
      
	void drawSegment(QPointF point0, QPointF point1, qreal penRadius);
        bool m_changed;
	Plasma::Applet *m_parentApplet;
        QString m_id;
        QColor m_color;
        QPixmap m_pixmap;
        QPointF m_oldPoint;
        QPainter m_painter;
};

#endif
