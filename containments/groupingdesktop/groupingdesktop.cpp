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

    newGridLayoutClicked();
}

void GroupingDesktop::newGridLayoutClicked()
{
    int id = m_gridLayouts.count(); //FIXME
    GridLayout *gridLayout = new GridLayout(this);
    gridLayout->setGeometry(20,20,200,200);
    gridLayout->setFlag(QGraphicsItem::ItemIsMovable, immutability() == Plasma::Mutable);

    m_gridLayouts.insert(id, gridLayout);

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
    foreach (GridLayout *gl, m_gridLayouts) {
        if (gl->geometry().contains(pos)) {
            gl->assignApplet(applet, pos);
        }
    }
}

void GroupingDesktop::save(KConfigGroup &group) const
{
    if (!group.isValid()) {
        group = config();
    }

    Plasma::Containment::save(group);

    KConfigGroup groupsConfig(&group, "Groups");
    foreach (GridLayout *gridLayout, m_gridLayouts) {
        KConfigGroup groupConfig(&groupsConfig, QString::number(m_gridLayouts.key(gridLayout)));
        groupConfig.writeEntry("Position", gridLayout->pos());
    }
}

void GroupingDesktop::saveContents(KConfigGroup& group) const
{
    Containment::saveContents(group);

    KConfigGroup appletsConfig(&group, "Applets");
    foreach (GridLayout *gridLayout, m_gridLayouts) {
        foreach (Plasma::Applet *applet, gridLayout->assignedApplets()) {
            KConfigGroup appletConfig(&appletsConfig, QString::number(applet->id()));
            KConfigGroup layoutConfig(&appletConfig, "GroupInformation");
            Position pos = gridLayout->itemPosition(applet);
            layoutConfig.writeEntry("Group", m_gridLayouts.key(gridLayout));
            layoutConfig.writeEntry("Column", pos.column);
            layoutConfig.writeEntry("Order", pos.row);
        }
    }
}

void GroupingDesktop::restore(KConfigGroup& group)
{
    kDebug()<<"Lk";
    Containment::restore(group);

    KConfigGroup groupsConfig(&group, "Groups");
    foreach (QString groupId, groupsConfig.groupList()) {
        int id = groupId.toInt();
        KConfigGroup groupConfig(&groupsConfig, groupId);
        QPointF pos = groupConfig.readEntry("Position", QPointF());

        GridLayout *gridLayout = new GridLayout(this);
        gridLayout->setPos(pos);
        m_gridLayouts.insert(id, gridLayout);
    }

    KConfigGroup appletsConfig(&group, "Applets");

    foreach (Applet *applet, applets()) {
        KConfigGroup appletConfig(&appletsConfig, QString::number(applet->id()));
        KConfigGroup layoutConfig(&appletConfig, "GroupInformation");

        int group = layoutConfig.readEntry("Group", -1);
        int column = layoutConfig.readEntry("Column", 0);
        int row = layoutConfig.readEntry("Row", 0);

        m_gridLayouts.value(group)->assignApplet(applet, row, column);
    }
}

void GroupingDesktop::onImmutabilityChanged(Plasma::ImmutabilityType immutability)
{
    bool movable = (immutability == Plasma::Mutable);
    foreach (GridLayout *gl, m_gridLayouts) {
        gl->setFlag(QGraphicsItem::ItemIsMovable, movable);
    }
}

#include "groupingdesktop.moc"