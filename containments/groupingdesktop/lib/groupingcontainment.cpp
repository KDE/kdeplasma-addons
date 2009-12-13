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

#include "groupingcontainment.h"

#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsSceneContextMenuEvent>
#include <QtGui/QAction>

#include <KDebug>
#include <KMenu>
#include <KIcon>

#include "abstractgroup.h"
#include "abstractgroup_p.h"
#include "grouphandle.h"
#include "gridgroup.h"
#include "floatinggroup.h"

class GroupingContainmentPrivate
{
    public:
        GroupingContainmentPrivate(GroupingContainment *containment)
            : q(containment),
              interestingGroup(0)
        {
            newGroupAction = new QAction(i18n("Add a new group"), q);
            newGroupMenu = new KMenu(i18n("Add a new group"), 0);
            newGroupAction->setMenu(newGroupMenu);
            newGridGroup = new QAction(i18n("Add a new grid group"), q);
            newGridGroup->setData("grid");
            newFloatingGroup = new QAction(i18n("Add a new floating group"), q);
            newFloatingGroup->setData("floating");
            newGroupMenu->addAction(newGridGroup);
            newGroupMenu->addAction(newFloatingGroup);

            deleteGroupAction = new QAction(i18n("Remove this group"), q);
            deleteGroupAction->setIcon(KIcon("edit-delete"));
            deleteGroupAction->setVisible(false);

            separator = new QAction(q);
            separator->setSeparator(true);

            q->connect(newGroupMenu, SIGNAL(triggered(QAction *)), q, SLOT(newGroupClicked(QAction *)));
            q->connect(deleteGroupAction, SIGNAL(triggered()), q, SLOT(deleteGroup()));
        }

        ~GroupingContainmentPrivate()
        {}

        AbstractGroup *createGroup(const QString &plugin, const QPointF &pos, unsigned int id)
        {
            kDebug()<<plugin;
            AbstractGroup *group = 0;
            if (plugin == "grid") {
                group = new GridGroup(q);
            } else if (plugin == "floating") {
                group = new FloatingGroup(q);
            }

            if (!group) {
                return 0;
            }

            if (groups.contains(group)) {
                delete group;
                return 0;
            }

            if (id == 0) {
                id = groups.count() + 1;
            }
            group->d->id = id;

            groups << group;

            q->addGroup(group, pos);

            return group;
        }

        void handleDisappeared(GroupHandle *handle)
        {
            if (handles.contains(handle->group())) {
                handles.remove(handle->group());
                handle->detachGroup();
                if (q->scene()) {
                    q->scene()->removeItem(handle);
                }
                handle->deleteLater();
            }
        }

        void onGroupRemoved(AbstractGroup *group)
        {
            kDebug()<<"Removed group"<<group->id();

            groups.removeAll(group);
            group->removeEventFilter(q);

            if (handles.contains(group)) {
                GroupHandle *handle = handles.value(group);
                handles.remove(group);
                delete handle;
            }

            emit q->groupRemoved(group);
        }

        void layoutApplet(Plasma::Applet *applet, const QPointF &pos)
        {
            foreach (AbstractGroup *group, groups) {
                if (group) {
                    QRectF rect = group->contentsRect();
                    rect.translate(group->pos());
                    if (rect.contains(pos)) {
                        group->addApplet(applet);

                        applet->installSceneEventFilter(q);
                    }

                    break;
                }
            }

            applet->installEventFilter(q);
        }

        void newGroupClicked(QAction *action)
        {
            kDebug()<<action->data();
            createGroup(action->data().toString(), lastClick, 0);
        }

        void deleteGroup()
        {
            int id = deleteGroupAction->data().toInt();

            foreach (AbstractGroup *group, groups) {
                if ((int)group->id() == id) {
                    group->destroy();

                    return;
                }
            }
        }

        GroupingContainment *q;
        QList<AbstractGroup *> groups;
        AbstractGroup *interestingGroup;
        QMap<AbstractGroup *, GroupHandle *> handles;
        QAction *newGroupAction;
        KMenu *newGroupMenu;
        QAction *newGridGroup;
        QAction *newFloatingGroup;
        QAction *separator;
        QAction *deleteGroupAction;
        QPointF lastClick;
};

