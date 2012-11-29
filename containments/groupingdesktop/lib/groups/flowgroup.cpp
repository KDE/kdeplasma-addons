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
            m_spacer(new Spacer(this)),
            SPACING(4)
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

    m_scrollWidget->setWidget(m_container);
    m_scrollWidget->setMinimumSize(0, 0);
    m_scrollWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollWidget->setOverflowBordersVisible(false);
    m_container->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));

    m_mainLayout->addItem(m_scrollWidget);
    setLayout(m_mainLayout);

    m_spacer->hide();

    setGroupType(AbstractGroup::ConstrainedGroup);
    setUseSimplerBackgroundForChildren(true);

    connect(this, SIGNAL(appletRemovedFromGroup(Plasma::Applet*,AbstractGroup*)),
            this, SLOT(appletRemoved(Plasma::Applet*)));
    connect(this, SIGNAL(subGroupRemovedFromGroup(AbstractGroup*,AbstractGroup*)),
            this, SLOT(groupRemoved(AbstractGroup*)));
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
    QRectF rect;
    if (m_mainLayout->orientation() == Qt::Horizontal) {
        rect = QRectF(QPointF(group.readEntry("Position", 0), 0),
                      QSizeF(group.readEntry("Size", 0), m_container->size().height()));
    } else {
        rect = QRectF(QPointF(0, group.readEntry("Position", 0)),
                      QSizeF(m_container->size().width(), group.readEntry("Size", 0)));
    }
    child->setParentItem(m_container);
    child->setGeometry(rect);

    updateContents();
}

void FlowGroup::saveChildGroupInfo(QGraphicsWidget *child, KConfigGroup group) const
{
    if (m_mainLayout->orientation() == Qt::Horizontal) {
        group.writeEntry("Position", child->pos().x());
        group.writeEntry("Size", child->size().width());
    } else {
        group.writeEntry("Position", child->pos().y());
        group.writeEntry("Size", child->size().height());
    }
}

