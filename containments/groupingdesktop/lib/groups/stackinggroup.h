/*
 *   Copyright 2010 by Giulio Camuffo <giuliocamuffo@gmail.com>
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
        explicit StackingGroup(QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);
        ~StackingGroup();

        QString pluginName() const;
        void restoreChildGroupInfo(QGraphicsWidget *child, const KConfigGroup &group);
        void saveChildGroupInfo(QGraphicsWidget *child, KConfigGroup group) const;
        bool showDropZone(const QPointF &pos);
        void releaseChild(QGraphicsWidget *child);

        static GroupInfo groupInfo();

    protected:
        void layoutChild(QGraphicsWidget *child, const QPointF &pos);
        void resizeEvent(QGraphicsSceneResizeEvent *event);
        void wheelEvent(QGraphicsSceneWheelEvent *event);
        bool eventFilter(QObject *obj, QEvent *event);

    private slots:
        void onAppletAdded(Plasma::Applet *applet, AbstractGroup *group);
        void onAppletRemoved(Plasma::Applet *applet, AbstractGroup *group);
        void onSubGroupAdded(AbstractGroup *subGroup, AbstractGroup *group);
        void onSubGroupRemoved(AbstractGroup *subGroup, AbstractGroup *group);
        void onAppletActivated();

    private:
        void drawStack();

        QList<QGraphicsWidget *> m_children;
        Spacer *m_spacer;
};

#endif // STACKINGGROUP_H
