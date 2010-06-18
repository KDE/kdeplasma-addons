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

#ifndef STACKINGGROUP_H
#define STACKINGGROUP_H

#include "abstractgroup.h"

class Spacer;

class StackingGroup : public AbstractGroup
{
    Q_OBJECT
    public:
        StackingGroup(QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);
        virtual ~StackingGroup();

        virtual QString pluginName() const;
        virtual void restoreChildGroupInfo(QGraphicsWidget *child, const KConfigGroup &group);
        virtual void saveChildGroupInfo(QGraphicsWidget *child, KConfigGroup group) const;
        void showDropZone(const QPointF &pos);

    protected:
        virtual void layoutChild(QGraphicsWidget *child, const QPointF &pos);
        virtual void resizeEvent(QGraphicsSceneResizeEvent *event);

    private slots:
        void onInitCompleted();
        void onWidgetStartsMoving(QGraphicsWidget *widget);
        void onAppletAdded(Plasma::Applet *applet, AbstractGroup *group);
        void onAppletRemoved(Plasma::Applet *applet, AbstractGroup *group);
        void onSubGroupAdded(AbstractGroup *subGroup, AbstractGroup *group);
        void onSubGroupRemoved(AbstractGroup *subGroup, AbstractGroup *group);

    private:
        void drawStack();

        QList<QGraphicsWidget *> m_children;
        Spacer *m_spacer;
};

#endif // STACKINGGROUP_H
