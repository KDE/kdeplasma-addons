/******************************************************************************
*                                   Container                                 *
*******************************************************************************
*                                                                             *
*        Copyright (C) 2009 Giulio Camuffo <giuliocamuffo@gmail.com>          *
*                                                                             *
*   This program is free software; you can redistribute it and/or modify      *
*   it under the terms of the GNU General Public License as published by      *
*   the Free Software Foundation; either version 2 of the License, or         *
*   (at your option) any later version.                                       *
*                                                                             *
*   This program is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
*   GNU General Public License for more details.                              *
*                                                                             *
*   You should have received a copy of the GNU General Public License along   *
*   with this program; if not, write to the Free Software Foundation, Inc.,   *
*   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA                *
*                                                                             *
******************************************************************************/

#include "groupingdesktop.h"

#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsSceneContextMenuEvent>
#include <QAction>

#include <KDebug>
#include <KMenu>

#include "abstractgroup.h"

#include "gridlayout.h"

K_EXPORT_PLASMA_APPLET(groupingdesktop, GroupingDesktop)

class GroupingDesktopPrivate
{
    public:
        GroupingDesktopPrivate(GroupingDesktop *containment)
            : q(containment)
        {
            newGroupAction = new QAction(i18n("Add a new group"), q);
            newGroupMenu = new KMenu(i18n("Add a new group"), 0);
            newGroupAction->setMenu(newGroupMenu);
            newGridGroup = new QAction(i18n("Add a new grid group"), q);
            newGroupMenu->addAction(newGridGroup);

            removeGroup = new QAction(i18n("Remove this group"), q);

            separator = new QAction(q);
            separator->setSeparator(true);
        }

        ~GroupingDesktopPrivate()
        {}

        AbstractGroup *createGroup(const QString &plugin, const QPointF &pos, unsigned int id)
        {
            AbstractGroup *group = 0;
            if (plugin == "gridlayout") {
                group = new GridLayout(q);
            }

            if (!group) {
                return 0;
            }

            if (id == 0) {
                id = groups.count() + 1;
            }
            group->setId(id);
            group->setImmutability(q->immutability());
            group->setContainment(q);

            if (groups.contains(group)) {
                delete group;
                return 0;
            }

            groups << group;

            q->onGroupAdded(group, pos);
            return group;
        }

        GroupingDesktop *q;
        QList<AbstractGroup *> groups;
        QAction *newGroupAction;
        KMenu *newGroupMenu;
        QAction *newGridGroup;
        QAction *removeGroup;
        QAction *separator;
        QPointF lastClick;
};

GroupingDesktop::GroupingDesktop(QObject* parent, const QVariantList& args)
               : Containment(parent, args),
                 d(new GroupingDesktopPrivate(this))
{
    setContainmentType(Plasma::Containment::DesktopContainment);

    connect(d->newGridGroup, SIGNAL(triggered()),
            this, SLOT(newGridLayoutClicked()));
    connect(d->removeGroup, SIGNAL(triggered()),
            this, SLOT(removeGroupClicked()));
}

GroupingDesktop::~GroupingDesktop()
{
    delete d;
}

void GroupingDesktop::init()
{
    Plasma::Containment::init();

    connect(this, SIGNAL(appletAdded(Plasma::Applet *, QPointF)),
            this, SLOT(layoutApplet(Plasma::Applet *, QPointF)));

    addGroup("gridlayout", QPointF(20,20), 0);
}

void GroupingDesktop::layoutApplet(Plasma::Applet *applet, const QPointF &pos)
{
    foreach (AbstractGroup *group, d->groups) {
        if (group && group->geometry().contains(pos)) {
            group->assignApplet(applet);

            emit configNeedsSaving();
        }
    }
}

void GroupingDesktop::addGroup(const QString &plugin, const QPointF &pos, int id)
{
    d->createGroup(plugin, pos, id);
}

void GroupingDesktop::newGridLayoutClicked()
{
    addGroup("gridlayout", d->lastClick, 0);
}

void GroupingDesktop::removeGroupClicked()
{
    int id = d->removeGroup->data().toInt();

    foreach (AbstractGroup *group, d->groups) {
        if ((int)group->id() == id) {
            kDebug()<<id;
            group->destroy();
            break;
        }
    }
}

void GroupingDesktop::onGroupAdded(AbstractGroup* group, const QPointF &pos)
{
    if (!group) {
        return;
    }

    group->setPos(pos);

    connect(group, SIGNAL(groupRemoved(AbstractGroup *)),
            this, SLOT(onGroupRemoved(AbstractGroup *)));
}

void GroupingDesktop::onGroupRemoved(AbstractGroup *group)
{
    kDebug()<<"Removed group"<<group->id();

    foreach (AbstractGroup *g, d->groups) {
        if (g == group) {
            d->groups.removeAll(group);
            break;
        }
    }
}

QList< QAction* > GroupingDesktop::contextualActions()
{
    d->removeGroup->setVisible(false);
    foreach (AbstractGroup *group, d->groups) {
        if (group && group->geometry().contains(d->lastClick)) {
            kDebug()<<"LKM";
            d->removeGroup->setVisible(true);
            d->removeGroup->setData(group->id());
            break;
        }
    }

    QList<QAction *> list;
    list << d->newGroupAction << d->removeGroup << d->separator;
    return list;
}

void GroupingDesktop::saveContents(KConfigGroup &group) const
{
    Plasma::Containment::saveContents(group);

    KConfigGroup groupsConfig(&group, "Groups");
    foreach (AbstractGroup *group, d->groups) {
        KConfigGroup groupConfig(&groupsConfig, QString::number(group->id()));
        group->save(groupConfig);
    }

    foreach (AbstractGroup *group, d->groups) {
        foreach (Plasma::Applet *applet, group->assignedApplets()) {
            if (applet) {
                KConfigGroup appletConfig = applet->config().parent();
                KConfigGroup groupConfig(&appletConfig, QString("GroupInformation"));
                groupConfig.writeEntry("Group", group->id());
                group->saveAppletLayoutInfo(applet, groupConfig);
            }
        }
    }
}

void GroupingDesktop::restoreContents(KConfigGroup& group)
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
                    group->assignApplet(applet, false);
                    group->restoreAppletLayoutInfo(applet, groupConfig);
                }
            }
        }
    }
}

// void GroupingDesktop::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
// {
//     d->lastClick = event->pos();
//     kDebug()<<d->lastClick;

//     Plasma::Containment::contextMenuEvent(event);
// }

#include "groupingdesktop.moc"