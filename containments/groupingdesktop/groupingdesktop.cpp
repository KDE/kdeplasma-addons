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

#include <QtGui/QGraphicsSceneContextMenuEvent>
#include <QAction>

#include <KDebug>

#include "abstractgroup.h"

K_EXPORT_PLASMA_APPLET(groupingdesktop, GroupingDesktop)

GroupingDesktop::GroupingDesktop(QObject* parent, const QVariantList& args)
               : Containment(parent, args)
{
    setContainmentType(Plasma::Containment::DesktopContainment);
}

GroupingDesktop::~GroupingDesktop()
{

}

void GroupingDesktop::init()
{
    Plasma::Containment::init();

    connect(this, SIGNAL(appletAdded(Plasma::Applet*, QPointF)),
            this, SLOT(layoutApplet(Plasma::Applet*, QPointF)));

    m_newGridLayout = new QAction(this);
    m_newGridLayout->setText(i18n("Add a new grid layout"));
    connect (m_newGridLayout, SIGNAL(triggered()),
             this, SLOT(newGridLayoutClicked()));

    addToolBoxAction(m_newGridLayout);
    newGridLayoutClicked();
}

void GroupingDesktop::newGridLayoutClicked()
{
    AbstractGroup *group = createGroup("gridlayout", 0);
    if (group) {
        group->setGeometry(20,20,400,400);

        emit configNeedsSaving();
    }
}

QList<QAction *> GroupingDesktop::contextualActions()
{
    QList<QAction *> list;
    list << m_newGridLayout;
    return list;
}

void GroupingDesktop::layoutApplet(Plasma::Applet *applet, const QPointF &pos)
{
    foreach (AbstractGroup *group, m_groups) {
        if (group->geometry().contains(pos)) {
            group->assignApplet(applet);

            emit configNeedsSaving();
        }
    }
}

AbstractGroup *GroupingDesktop::createGroup(const QString &plugin, int id)
{

    Plasma::Applet *applet = Plasma::Applet::load(plugin, id);
    AbstractGroup *group = dynamic_cast<AbstractGroup *>(applet);

    if (!group) {
        delete applet;
        return 0;
    }

    group->setParent(this);
    group->setParentItem(this);
    group->init();
    group->updateConstraints(Plasma::StartupCompletedConstraint);
    group->flushPendingConstraintsEvents();
    group->updateConstraints(Plasma::AllConstraints);
    group->flushPendingConstraintsEvents();
    group->setZValue(-100000000); //maybe FIXME ?
    group->setImmutability(Plasma::Mutable);

    connect(group, SIGNAL(configNeedsSaving()), this, SIGNAL(configNeedsSaving()));
    connect(group, SIGNAL(appletDestroyed(Plasma::Applet*)), this, SLOT(groupDestroyed(Plasma::Applet*)));

    m_groups.insert(id, group);

    return group;
}

void GroupingDesktop::saveContents(KConfigGroup &group) const
{
    Containment::saveContents(group);

    kDebug()<<group.accessMode();
kDebug()<<"jklN===============================================================";
    KConfigGroup appletsConfig(&group, "Applets");
    foreach (AbstractGroup *group, m_groups) {
        foreach (Plasma::Applet *applet, group->assignedApplets()) {
            kDebug()<<QString::number(applet->id());
            KConfigGroup appletConfig(&appletsConfig, QString::number(applet->id()));
            appletConfig.writeEntry("aa", "dd");
            KConfigGroup groupConfig(&appletConfig, QString("GroupInformation"));
            kDebug()<<appletConfig.isImmutable()<<group->id();
            groupConfig.writeEntry("Group", group->id());
            group->saveAppletLayoutInfo(applet, groupConfig);
        }
    }
}

void GroupingDesktop::restore(KConfigGroup& group)
{
    Containment::restore(group);

    KConfigGroup groupsConfig(&group, "Groups");
    foreach (QString groupId, groupsConfig.groupList()) {
        int id = groupId.toInt();
        KConfigGroup groupConfig(&groupsConfig, groupId);
        QRectF geometry = groupConfig.readEntry("geometry", QRectF());
        QString plugin = groupConfig.readEntry("plugin", QString());

        AbstractGroup *group = createGroup(plugin, id);
        if (group) {
            group->setGeometry(geometry);
        }
    }

    KConfigGroup appletsConfig(&group, "Applets");

    foreach (Applet *applet, applets()) {
        KConfigGroup appletConfig(&appletsConfig, QString::number(applet->id()));
        KConfigGroup groupConfig(&appletConfig, "GroupInformation");

        if (groupConfig.isValid()) {
            int groupId = groupConfig.readEntry("Group", -1);

            if (groupId != -1) {
                AbstractGroup *group = m_groups.value(groupId);
                if (group) {
                    kDebug()<<group->id();
                    group->assignApplet(applet, false);
                    group->restoreAppletLayoutInfo(applet, groupConfig);
                }
            }
        }
    }
}

void GroupingDesktop::groupDestroyed(Plasma::Applet *group)
{
    int id = group->id();

//     m_groups.remove(id);
}

#include "groupingdesktop.moc"