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

#include "flowgroup.h"

#include <QtGui/QGraphicsLinearLayout>
#include <QtGui/QPainter>

#include <Plasma/Theme>
#include <Plasma/PaintUtils>

#include "gridhandle.h"
#include "groupingcontainment.h"
#include "spacer.h"

REGISTER_GROUP(flow, FlowGroup)

FlowGroup::FlowGroup(QGraphicsItem *parent, Qt::WindowFlags wFlags)
          : AbstractGroup(parent, wFlags),
            m_layout(new QGraphicsLinearLayout(Qt::Horizontal)),
            m_spacer(new Spacer(this))
{
    resize(200,200);

    m_layout->setSpacing(4);
    m_layout->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    m_layout->addStretch();
    setLayout(m_layout);

    m_spacer->hide();

    setGroupType(AbstractGroup::ConstrainedGroup);
    setUseSimplerBackgroundForChildren(true);
}

FlowGroup::~FlowGroup()
{

}

QString FlowGroup::pluginName() const
{
    return QString("flow");
}

void FlowGroup::restoreChildGroupInfo(QGraphicsWidget *child, const KConfigGroup &group)
{
    m_layout->insertItem(group.readEntry("Position", -1), child);
}

void FlowGroup::saveChildGroupInfo(QGraphicsWidget *child, KConfigGroup group) const
{
    int pos = 0;
    for (int i = 0; i < m_layout->count(); ++i) {
        if (child == m_layout->itemAt(i)) {
            pos = i;
            break;
        }
    }

    group.writeEntry("Position", pos);
}

bool FlowGroup::showDropZone(const QPointF &pos)
{
    if (pos == QPointF()) {
        m_layout->removeItem(m_spacer);
        m_spacer->hide();

        return false;
    }

    Plasma::FormFactor f = containment()->formFactor();
    int insertIndex = m_layout->count();

    //FIXME: needed in two places, make it a function?
    for (int i = 0; i < m_layout->count(); ++i) {
        QRectF siblingGeometry = m_layout->itemAt(i)->geometry();

        if (f == Plasma::Horizontal) {
            qreal middle = siblingGeometry.left() + (siblingGeometry.width() / 2.0);
            if (pos.x() < middle) {
                insertIndex = i;
                break;
            } else if (pos.x() <= siblingGeometry.right()) {
                insertIndex = i + 1;
                break;
            }
        } else { // Plasma::Vertical
            qreal middle = siblingGeometry.top() + (siblingGeometry.height() / 2.0);
            if (pos.y() < middle) {
                insertIndex = i;
                break;
            } else if (pos.y() <= siblingGeometry.bottom()) {
                insertIndex = i + 1;
                break;
            }
        }
    }

    m_spacerIndex = insertIndex;
    if (insertIndex != -1) {
        m_spacer->show();
        m_layout->insertItem(insertIndex, m_spacer);

        return true;
    }

    return false;
}

Handle *FlowGroup::createHandleForChild(QGraphicsWidget *child)
{
    if (!children().contains(child)) {
        return 0;
    }

    Plasma::Applet *a = qobject_cast<Plasma::Applet *>(child);
    if (a) {
        return new GridHandle(containment(), a);
    }

    return new GridHandle(containment(), static_cast<AbstractGroup *>(child));
}

void FlowGroup::layoutChild(QGraphicsWidget *child, const QPointF &)
{
    m_spacer->hide();
    m_layout->removeItem(m_spacer);
    m_layout->insertItem(m_spacerIndex, child);
}

void FlowGroup::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint) {
        Plasma::FormFactor f = containment()->formFactor();
        if (f == Plasma::Vertical) {
            m_layout->setOrientation(Qt::Vertical);
        } else {
            m_layout->setOrientation(Qt::Horizontal);
        }
    }
}

QString FlowGroup::prettyName()
{
    return i18n("Flow Group");
}

QSet<Plasma::FormFactor> FlowGroup::availableOnFormFactors()
{
    QSet<Plasma::FormFactor> set;
    set.insert(Plasma::Planar);
    set.insert(Plasma::MediaCenter);
    set.insert(Plasma::Vertical);
    set.insert(Plasma::Horizontal);

    return set;
}

#include "flowgroup.moc"
