/*
 *   Copyright 2009 by Giulio Camuffo <giuliocamuffo@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef APPLETOVERLAY_H
#define APPLETOVERLAY_H

#include <QtGui/QGraphicsWidget>

namespace Plasma
{
    class Applet;
}

class AppletOverlay : public QGraphicsWidget
{
    Q_OBJECT
    public:
        AppletOverlay(Plasma::Applet *applet, Qt::WindowFlags wFlags = 0);
        ~AppletOverlay();

        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);
        Plasma::Applet *applet() const;

    protected:
        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

    signals:
        void startMoving();
        void movedOf(qreal x, qreal y);
        void endMoving();
        void appletMovedOutside(qreal x, qreal y);

    private slots:
        void syncGeometry();
        void delayedSyncGeometry();

    private:
        Plasma::Applet *m_applet;
        bool m_moving;
        QPointF m_startPos;
};

#endif // APPLETOVERLAY_H
