/*
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

#ifndef GROUPINGCONTAINMENTPRIVATE_H
#define GROUPINGCONTAINMENTPRIVATE_H

#include "groupingcontainment.h"

class QGraphicsWidget;
class QGraphicsLinearLayout;
class QAction;

class KMenu;

namespace Plasma {
    class Applet;
}

class AbstractGroup;
class Handle;

class GroupingContainmentPrivate
{
    public:
        GroupingContainmentPrivate(GroupingContainment *containment);
        ~GroupingContainmentPrivate();

        AbstractGroup *createGroup(const QString &plugin, const QPointF &pos, unsigned int id, bool delayInit = false);
        void handleDisappeared(Handle *handle);
        void onGroupRemoved(AbstractGroup *group);
        void onAppletRemoved(Plasma::Applet *applet);
        AbstractGroup *groupAt(const QPointF &pos, QGraphicsWidget *uppermostWidget = 0);
        QList<AbstractGroup *> groupsAt(const QPointF &pos, QGraphicsWidget *uppermostWidget = 0);
        void manageApplet(Plasma::Applet *applet, const QPointF &pos);
        void manageGroup(AbstractGroup *subGroup, const QPointF &pos);
        void newGroupClicked();
        void deleteGroup();
        void configureGroup();
        void onAppletRemovedFromGroup(Plasma::Applet *applet, AbstractGroup *group);
        void onSubGroupRemovedFromGroup(AbstractGroup *subGroup, AbstractGroup *group);
        void onWidgetMoved(QGraphicsWidget *widget);
        void onImmutabilityChanged(Plasma::ImmutabilityType immutability);
        void restoreGroups();
        void prepareWidgetToMove();
        void widgetMovedAnimationComplete();

        GroupingContainment *q;
        QList<AbstractGroup *> groups;
        QWeakPointer<AbstractGroup> interestingGroup;
        QMap<QGraphicsWidget *, Handle *> handles;
        QAction *newGroupAction;
        QAction *separator;
        QAction *deleteGroupAction;
        QAction *configureGroupAction;
        QPointF lastClick;
        AbstractGroup *mainGroup;
        unsigned int mainGroupId;
        QGraphicsLinearLayout *layout;
        QString mainGroupPlugin;
        bool loading;
        QGraphicsWidget *movingWidget;
        QGraphicsWidget *interestingWidget;
        QGraphicsWidget *movementHelperWidget;
        QGraphicsWidget *widgetToBeSetMoving;
        bool blockSceneEventFilter;

        static int s_maxZValue;
        static unsigned int s_groupId;
};

#endif
