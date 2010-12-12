/*
 *   Copyright 2007 by Kevin Ottens <ervin@kde.org>
 *   Copyright 2009-2010 by Giulio Camuffo <giuliocamuffo@gmail.com>
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

#ifndef HANDLE_P_H
#define HANDLE_P_H

#include <QtCore/QObject>
#include <QtGui/QGraphicsObject>

#include <Plasma/Animator>
#include <Plasma/Svg>

class QGraphicsView;

namespace Plasma
{
    class Applet;
    class View;
}

class AbstractGroup;
class GroupingContainment;

class Handle : public QGraphicsObject
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
    public:
        enum ButtonType {
            NoButton,
            MoveButton,
            RotateButton,
            ConfigureButton,
            RemoveButton,
            ResizeButton,
            MaximizeButton
        };

        Handle(GroupingContainment *parent, Plasma::Applet *applet);
        Handle(GroupingContainment *parent, AbstractGroup *group);
        virtual ~Handle();

        virtual void detachWidget();

        Plasma::Applet *applet() const;
        AbstractGroup *group() const;
        QGraphicsWidget *widget() const;
        GroupingContainment *containment() const;
        QGraphicsView *currentView() const;

        virtual void setHoverPos(const QPointF &hoverPos);

    protected:
        /**
        * move our widget to another containment
        * @param containment the containment to move to
        * @param pos the (scene-relative) position to place it at
        */
        void switchContainment(GroupingContainment *containment, const QPointF &pos);
        bool leaveCurrentView(const QPoint &pos) const;

    Q_SIGNALS:
       void disappearDone(Handle *self);
       void widgetMoved(QGraphicsWidget *widget);

    private Q_SLOTS:
        void widgetDestroyed();

    private:
        GroupingContainment *m_containment;
        Plasma::Applet *m_applet;
        AbstractGroup *m_group;
        QGraphicsWidget *m_widget;
        QGraphicsView *m_currentView;
};

#endif // multiple inclusion guard
