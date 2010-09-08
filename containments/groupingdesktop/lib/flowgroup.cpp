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
            m_spacer(new Spacer(this)),
            m_spaceFiller(new QGraphicsWidget(this))
{
    resize(200,200);

    //using this widget to fill the empty space. Unfortunately it will cause 2*spacing of empty
    //space, but i don't know how to do otherwise. addStretch() isn't enough.
    m_spaceFiller->setMinimumSize(QSizeF(0, 0));
    m_spaceFiller->setPreferredSize(QSizeF(0.1, 0.1)); //it doesn't like 0

    m_layout->setSpacing(4);
    m_layout->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    setLayout(m_layout);

    m_spacer->hide();

    setGroupType(AbstractGroup::ConstrainedGroup);
    setUseSimplerBackgroundForChildren(true);
}

FlowGroup::~FlowGroup()
{

}

void FlowGroup::init()
{
    int stretchIndex = config().readEntry("StretchIndex", 0);
    m_layout->insertItem(stretchIndex, m_spaceFiller);
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

void FlowGroup::releaseChild(QGraphicsWidget *child)
{
    m_layout->removeItem(child);
}

bool FlowGroup::showDropZone(const QPointF &pos)
{
    if (pos == QPointF()) {
        m_layout->removeItem(m_spacer);
        m_spacer->hide();

        return false;
    }

    int insertIndex = m_layout->count();
    qreal currPos = contentsRect().left();
    const qreal x = pos.x();
    const qreal y = pos.y();

    //FIXME: needed in two places, make it a function?
    for (int i = 0; i < m_layout->count(); ++i) {
        QRectF siblingGeometry = m_layout->itemAt(i)->geometry();

        if (m_layout->orientation() == Qt::Horizontal) {
            siblingGeometry.moveTopLeft(QPointF(currPos, 0));
            const qreal middle = siblingGeometry.left() + (siblingGeometry.width() / 2.0);
            if (x <= middle && x > siblingGeometry.left() - 2) {
                insertIndex = i;
                break;
            } else if (x >= middle && x <= siblingGeometry.right() + 2) {
                insertIndex = i + 1;
                break;
            }

            currPos += siblingGeometry.width() + 4;
        } else { // Vertical
            siblingGeometry.moveTopLeft(QPointF(0, currPos));
            const qreal middle = siblingGeometry.top() + (siblingGeometry.height() / 2.0);
            if (y <= middle && y > siblingGeometry.top() - 2) {
                insertIndex = i;
                break;
            } else if (y >= middle && y <= siblingGeometry.bottom() + 2) {
                insertIndex = i + 1;
                break;
            }

            currPos += siblingGeometry.height() + 4;
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

    m_layout->activate();

    for (int i = 0; i < m_layout->count(); ++i) {
        if (m_layout->itemAt(i) == m_spaceFiller) {
            config().writeEntry("StretchIndex", i);
            emit configNeedsSaving();

            return;
        }
    }
}

void FlowGroup::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint) {
        Plasma::FormFactor f = containment()->formFactor();
        if (f == Plasma::Vertical) {
            m_layout->setOrientation(Qt::Vertical);
            m_spacer->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum));
        } else {
            m_layout->setOrientation(Qt::Horizontal);
            m_spacer->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred));
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
