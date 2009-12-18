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
        virtual void save(KConfigGroup &group) const;
        virtual void restore(KConfigGroup &group);
        QList<QAction *> contextualActions();
        AbstractGroup *addGroup(const QString &plugin, const QPointF &pos = QPointF(0, 0), int id = 0);
        void addGroup(AbstractGroup *group, const QPointF &pos);
        QList<AbstractGroup *> groups() const;

    protected:
        virtual bool eventFilter(QObject *obj, QEvent *event);
        virtual bool sceneEventFilter(QGraphicsItem *watched, QEvent *event);
        virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
        virtual void saveContents(KConfigGroup &group) const;
        virtual void restoreContents(KConfigGroup &group);
        void setMainGroup(const QString &name);
        void setMainGroup(AbstractGroup *group);
        AbstractGroup *mainGroup() const;

    signals:
        void groupAdded(AbstractGroup *group, const QPointF &pos);
        void groupRemoved(AbstractGroup *group);

    private:
        Q_PRIVATE_SLOT(d, void handleDisappeared(GroupHandle *handle))
        Q_PRIVATE_SLOT(d, void onGroupRemoved(AbstractGroup *group))
        Q_PRIVATE_SLOT(d, void manageApplet(Plasma::Applet *applet, const QPointF &pos))
        Q_PRIVATE_SLOT(d, void newGroupClicked(QAction *action))
        Q_PRIVATE_SLOT(d, void deleteGroup())
        Q_PRIVATE_SLOT(d, void onAppletRemovedFromGroup(Plasma::Applet *applet, AbstractGroup *group))
        Q_PRIVATE_SLOT(d, void onSubGroupRemovedFromGroup(AbstractGroup *subGroup, AbstractGroup *group))

        GroupingContainmentPrivate *const d;

        friend class GroupingContainmentPrivate;
};

#endif // GROUPINGCONTAINMENT_H
