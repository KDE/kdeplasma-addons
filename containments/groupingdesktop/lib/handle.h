/*
 *   Copyright 2007 by Kevin Ottens <ervin@kde.org>
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

#ifndef PLASMA_APPLETHANDLE_P_H
#define PLASMA_APPLETHANDLE_P_H

#include <QtCore/QObject>
#include <QtGui/QGraphicsObject>
#include <QTimer>
#include <QWeakPointer>
#include <QPropertyAnimation>

#include <Plasma/Animator>
#include <Plasma/Svg>

class QGraphicsView;

namespace Plasma
{
    class Applet;
    class FrameSvg;
    class View;
};

class AbstractGroup;
class GroupingContainment;

class Handle : public QGraphicsObject
{
    Q_OBJECT
    Q_PROPERTY(qreal fadeAnimation READ fadeAnimation WRITE setFadeAnimation)
    Q_INTERFACES(QGraphicsItem)
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
            MaximizeButton
        };

        Handle(GroupingContainment *parent, Plasma::Applet *applet, const QPointF &hoverPos);
        Handle(GroupingContainment *parent, AbstractGroup *group, const QPointF &hoverPos);
        virtual ~Handle();

        void detachWidget();

        Plasma::Applet *applet() const;
        AbstractGroup *group() const;
        QGraphicsWidget *widget() const;

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
        bool sceneEvent(QEvent*);

    Q_SIGNALS:
       void disappearDone(Handle *self);
       void widgetMoved(QGraphicsWidget *widget);

    private Q_SLOTS:
        void setFadeAnimation(qreal progress);
        qreal fadeAnimation() const;
        void widgetDestroyed();
        void widgetResized();
        void hoverTimeout();
        void leaveTimeout();
        void emitDisappear();

    private:
        static const int HANDLE_MARGIN = 3;

        void init();
        void calculateSize();
        ButtonType mapToButton(const QPointF &point) const;
        void forceDisappear();
        int minimumHeight();

        /**
         * move our widget to another containment
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
        Plasma::Applet *m_applet;
        AbstractGroup *m_group;
        QGraphicsWidget *m_widget;
        int m_iconSize;
        qreal m_opacity;
        FadeType m_animType;
        QWeakPointer<QPropertyAnimation> m_anim;
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
        QPointF m_origWidgetCenter;
        QPointF m_origWidgetSize;

        // used for resize
        QPointF m_resizeStaticPoint;
        QPointF m_resizeGrabPoint;
        // used for rotate
        qreal m_rotateAngleOffset; // applet angle minus cursor angle

        bool m_buttonsOnRight : 1;
        bool m_pendingFade : 1;
};

#endif // multiple inclusion guard