GroupingContainment::GroupingContainment(QObject* parent, const QVariantList& args)
               : Containment(parent, args),
                 d(new GroupingContainmentPrivate(this))
{
    setContainmentType(Plasma::Containment::NoContainmentType);
}

GroupingContainment::~GroupingContainment()
{
    delete d;
}

void GroupingContainment::init()
{
    Plasma::Containment::init();

    connect(this, SIGNAL(appletAdded(Plasma::Applet *, QPointF)),
            this, SLOT(layoutApplet(Plasma::Applet *, QPointF)));

//     addGroup("grid", QPointF(100,100), 0);
}

void GroupingContainment::addGroup(const QString &plugin, const QPointF &pos, int id)
{
    d->createGroup(plugin, pos, id);
}

void GroupingContainment::addGroup(AbstractGroup *group, const QPointF &pos)
{
    if (!group) {
        return;
    }

    kDebug()<<"adding group"<<group->id();
    group->setImmutability(immutability());
    group->d->containment = this;
    connect(this, SIGNAL(immutabilityChanged(Plasma::ImmutabilityType)),
            group, SLOT(setImmutability(Plasma::ImmutabilityType)));
    connect(group, SIGNAL(groupDestroyed(AbstractGroup *)),
            this, SLOT(onGroupRemoved(AbstractGroup *)));
    connect(group, SIGNAL(configNeedsSaving()), this, SIGNAL(configNeedsSaving()));
    group->setPos(pos);

    if (containmentType() == Plasma::Containment::DesktopContainment) {
        group->installEventFilter(this);
    }

    emit groupAdded(group, pos);
}

QList<QAction *> GroupingContainment::contextualActions()
{
    QList<QAction *> list;
    list << d->newGroupAction << d->separator << d->deleteGroupAction;
    return list;
}

bool GroupingContainment::sceneEventFilter(QGraphicsItem* watched, QEvent* event)
{
    Plasma::Applet *applet = qgraphicsitem_cast<Plasma::Applet *>(watched);

    if (applet) {
        switch (event->type()) {
        case QEvent::GraphicsSceneHoverMove:
        case QEvent::GraphicsSceneHoverEnter:
            foreach (AbstractGroup *group, d->groups) {
                if (group->applets().contains(applet)) {
                    if (group->groupType() == AbstractGroup::ConstrainedGroup) {
                        return Plasma::Applet::sceneEventFilter(watched, event);
                    } else {
                        return Plasma::Containment::sceneEventFilter(watched, event);
                    }
                }
            }
            break;

        default:
            break;
        }
    }

    return Plasma::Containment::sceneEventFilter(watched, event);
}