bool FlowGroup::showDropZone(const QPointF &pos)
{
    if (pos == QPointF()) {
        m_spacer->hide();
        updateContents();

        return false;
    }

    m_spacer->show();
    QGraphicsWidget *w = containment()->movingWidget();
    QSizeF pref;
    QSizeF min;
    if (children().contains(w)) {
        pref = QSizeF(w->effectiveSizeHint(Qt::PreferredSize));
        min = QSizeF(w->effectiveSizeHint(Qt::MinimumSize));
    } else {
        pref = QSizeF(40, 40);
        min = QSizeF(5, 5);
    }
    if (m_mainLayout->orientation() == Qt::Horizontal) {
        m_spacer->setPreferredWidth(pref.width());
        m_spacer->setMinimumWidth(min.width());
    } else {
        m_spacer->setPreferredHeight(pref.height());
        m_spacer->setMinimumHeight(min.height());
    }
    addItem(m_spacer, mapToItem(m_container, pos));

    return true;
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

void FlowGroup::layoutChild(QGraphicsWidget *child, const QPointF &pos)
{
    addItem(child, mapToItem(m_container, pos));
    m_spacer->hide();
    updateContents();
}

void FlowGroup::addItem(QGraphicsWidget *child, const QPointF &pos)
{
    child->setParentItem(m_container);
    QRectF rect(pos, child->effectiveSizeHint(Qt::PreferredSize));
    if (m_mainLayout->orientation() == Qt::Horizontal) {
        rect.setY(0);
    } else {
        rect.setX(0);
    }
    child->setGeometry(rect);

    updateContents();
}

void FlowGroup::appletRemoved(Plasma::Applet *)
{
    updateContents();
}

void FlowGroup::groupRemoved(AbstractGroup *)
{
    updateContents();
}

void FlowGroup::scrollPrev()
{
    QRectF geom(m_scrollWidget->viewportGeometry());
    if (m_mainLayout->orientation() == Qt::Horizontal) {
        m_scrollWidget->ensureRectVisible(QRectF(m_scrollWidget->mapToItem(m_container, geom.topLeft()) -
                                                 QPointF(40, 0), QSize(40, geom.height())));
    } else {
        m_scrollWidget->ensureRectVisible(QRectF(m_scrollWidget->mapToItem(m_container, geom.topLeft()) -
                                                 QPointF(0, 40), QSize(geom.width(), 40)));
    }
}

void FlowGroup::scrollNext()
{
    QRectF geom(m_scrollWidget->viewportGeometry());
    if (m_mainLayout->orientation() == Qt::Horizontal) {
        m_scrollWidget->ensureRectVisible(QRectF(m_scrollWidget->mapToItem(m_container, geom.topRight()),
                                                 QSize(40, geom.height())));
    } else {
        m_scrollWidget->ensureRectVisible(QRectF(m_scrollWidget->mapToItem(m_container, geom.bottomLeft()),
                                                 QSize(geom.width(), 40)));
    }
}

void FlowGroup::updateContents()
{
    const bool horizontal = (m_mainLayout->orientation() == Qt::Horizontal);

    //generate a list of the children in a left-right or top-bottom order
    QList<QGraphicsWidget *> list = children();
    if (m_spacer->isVisible()) {
        list << m_spacer;
    }
    GroupingContainment *c = containment();
    if (c) {
        list.removeOne(c->movingWidget());
    }
    QList<QGraphicsWidget *> childs;
    foreach (QGraphicsWidget *child, list) {
        for (int i = 0; i < childs.count(); ++i) {
            QGraphicsWidget *c = childs.at(i);
            if ((horizontal && child->pos().x() < c->pos().x()) ||
                (!horizontal && child->pos().y() < c->pos().y())) {
                childs.insert(i, child);
                break;
            }
        }
        if (!childs.contains(child)) {
            childs << child;
        }
    }

    QRectF containerRect(m_container->boundingRect());
    for (int i = 0; i < childs.count(); ++i) {
        QGraphicsWidget *child = childs.at(i);
        QRectF rect(child->geometry());
        rect.setWidth(child->effectiveSizeHint(Qt::PreferredSize).width());

        QGraphicsWidget *next = (i < childs.count() - 1 ? childs.at(i + 1) : 0);

        if (horizontal) {
            const qreal min = child->effectiveSizeHint(Qt::MinimumSize).width();
            const qreal minRight = childs.last()->geometry().right();
            containerRect.setRight(minRight);
            m_container->resize(containerRect.size());

            const qreal r = (next ? next->pos().x() - SPACING : containerRect.right());
            if (r < rect.right() || QSizePolicy::ExpandFlag & child->sizePolicy().horizontalPolicy()) {
                rect.setRight(r);
            }

            if (rect.width() < min) {
                qreal newR = rect.x() + min;
                if (newR > r) {
                    newR = r;
                }
                rect.setRight(newR);
            }
        } else {
            const qreal min = child->effectiveSizeHint(Qt::MinimumSize).height();
            const qreal minBottom = childs.last()->geometry().bottom();
            containerRect.setBottom(minBottom);
            m_container->resize(containerRect.size());

            const qreal b = (next ? next->pos().y() - SPACING : containerRect.bottom());
            if (b < rect.bottom() || QSizePolicy::ExpandFlag & child->sizePolicy().verticalPolicy()) {
                rect.setBottom(b);
            }

            if (rect.height() < min) {
                qreal newB = rect.y() + min;
                if (newB > b) {
                    newB = b;
                }
                rect.setBottom(newB);
            }
        }

        child->setGeometry(rect);
    }

    QRectF geom(m_scrollWidget->viewportGeometry());
    if (horizontal) {
        if (m_prevArrow->isVisible()) {
            geom.setWidth(geom.width() + m_prevArrow->geometry().width() * 2 + SPACING * 2);
        }
        m_container->resize(qMax(m_container->size().width(), geom.width()), geom.height());
        m_container->setMaximumHeight(geom.height());
        foreach (QGraphicsWidget *c, childs) {
            c->resize(c->size().width(), geom.height());
        }
    } else {
        if (m_prevArrow->isVisible()) {
            geom.setHeight(geom.height() + m_prevArrow->geometry().height() * 2 + SPACING * 2);
        }
        m_container->resize(geom.width(), qMax(m_container->size().height(), geom.height()));
        m_container->setMaximumWidth(geom.width());
        foreach (QGraphicsWidget *c, childs) {
            c->resize(geom.width(), c->size().height());
        }
    }

    QRectF r;
    if (!childs.isEmpty()) {
        r = QRectF(childs.first()->pos(), childs.last()->geometry().bottomRight());
    }
    if ((horizontal && m_container->size().width() > geom.width()) ||
        (!horizontal && m_container->size().height() > geom.height())) {
        if ((horizontal && geom.width() >= r.width()) ||
            (!horizontal && geom.height() >= r.height())) {
            r.setLeft(r.right() - geom.width());
            r.setTop(r.bottom() - geom.height());
            m_scrollWidget->ensureRectVisible(r);
            if (m_prevArrow->isVisible()) {
                m_mainLayout->removeItem(m_prevArrow);
                m_mainLayout->removeItem(m_nextArrow);
                m_prevArrow->hide();
                m_nextArrow->hide();
            }
        } else {
            if (!m_prevArrow->isVisible()) {
                m_mainLayout->insertItem(0, m_prevArrow);
                m_mainLayout->addItem(m_nextArrow);
                m_prevArrow->show();
                m_nextArrow->show();
            }
        }
    } else if (m_prevArrow->isVisible()) {
        m_mainLayout->removeItem(m_prevArrow);
        m_mainLayout->removeItem(m_nextArrow);
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
            m_spacer->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum));
            m_prevArrow->setIcon(KIcon(m_arrows->pixmap("up-arrow")));
            m_nextArrow->setIcon(KIcon(m_arrows->pixmap("down-arrow")));
            m_prevArrow->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
            m_nextArrow->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
            foreach (QGraphicsWidget *child, children()) {
                child->setPos(0, child->pos().x());
            }
        } else {
            m_mainLayout->setOrientation(Qt::Horizontal);
            m_spacer->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred));
            m_prevArrow->setIcon(KIcon(m_arrows->pixmap("left-arrow")));
            m_nextArrow->setIcon(KIcon(m_arrows->pixmap("right-arrow")));
            m_prevArrow->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
            m_nextArrow->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
            foreach (QGraphicsWidget *child, children()) {
                child->setPos(child->pos().y(), 0);
            }
        }
        updateContents();
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
