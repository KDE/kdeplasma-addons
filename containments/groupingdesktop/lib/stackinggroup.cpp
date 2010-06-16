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

#include "stackinggroup.h"

StackingGroup::StackingGroup(QGraphicsItem *parent, Qt::WindowFlags wFlags)
             : AbstractGroup(parent, wFlags)
{
    resize(200,200);
    setGroupType(AbstractGroup::ConstrainedGroup);

    connect(this, SIGNAL(appletRemovedFromGroup(Plasma::Applet*,AbstractGroup*)),
            this, SLOT(onAppletRemoved(Plasma::Applet*,AbstractGroup*)));
    connect(this, SIGNAL(appletAddedInGroup(Plasma::Applet*,AbstractGroup*)),
            this, SLOT(onAppletAdded(Plasma::Applet*,AbstractGroup*)));
    connect(this, SIGNAL(subGroupRemovedFromGroup(AbstractGroup*,AbstractGroup*)),
            this, SLOT(onSubGroupRemoved(AbstractGroup*, AbstractGroup*)));
    connect(this, SIGNAL(subGroupAddedInGroup(AbstractGroup*,AbstractGroup*)),
            this, SLOT(onSubGroupAdded(AbstractGroup*,AbstractGroup*)));
}

StackingGroup::~StackingGroup()
{

}

QString StackingGroup::pluginName() const
{
    return QString("stacking");
}

void StackingGroup::restoreChildGroupInfo(QGraphicsWidget *child, const KConfigGroup &group)
{
    m_children.move(m_children.indexOf(child), group.readEntry("Index", 0));

    drawStack();
}

void StackingGroup::saveChildGroupInfo(QGraphicsWidget *child, KConfigGroup group) const
{
    group.writeEntry("Index", m_children.indexOf(child));
}

void StackingGroup::layoutChild(QGraphicsWidget *child, const QPointF &)
{
    if (!m_children.contains(child)) {
        m_children << child;
    }

    drawStack();
}

void StackingGroup::drawStack()
{
    int gap = 10;
    foreach (QGraphicsWidget *widget, m_children) {
        if (widget) {
            QRectF rect(QPoint(gap, gap), contentsRect().bottomRight());
            widget->setGeometry(rect);
            gap = gap + 10;
        }
    }
}

void StackingGroup::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    AbstractGroup::resizeEvent(event);

    drawStack();
}

void StackingGroup::onAppletAdded(Plasma::Applet *applet, AbstractGroup *)
{
    if (!m_children.contains(applet)) {
        m_children << applet;
    }
}

void StackingGroup::onAppletRemoved(Plasma::Applet *applet, AbstractGroup *)
{
    kDebug()<<m_children;
    m_children.removeOne(applet);

    drawStack();
}

void StackingGroup::onSubGroupAdded(AbstractGroup *subGroup, AbstractGroup *)
{
    if (!m_children.contains(subGroup)) {
        m_children << subGroup;
    }
}

void StackingGroup::onSubGroupRemoved(AbstractGroup *subGroup, AbstractGroup *)
{
    m_children.removeOne(subGroup);

    drawStack();
}

#include "stackinggroup.moc"
