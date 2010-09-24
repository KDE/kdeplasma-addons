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

#include <KIconLoader>
#include <KIcon>

#include <Plasma/Theme>
#include <Plasma/PaintUtils>
#include <Plasma/ScrollWidget>
#include <Plasma/ToolButton>

#include "gridhandle.h"
#include "groupingcontainment.h"
#include "spacer.h"

REGISTER_GROUP(FlowGroup)

FlowGroup::FlowGroup(QGraphicsItem *parent, Qt::WindowFlags wFlags)
          : AbstractGroup(parent, wFlags),
            m_mainLayout(new QGraphicsLinearLayout(Qt::Horizontal)),
            m_arrows(new Plasma::Svg(this)),
            m_prevArrow(new Plasma::ToolButton(this)),
            m_nextArrow(new Plasma::ToolButton(this)),
            m_scrollWidget(new Plasma::ScrollWidget(this)),
            m_container(new QGraphicsWidget(this)),
            m_layout(new QGraphicsLinearLayout(Qt::Horizontal)),
            m_spacer(new Spacer(this)),
            m_spaceFiller(new QGraphicsWidget(this))
{
    resize(200,200);

    m_arrows->setImagePath("widgets/arrows");
    m_arrows->setContainsMultipleImages(true);
    m_arrows->resize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);

    m_prevArrow->setPreferredSize(IconSize(KIconLoader::Panel), IconSize(KIconLoader::Panel));
    m_prevArrow->hide();
    m_nextArrow->setPreferredSize(IconSize(KIconLoader::Panel), IconSize(KIconLoader::Panel));
    m_nextArrow->hide();

    connect(m_prevArrow, SIGNAL(pressed()), this, SLOT(scrollPrev()));
    connect(m_nextArrow, SIGNAL(pressed()), this, SLOT(scrollNext()));

    //using this widget to fill the empty space. Unfortunately it will cause 2*spacing of empty
    //space, but i don't know how to do otherwise. addStretch() isn't enough.
    m_spaceFiller->setFlag(QGraphicsItem::ItemHasNoContents);
    m_spaceFiller->setMinimumSize(QSizeF(0, 0));
    m_spaceFiller->setPreferredSize(QSizeF(0.1, 0.1)); //it doesn't like 0

    m_layout->setSpacing(4);
    m_layout->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    m_container->setLayout(m_layout);
    m_container->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));
    m_scrollWidget->setWidget(m_container);
    m_scrollWidget->setMinimumSize(0, 0);
    m_scrollWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_mainLayout->addItem(m_scrollWidget);
    setLayout(m_mainLayout);

    m_spacer->hide();
    m_spacer->setMinimumSize(20, 20);

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

    connect(this, SIGNAL(appletAddedInGroup(Plasma::Applet*,AbstractGroup*)),
            this, SLOT(updateContents()));
    connect(this, SIGNAL(subGroupAddedInGroup(AbstractGroup*,AbstractGroup*)),
            this, SLOT(updateContents()));
    connect(this, SIGNAL(appletRemovedFromGroup(Plasma::Applet*,AbstractGroup*)),
            this, SLOT(appletRemoved(Plasma::Applet*)));
    connect(this, SIGNAL(subGroupRemovedFromGroup(AbstractGroup*,AbstractGroup*)),
            this, SLOT(groupRemoved(AbstractGroup*)));
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

        updateContents();

        return false;
    }

    if (m_spacer->geometry().contains(pos)) {
        return true;
    }

    m_layout->removeItem(m_spacer);
    m_spacer->hide();

    int insertIndex = m_layout->count();
    qreal currPos = contentsRect().left();
    const qreal x = pos.x();
    const qreal y = pos.y();

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

        updateContents();

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

void FlowGroup::appletRemoved(Plasma::Applet *applet)
{
    m_layout->removeItem(applet);

    updateContents();
}

void FlowGroup::groupRemoved(AbstractGroup *group)
{
    m_layout->removeItem(group);

    updateContents();
}

void FlowGroup::scrollPrev()
{
    QRectF geom(m_scrollWidget->viewportGeometry());
    if (m_mainLayout->orientation() == Qt::Horizontal) {
        m_scrollWidget->ensureRectVisible(QRectF(m_scrollWidget->scrollPosition() -
                                                 QPointF(20, 0), QSize(20, geom.height())));
    } else {
        m_scrollWidget->ensureRectVisible(QRectF(m_scrollWidget->scrollPosition() -
                                                 QPointF(0, 20), QSize(geom.width(), 20)));
    }
}

void FlowGroup::scrollNext()
{
    QRectF geom(m_scrollWidget->viewportGeometry());
    if (m_mainLayout->orientation() == Qt::Horizontal) {
        m_scrollWidget->ensureRectVisible(QRectF(m_scrollWidget->scrollPosition() + QPointF(20, 0) +
                                                 QPointF(geom.width(), geom.height()), QSize(20, geom.height())));
    } else {
        m_scrollWidget->ensureRectVisible(QRectF(m_scrollWidget->scrollPosition() + QPointF(0, 20) +
                                                 QPointF(geom.width(), geom.height()), QSize(geom.width(), 20)));
    }
}

void FlowGroup::updateContents()
{
    m_mainLayout->removeItem(m_prevArrow);
    m_mainLayout->removeItem(m_nextArrow);
    m_mainLayout->activate();

    const bool horizontal = (m_mainLayout->orientation() == Qt::Horizontal);
    if (horizontal) {
        m_container->setMinimumHeight(m_scrollWidget->viewportGeometry().height());
    } else {
        m_container->setMinimumWidth(m_scrollWidget->viewportGeometry().width());
    }

    if ((horizontal && m_container->size().width() > m_scrollWidget->viewportGeometry().width()) ||
        (!horizontal && m_container->size().height() > m_scrollWidget->viewportGeometry().height())) {
        m_mainLayout->insertItem(0, m_prevArrow);
        m_mainLayout->addItem(m_nextArrow);
        m_prevArrow->show();
        m_nextArrow->show();
    } else {
        m_prevArrow->hide();
        m_nextArrow->hide();
    }
}

void FlowGroup::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint) {
        Plasma::FormFactor f = containment()->formFactor();
        if (f == Plasma::Vertical) {
            m_mainLayout->setOrientation(Qt::Vertical);
            m_layout->setOrientation(Qt::Vertical);
            m_spacer->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum));
            m_prevArrow->setIcon(KIcon(m_arrows->pixmap("up-arrow")));
            m_nextArrow->setIcon(KIcon(m_arrows->pixmap("down-arrow")));
            m_prevArrow->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
            m_nextArrow->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        } else {
            m_mainLayout->setOrientation(Qt::Horizontal);
            m_layout->setOrientation(Qt::Horizontal);
            m_spacer->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred));
            m_prevArrow->setIcon(KIcon(m_arrows->pixmap("left-arrow")));
            m_nextArrow->setIcon(KIcon(m_arrows->pixmap("right-arrow")));
            m_prevArrow->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
            m_nextArrow->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        }
    }
}

void FlowGroup::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    AbstractGroup::resizeEvent(event);

    updateContents();
}

GroupInfo FlowGroup::groupInfo()
{
    GroupInfo gi("flow", i18n("Flow Group"));
    QSet<Plasma::FormFactor> f;
    f << Plasma::Planar << Plasma::MediaCenter << Plasma::Horizontal << Plasma::Vertical;
    gi.setFormFactors(f);

    return gi;
}

#include "flowgroup.moc"
