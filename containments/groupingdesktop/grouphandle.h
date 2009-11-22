/*
 *   Copyright 2009 by Giulio Camuffo <giuliocamuffo@kde.org>
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

#ifndef GROUPHANDLE_P_H
#define GROUPHANDLE_P_H

#include <QtCore/QObject>
#include <QtGui/QGraphicsObject>
#include <QTimer>

#include <Plasma/Svg>

class QGraphicsView;

namespace Plasma {
    class FrameSvg;
    class View;
};

class GroupingContainment;
class AbstractGroup;

class GroupHandle : public QGraphicsObject
{
    Q_OBJECT
#if QT_VERSION >= 0x040600
    Q_INTERFACES(QGraphicsItem)
#endif
    public:
        enum FadeType {
            FadeIn,
            FadeOut
        };
        enum ButtonType {
            NoButton,
            MoveButton,
            RotateButton,
            ConfigureButton,
            RemoveButton,
            ResizeButton,
        };

        GroupHandle(GroupingContainment *parent, AbstractGroup *group, const QPointF &hoverPos);
        virtual ~GroupHandle();

        void detachGroup ();

        AbstractGroup *group() const;

        QRectF boundingRect() const;
        QPainterPath shape() const;
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
        void startFading(FadeType anim, const QPointF &hoverPos, bool preserveSide = false);
        void setHoverPos(const QPointF &hoverPos);

    protected:
        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
        bool sceneEventFilter(QGraphicsItem *watched, QEvent *event);

    Q_SIGNALS:
       void disappearDone(GroupHandle *self);

    private Q_SLOTS:
        void fadeAnimation(qreal progress);
        void groupDestroyed();
        void groupResized();
        void hoverTimeout();
        void leaveTimeout();
        void emitDisappear();

    private:
        static const int HANDLE_MARGIN = 3;

        void calculateSize();
        ButtonType mapToButton(const QPointF &point) const;
        void forceDisappear();
        int minimumHeight();

        /**
         * move our applet to another containment
         * @param containment the containment to move to
         * @param pos the (scene-relative) position to place it at
         */
        void switchContainment(GroupingContainment *containment, const QPointF &pos);
        bool leaveCurrentView(const QPoint &pos) const;

        QRectF m_rect;
        QRectF m_decorationRect;
        QRectF m_totalRect;
        ButtonType m_pressedButton;
        GroupingContainment *m_containment;
        AbstractGroup *m_group;
        int m_iconSize;
        qreal m_opacity;
        FadeType m_anim;
        int m_animId;
        qreal m_angle;
        QColor m_gradientColor;
        QTimer *m_hoverTimer;
        QTimer *m_leaveTimer;
        QPixmap *m_backgroundBuffer;
        QGraphicsView *m_currentView;

        Plasma::Svg *m_configureIcons;
        Plasma::FrameSvg *m_background;

        QPoint m_mousePos;  //mousepos relative to applet
        QPointF m_entryPos; //where the hover in event occurred
        qreal m_zValue;     //current zValue of the applet, so it can be restored after drag.
        QRectF m_originalGeom;
        QTransform m_originalTransform;

        // used for both resize and rotate
        QPointF m_origGroupCenter;
        QPointF m_origGroupSize;

        // used for resize
        QPointF m_resizeStaticPoint;
        QPointF m_resizeGrabPoint;
        // used for rotate
        qreal m_rotateAngleOffset; // applet angle minus cursor angle

        bool m_buttonsOnRight : 1;
        bool m_pendingFade : 1;
};

#endif // multiple inclusion guard