bool GroupingContainment::eventFilter(QObject *obj, QEvent *event)
{
    AbstractGroup *group = qobject_cast<AbstractGroup *>(obj);
    Plasma::Applet *applet = qobject_cast<Plasma::Applet *>(obj);

    if (group) {
        switch (event->type()) {
            case QEvent::GraphicsSceneHoverEnter:
                if (immutability() == Plasma::Mutable && group->immutability() == Plasma::Mutable) {
                    QGraphicsSceneHoverEvent *he = static_cast<QGraphicsSceneHoverEvent *>(event);
                    if (d->handles.contains(group)) {
                        GroupHandle *handle = d->handles.value(group);
                        if (handle) {
                            handle->setHoverPos(he->pos());
                        }
                    } else {
//                         kDebug() << "generated group handle";
                        GroupHandle *handle = new GroupHandle(this, group, he->pos());
                        d->handles[group] = handle;
                        connect(handle, SIGNAL(disappearDone(GroupHandle *)),
                                this, SLOT(handleDisappeared(GroupHandle *)));
                        connect(group, SIGNAL(geometryChanged()),
                                handle, SLOT(groupResized()));
                    }
                }
                break;

            case QEvent::GraphicsSceneHoverMove:
                if (immutability() == Plasma::Mutable && group->immutability() == Plasma::Mutable) {
                    QGraphicsSceneHoverEvent *he = static_cast<QGraphicsSceneHoverEvent *>(event);
                    if (d->handles.contains(group)) {
                        GroupHandle *handle = d->handles.value(group);
                        if (handle) {
                            handle->setHoverPos(he->pos());
                        }
                    }
                }
                break;

            default:
                break;
        }
    } else if (applet) {
        switch (event->type()) {
            case QEvent::GraphicsSceneMove:
                foreach (AbstractGroup *group, d->groups) {
                    if (!group->applets().contains(applet)) {
                        QRectF rect = group->contentsRect();
                        rect.translate(group->pos());
                        if (rect.contains(applet->geometry())) {
                            group->addApplet(applet);
                            d->interestingGroup = 0;
                            break;
                        } else {
                            QRectF intersected(rect.intersected(applet->geometry()));
                            if (intersected.isValid()) {
                                group->showDropZone(mapToItem(group, intersected.center()));
                                d->interestingGroup = group;
                                break;
                            } else {
                                if (group == d->interestingGroup) {
                                    group->showDropZone(QPointF());
                                    d->interestingGroup = 0;
                                }
                            }
                        }

                    }
                }
                break;

            default:
                break;
        }
    }

    return Plasma::Containment::eventFilter(obj, event);
}

void GroupingContainment::saveContents(KConfigGroup &group) const
{
    Plasma::Containment::saveContents(group);

    KConfigGroup groupsConfig(&group, "Groups");
    foreach (AbstractGroup *group, d->groups) {
        KConfigGroup groupConfig(&groupsConfig, QString::number(group->id()));
        group->save(groupConfig);
    }

    foreach (AbstractGroup *group, d->groups) {
        foreach (Plasma::Applet *applet, group->applets()) {
            if (applet) {
                KConfigGroup appletConfig = applet->config().parent();
                KConfigGroup groupConfig(&appletConfig, QString("GroupInformation"));
                groupConfig.writeEntry("Group", group->id());
                group->saveAppletLayoutInfo(applet, groupConfig);
            }
        }
    }
}

void GroupingContainment::restoreContents(KConfigGroup& group)
{
    Plasma::Containment::restoreContents(group);

    KConfigGroup groupsConfig(&group, "Groups");
    foreach (QString groupId, groupsConfig.groupList()) {
        int id = groupId.toInt();
        KConfigGroup groupConfig(&groupsConfig, groupId);
        QRectF geometry = groupConfig.readEntry("geometry", QRectF());
        QString plugin = groupConfig.readEntry("plugin", QString());

        AbstractGroup *group = d->createGroup(plugin, geometry.topLeft(), id);
        if (group) {
            group->resize(geometry.size());
        }
    }

    KConfigGroup appletsConfig(&group, "Applets");

    foreach (Applet *applet, applets()) {
        KConfigGroup appletConfig(&appletsConfig, QString::number(applet->id()));
        KConfigGroup groupConfig(&appletConfig, "GroupInformation");

        if (groupConfig.isValid()) {
            int groupId = groupConfig.readEntry("Group", -1);

            if (groupId != -1) {
                AbstractGroup *group = 0;
                foreach (AbstractGroup *g, d->groups) {
                    if ((int)g->id() == groupId) {
                        group = g;
                        break;
                    }
                }
                if (group) {
                    group->addApplet(applet, false);
                    group->restoreAppletLayoutInfo(applet, groupConfig);
                }
            }
        }
    }
}

void GroupingContainment::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    AbstractGroup *group = qgraphicsitem_cast<AbstractGroup *>(scene()->itemAt(event->scenePos()));

    if (group) {
        d->deleteGroupAction->setVisible(true);
        d->deleteGroupAction->setData(group->id());
        showContextMenu(event->pos(), event->scenePos().toPoint());
        d->deleteGroupAction->setVisible(false);
    }

    event->ignore();

    Plasma::Containment::contextMenuEvent(event);
}

#include "groupingcontainment.moc"