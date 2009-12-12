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

#ifndef GROUPINGCONTAINMENT_H
#define GROUPINGCONTAINMENT_H

#include <Plasma/Containment>

class AbstractGroup;
class GroupingContainmentPrivate;

class PLASMA_EXPORT GroupingContainment: public Plasma::Containment
{
    Q_OBJECT
    public:
        GroupingContainment(QObject *parent, const QVariantList &args);
        virtual ~GroupingContainment();

        void init();

        void saveContents(KConfigGroup &group) const;
        void restoreContents(KConfigGroup &group);
        QList<QAction *> contextualActions();
        void addGroup(const QString &plugin, const QPointF &pos, int id = 0);
        void addGroup(AbstractGroup *group, const QPointF &pos);

    protected:
        virtual bool eventFilter(QObject *obj, QEvent *event);
        virtual bool sceneEventFilter(QGraphicsItem *watched, QEvent *event);
        virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);

    signals:
        void groupAdded(AbstractGroup *group, const QPointF &pos);
        void groupRemoved(AbstractGroup *group);

    private:
        Q_PRIVATE_SLOT(d, void handleDisappeared(GroupHandle *handle))
        Q_PRIVATE_SLOT(d, void onGroupRemoved(AbstractGroup *group))
        Q_PRIVATE_SLOT(d, void layoutApplet(Plasma::Applet *applet, const QPointF &pos))
        Q_PRIVATE_SLOT(d, void newGroupClicked(QAction *action))
        Q_PRIVATE_SLOT(d, void deleteGroup())

        GroupingContainmentPrivate *const d;

        friend class GroupingContainmentPrivate;
};

#endif // GROUPINGCONTAINMENT_H
