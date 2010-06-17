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

#include <QtGui/QGraphicsSceneDragDropEvent>
#include <QPainter>

#include <Plasma/PaintUtils>
#include <Plasma/Corona>

#include "groupingcontainment.h"

class Spacer : public QGraphicsWidget
{
    public:
        Spacer(QGraphicsWidget *parent)
        : QGraphicsWidget(parent),
        m_visible(true)
        {
            setAcceptDrops(true);
        }

        ~Spacer()
        {}

        StackingGroup *parent;
        bool m_visible;

    protected:
        void dropEvent(QGraphicsSceneDragDropEvent *event)
        {
            event->setPos(mapToParent(event->pos()));
            parent->dropEvent(event);
        }

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget * widget = 0)
        {
            Q_UNUSED(option)
            Q_UNUSED(widget)

            if (!m_visible) {
                return;
            }

            //TODO: make this a pretty gradient?
            painter->setRenderHint(QPainter::Antialiasing);
            QPainterPath p = Plasma::PaintUtils::roundedRectangle(contentsRect().adjusted(1, 1, -2, -2), 4);
            QColor c = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
            c.setAlphaF(0.3);

            painter->fillPath(p, c);
        }
};

StackingGroup::StackingGroup(QGraphicsItem *parent, Qt::WindowFlags wFlags)
             : AbstractGroup(parent, wFlags),
               m_spacer(new Spacer(this))
{
    resize(200,200);
    setGroupType(AbstractGroup::ConstrainedGroup);

    m_spacer->parent = this;
    m_spacer->hide();

    connect(this, SIGNAL(initCompleted()), this, SLOT(onInitCompleted()));
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
    Plasma::Corona *c = static_cast<Plasma::Corona *>(scene());
    Plasma::ImmutabilityType mut = c->immutability();
    c->setImmutability(Plasma::Mutable);
    setImmutability(Plasma::Mutable);

    int gap = 20;
    foreach (QGraphicsWidget *widget, m_children) {
        if (widget) {
            widget->resize(contentsRect().size());
            widget->setPos(gap, gap);
            widget->setZValue(gap);
            gap = gap + 20;
        }
    }
    c->setImmutability(mut);
    setImmutability(mut);
}

void StackingGroup::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    AbstractGroup::resizeEvent(event);

    drawStack();
}

void StackingGroup::onInitCompleted()
{
    connect(containment(), SIGNAL(widgetStartsMoving(QGraphicsWidget*)),
            this, SLOT(onWidgetStartsMoving(QGraphicsWidget*)));
}

void StackingGroup::onWidgetStartsMoving(QGraphicsWidget *widget)
{
    if (children().contains(widget)) {
        m_children.removeOne(widget);
    }
}

void StackingGroup::onAppletAdded(Plasma::Applet *applet, AbstractGroup *)
{
    if (!m_children.contains(applet)) {
        m_children << applet;
    }
}

void StackingGroup::onAppletRemoved(Plasma::Applet *applet, AbstractGroup *)
{
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

void StackingGroup::showDropZone(const QPointF &pos)
{
    if (m_children.contains(m_spacer)) {
        m_children.removeOne(m_spacer);
    }

    if (pos.isNull()) {
        m_spacer->hide();
        return;
    } else {
        m_spacer->show();
        m_children << m_spacer;
    }

    drawStack();
}

#include "stackinggroup.moc"
