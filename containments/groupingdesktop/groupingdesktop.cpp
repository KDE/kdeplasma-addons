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

#include <QAction>

#include <KDebug>

#include "abstractgroup.h"
#include "gridlayout.h"

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
    connect (this, SIGNAL(immutabilityChanged(Plasma::ImmutabilityType)),
             this, SLOT(onImmutabilityChanged(Plasma::ImmutabilityType)));

    addToolBoxAction(m_newGridLayout);
    newGridLayoutClicked();
}

void GroupingDesktop::newGridLayoutClicked()
{
    int id = m_groups.count(); //FIXME
    AbstractGroup *group = createGroup("grid", id);
    group->setGeometry(20,20,400,400);

    emit configNeedsSaving();
}

QList<QAction *> GroupingDesktop::contextualActions()
{
    QList<QAction *> list;
    list << m_newGridLayout;
    return list;
}

void GroupingDesktop::layoutApplet(Plasma::Applet* applet, const QPointF& pos)
{
    foreach (AbstractGroup *group, m_groups) {
        if (group->geometry().contains(pos)) {
            group->assignApplet(applet);
        }
    }
}

AbstractGroup* GroupingDesktop::createGroup(const QString& plugin, int id)
{
    AbstractGroup *group;

    if (plugin == "grid") {
        group = new GridLayout(id, this);
    } else {
        return 0;
    }

    group->setFlag(QGraphicsItem::ItemIsMovable, immutability() == Plasma::Mutable);

    m_groups.insert(id, group);

    return group;
}

void GroupingDesktop::save(KConfigGroup &group) const
{
    if (!group.isValid()) {
        group = config();
    }

    Plasma::Containment::save(group);

    KConfigGroup groupsConfig(&group, "Groups");
    foreach (AbstractGroup *group, m_groups) {
        KConfigGroup groupConfig(&groupsConfig, QString::number(m_groups.key(group)));
        groupConfig.writeEntry("Plugin", group->plugin());
        groupConfig.writeEntry("Geometry", group->geometry());
    }
}

void GroupingDesktop::saveContents(KConfigGroup& group) const
{
    Containment::saveContents(group);

    KConfigGroup appletsConfig(&group, "Applets");
    foreach (AbstractGroup *group, m_groups) {
        foreach (Plasma::Applet *applet, group->assignedApplets()) {
            KConfigGroup appletConfig(&appletsConfig, QString::number(applet->id()));
            KConfigGroup groupConfig(&appletConfig, "GroupInformation");
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
        QRectF geometry = groupConfig.readEntry("Geometry", QRectF());
        QString plugin = groupConfig.readEntry("Plugin", QString());

        AbstractGroup *group = createGroup(plugin, id);
        group->setGeometry(geometry);
    }

    KConfigGroup appletsConfig(&group, "Applets");

    foreach (Applet *applet, applets()) {
        KConfigGroup appletConfig(&appletsConfig, QString::number(applet->id()));
        KConfigGroup groupConfig(&appletConfig, "GroupInformation");

        int groupId = groupConfig.readEntry("Group", -1);

        if (groupId != -1) {
            AbstractGroup *group = m_groups.value(groupId);
            if (group) {
                group->assignApplet(applet);
                group->restoreAppletLayoutInfo(applet, groupConfig);
            }
        }
    }
}

void GroupingDesktop::onImmutabilityChanged(Plasma::ImmutabilityType immutability)
{
    bool movable = (immutability == Plasma::Mutable);
    foreach (AbstractGroup *group, m_groups) {
        group->setFlag(QGraphicsItem::ItemIsMovable, movable);
    }
}

#include "groupingdesktop.moc"