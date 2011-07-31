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

#include "stackinggroup.h"

#include <QtGui/QGraphicsSceneDragDropEvent>
#include <QPainter>

#include <Plasma/PaintUtils>
#include <Plasma/Corona>

#include "groupingcontainment.h"
#include "spacer.h"

REGISTER_GROUP(StackingGroup)

StackingGroup::StackingGroup(QGraphicsItem *parent, Qt::WindowFlags wFlags)
             : AbstractGroup(parent, wFlags),
               m_spacer(new Spacer(this))
{
    resize(200,200);
    setGroupType(AbstractGroup::ConstrainedGroup);

    m_spacer->hide();

    connect(this, SIGNAL(appletRemovedFromGroup(Plasma::Applet*,AbstractGroup*)),
            this, SLOT(onAppletRemoved(Plasma::Applet*,AbstractGroup*)));
    connect(this, SIGNAL(appletAddedInGroup(Plasma::Applet*,AbstractGroup*)),
            this, SLOT(onAppletAdded(Plasma::Applet*,AbstractGroup*)));
    connect(this, SIGNAL(subGroupRemovedFromGroup(AbstractGroup*,AbstractGroup*)),
            this, SLOT(onSubGroupRemoved(AbstractGroup*,AbstractGroup*)));
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
    int from = m_children.indexOf(child);
    int to = group.readEntry("Index", 0);

    if (m_children.size() > to) {
        m_children.move(from, to);

        drawStack();
    }
}

void StackingGroup::saveChildGroupInfo(QGraphicsWidget *child, KConfigGroup group) const
{
    group.writeEntry("Index", m_children.indexOf(child));
}

void StackingGroup::layoutChild(QGraphicsWidget *child, const QPointF &)
{
    if (!m_children.contains(child)) {
        m_children << child;
    } else {
        m_children << m_children.takeAt(m_children.indexOf(child));
    }

    if (m_spacer->isVisible()) {
        m_children.removeOne(m_spacer);
        m_spacer->hide();
    }

    drawStack();
}

void StackingGroup::drawStack()
{
    int gap = 20;
    foreach (QGraphicsWidget *widget, m_children) {
        if (widget) {
            QRectF rect(QPointF(gap, gap), contentsRect().size());
            widget->setMaximumSize(rect.size());
            widget->setGeometry(rect);
            widget->setZValue(gap);
            gap = gap + 20;
        }
    }
}

void StackingGroup::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    AbstractGroup::resizeEvent(event);

    drawStack();
}

void StackingGroup::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    if (m_children.size() < 2) {
        return;
    }

    if (event->delta() > 0) {
        m_children.move(m_children.size() - 1, 0);
    } else {
        m_children << m_children.takeAt(0);
    }

    drawStack();
    saveChildren();

    event->accept();
}

void StackingGroup::releaseChild(QGraphicsWidget *child)
{
    m_children.removeOne(child);
}

void StackingGroup::onAppletAdded(Plasma::Applet *applet, AbstractGroup *)
{
    if (!m_children.contains(applet)) {
        m_children << applet;
        connect(applet, SIGNAL(activate()), this, SLOT(onAppletActivated()));
        applet->installEventFilter(this);
    }
}

void StackingGroup::onAppletRemoved(Plasma::Applet *applet, AbstractGroup *)
{
    m_children.removeOne(applet);
    applet->removeEventFilter(this);

    drawStack();
}

void StackingGroup::onSubGroupAdded(AbstractGroup *subGroup, AbstractGroup *)
{
    if (!m_children.contains(subGroup)) {
        m_children << subGroup;
        subGroup->installEventFilter(this);
    }
}

void StackingGroup::onSubGroupRemoved(AbstractGroup *subGroup, AbstractGroup *)
{
    m_children.removeOne(subGroup);
    subGroup->removeEventFilter(this);

    drawStack();
}

bool StackingGroup::showDropZone(const QPointF &pos)
{
    if (pos.isNull()) {
        m_spacer->hide();
        m_children.removeOne(m_spacer);
        return false;
    } else if (!m_spacer->isVisible()) {
        m_spacer->show();
        m_children << m_spacer;
    }

    drawStack();

    return true;
}

void StackingGroup::onAppletActivated()
{
    Plasma::Applet *applet = qobject_cast<Plasma::Applet *>(sender());

    if (applet && m_children.contains(applet)) {
        m_children << m_children.takeAt(m_children.indexOf(applet));

        drawStack();
    }
}

bool StackingGroup::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::GraphicsSceneMousePress) {
        QGraphicsWidget *widget = qobject_cast<QGraphicsWidget *>(obj);
        if (m_children.contains(widget)) {
            m_children << m_children.takeAt(m_children.indexOf(widget));

            drawStack();
        }
    }

    return AbstractGroup::eventFilter(obj, event);
}

GroupInfo StackingGroup::groupInfo()
{
    GroupInfo gi("stacking", i18n("Stacking Group"));
    gi.setIcon("object-order-raise"); //FIXME: isn't there a better one?

    return gi;
}

#include "stackinggroup.moc"
