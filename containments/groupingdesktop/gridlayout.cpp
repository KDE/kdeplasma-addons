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

#include "gridlayout.h"

#include <QtGui/QGraphicsLinearLayout>
#include <QtGui/QGraphicsGridLayout>
#include <QtGui/QGraphicsSceneDragDropEvent>
#include <QPainter>

#include <Plasma/Applet>
#include <Plasma/IconWidget>
#include <Plasma/PaintUtils>

#include "groupingcontainment.h"
#include "appletoverlay.h"

class Position {
    public:
        Position(int r = -1, int c = -1)
        : row(r),
        column(c)
        {
        }

        bool isValid()
        {
            return ((row >= 0) && (column >= 0));
        }

        int row;
        int column;
};

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

        GridLayout *parent;
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

GridLayout::GridLayout(QGraphicsItem *parent, Qt::WindowFlags wFlags)
          : AbstractGroup(parent, wFlags),
            m_layout(new QGraphicsGridLayout(this)),
            m_spacer(new Spacer(this)),
            m_overlay(0)
{
    resize(200,200);
    setGroupType(AbstractGroup::ConstrainedGroup);

    m_layout->setContentsMargins(10, 10, 10, 10);
    setLayout(m_layout);

    m_spacer->parent = this;
    m_spacer->setZValue(1000);
    m_spacer->hide();

    connect(this, SIGNAL(appletRemovedFromGroup(Plasma::Applet *, AbstractGroup *)),
            this, SLOT(onAppletRemoved(Plasma::Applet *, AbstractGroup *)));
    connect(this, SIGNAL(appletAddedInGroup(Plasma::Applet*,AbstractGroup*)),
            this, SLOT(onAppletAdded(Plasma::Applet *, AbstractGroup *)));
}

GridLayout::~GridLayout()
{

}

void GridLayout::onAppletAdded(Plasma::Applet* applet, AbstractGroup* group)
{
    Q_UNUSED(group)

    applet->installSceneEventFilter(this);
}

void GridLayout::onAppletRemoved(Plasma::Applet *applet, AbstractGroup *group)
{
    Q_UNUSED(group)

    if (m_overlay && applet == m_overlay->applet()) {
        removeItem(m_spacer);
        delete m_overlay;
        m_spacer->hide();
        m_overlay = 0;
    } else {
        removeItem(applet);
    }
}

QString GridLayout::pluginName() const
{
    return QString("grid");
}

void GridLayout::showDropZone(const QPointF &pos)
{
    if (pos.isNull()) {
        m_spacer->hide();
        removeItem(m_spacer);

        return;
    }

    const qreal x = pos.x();
    const qreal y = pos.y();

    if (!contentsRect().contains(pos)) {
        m_spacer->hide();
        removeItem(m_spacer);
    }

    if ((m_spacer->isVisible()) && (m_spacer->geometry().contains(pos))) {
        return;
    }

    Position itemPos = itemPosition(m_spacer);

    if ((itemPos.row != -1) && (itemPos.column != -1)) {
        removeItemAt(itemPos, true);
        m_spacer->hide();
    }

    const int rows = m_layout->rowCount();
    const int columns = m_layout->columnCount();

    if (columns == 0) {
        insertItemAt(m_spacer, 0, 0, Horizontal);
        return;
    }

    const qreal rowHeight = boundingRect().height() / rows;
    const qreal columnWidth = boundingRect().width() / columns;

    const int i = x / columnWidth;
    const int j = y / rowHeight;

    int n;
    if ((n = nearestBoundair(x, columnWidth)) != -1) {
        insertItemAt(m_spacer, j, n, Horizontal);
        return;
    }
    if ((n = nearestBoundair(y, rowHeight)) != -1) {
        insertItemAt(m_spacer, n, i, Vertical);
        return;
    }
}

QGraphicsLayoutItem* GridLayout::removeItemAt(Position position, bool fillLayout)
{
    return removeItemAt(position.row, position.column, fillLayout);
}

void GridLayout::removeItem(QGraphicsWidget *item, bool fillLayout)
{
    Position pos = itemPosition(item);
    if (!pos.isValid()) {
        kDebug()<<"The item"<<item<<"is not in the layout!";
        return;
    }

    removeItemAt(pos, fillLayout);
}

QGraphicsLayoutItem *GridLayout::removeItemAt(int row, int column, bool fillLayout)
{
    QGraphicsLayoutItem *item = m_layout->itemAt(row, column);
    for (int i = 0; i < m_layout->count(); ++i) {
        if (item == m_layout->itemAt(i)) {
            m_layout->removeAt(i);
            if (fillLayout) {
                if (m_layout->columnCount() > column) {
                    QGraphicsLayoutItem *movingWidget = removeItemAt(row, column + 1);
                    if (movingWidget) {
                        m_layout->addItem(movingWidget, row, column);
                    }
                }
                if (m_layout->columnCount() > row) {
                    QGraphicsLayoutItem *movingWidget = removeItemAt(row + 1, column);
                    if (movingWidget) {
                        m_layout->addItem(movingWidget, row, column);
                    }
                }
            }

            return item;
        }
    }

    return 0;
}

void GridLayout::insertItemAt(QGraphicsWidget *item, int row, int column, Orientation orientation)
{
    if (!item) {
        return;
    }

    const int rows = m_layout->rowCount();
    const int columns = m_layout->columnCount();

    item->show();
    if ((rows > row) && (columns > column) && m_layout->itemAt(row, column)) {
        if (orientation == Horizontal) {
            for (int i = columns - 1; i >= column; --i) {
                QGraphicsLayoutItem *nextItem = removeItemAt(row, i, false);
                if (nextItem) {
                    m_layout->addItem(nextItem, row, i + 1);
                }
            }
        } else {
            for (int i = rows - 1; i >= row; --i) {
                QGraphicsLayoutItem *nextItem = removeItemAt(i, column, false);
                if (nextItem) {
                    m_layout->addItem(nextItem, i + 1, column);
                }
            }
        }
    }

    m_layout->addItem(item, row, column);
    m_layout->activate();
}

Position GridLayout::itemPosition(QGraphicsItem *item) const
{
    for (int i = 0; i < m_layout->rowCount(); ++i) {
        for (int j = 0; j < m_layout->columnCount(); ++j) {
            QGraphicsLayoutItem *layoutItem = m_layout->itemAt(i, j);
            if (layoutItem && layoutItem->graphicsItem() == item) {
                return Position(i, j);
            }
        }
    }

    return Position(-1, -1);
}

void GridLayout::layoutApplet(Plasma::Applet *applet, const QPointF &pos)
{
    Q_UNUSED(pos)

    Position spacerPos = itemPosition(m_spacer);
    if ((spacerPos.row != -1) && (spacerPos.column != -1)) {
        m_spacer->hide();
        removeItemAt(spacerPos, false);
        insertItemAt(applet, spacerPos.row, spacerPos.column, Horizontal);
    }
}

int GridLayout::nearestBoundair(qreal pos, qreal size) const
{
    const int gap = size / 3;

    int x = pos / size;
    qreal n = pos / size;
    while (n > 1) { //equivalent of "point % size" that won't work
        --n;         //because they are qreal
    }
    if (n * size > size / 2) {
        ++x;
    }

    const qreal y = x * size;
    if (((pos < y) && (pos > y - gap)) || ((pos > y) && (pos < y + gap))) {
        return x;
    }

    return -1;
}

void GridLayout::overlayStartsMoving()
{
    Position pos = itemPosition(m_overlay->applet());

    if (pos.isValid()) {
        int z = 0;
        for (int i = 0; i < m_layout->count(); ++i) {
            QGraphicsItem *item = m_layout->itemAt(i)->graphicsItem();
            if (item && (item->zValue() > z)) {
                z = item->zValue() + 1;
            }
        }
        m_overlay->setZ(z);

        removeItemAt(pos, false);
        m_layout->addItem(m_spacer, pos.row, pos.column);
        m_spacer->show();
    }
}

void GridLayout::overlayMoving(qreal x, qreal y)
{
    Plasma::Applet *applet = m_overlay->applet();
    QPointF newPos(applet->pos() + QPointF(x, y));
    QRectF newRect(applet->geometry().translated(x, y));

    if (contentsRect().contains(newRect)) {
        applet->moveBy(x, y);
    } else {
        if (newPos.x() < contentsRect().topLeft().x()) {
            if (x > 0) {
                applet->moveBy(x, 0);
            }
        } else if (newRect.topRight().x() > contentsRect().topRight().x()) {
            if (x < 0) {
                applet->moveBy(x, 0);
            }
        } else {
            applet->moveBy(x, 0);
        }
        if (newPos.y() < contentsRect().topLeft().y()) {
            if (y > 0) {
                applet->moveBy(0, y);
            }
        } else if (newRect.bottomRight().y() > contentsRect().bottomRight().y()) {
            if (y < 0) {
                applet->moveBy(0, y);
            }
        } else {
            applet->moveBy(0, y);
        }
    }

    foreach (Plasma::Applet *applet, applets()) {
        if (applet->geometry().contains(newRect.center())) {
            Position appletPos = itemPosition(applet);
            Position spacerPos = itemPosition(m_spacer);

            if (appletPos.isValid() && spacerPos.isValid()) {
                removeItemAt(appletPos, false);
                removeItemAt(spacerPos, false);

                m_layout->addItem(applet, spacerPos.row, spacerPos.column);
                m_layout->addItem(m_spacer, appletPos.row, appletPos.column);
            }
        }
    }
}

void GridLayout::overlayEndsMoving()
{
    Position pos = itemPosition(m_spacer);
    if (pos.isValid()) {
        removeItemAt(pos, false);
        m_layout->addItem(m_overlay->applet(), pos.row, pos.column);
    }

    delete m_overlay;
    m_overlay = 0;
    m_spacer->hide();
}

void GridLayout::onAppletMovedOutside(qreal x, qreal y)
{
    m_overlay->applet()->moveBy(x, y);
    removeApplet(m_overlay->applet());
}

void GridLayout::saveAppletLayoutInfo(Plasma::Applet *applet, KConfigGroup group) const
{
    Position pos = itemPosition(applet);
    group.writeEntry("Row", pos.row);
    group.writeEntry("Column", pos.column);
}

void GridLayout::restoreAppletLayoutInfo(Plasma::Applet *applet, const KConfigGroup &group)
{
    int row = group.readEntry("Row", -1);
    int column = group.readEntry("Column", -1);

    m_layout->addItem(applet, row, column);
}

bool GridLayout::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
    if (immutability() != Plasma::Mutable) {
        return AbstractGroup::sceneEventFilter(watched, event);
    }

    Plasma::Applet *applet = qgraphicsitem_cast<Plasma::Applet *>(watched);

    if (applet && applets().contains(applet)) {
        switch (event->type()) {
        case QEvent::GraphicsSceneHoverMove:
        case QEvent::GraphicsSceneHoverEnter:
            {
                if (!m_overlay) {
                    m_overlay = new AppletOverlay(applet);
                    connect(m_overlay, SIGNAL(startMoving()), this, SLOT(overlayStartsMoving()));
                    connect(m_overlay, SIGNAL(endMoving()), this, SLOT(overlayEndsMoving()));
                    connect(m_overlay, SIGNAL(movedOf(qreal, qreal)),
                            this, SLOT(overlayMoving(qreal, qreal)));
                    connect(m_overlay, SIGNAL(appletMovedOutside(qreal, qreal)),
                            this, SLOT(onAppletMovedOutside(qreal, qreal)));
                }
            }
            break;

        case QEvent::GraphicsSceneHoverLeave:
            if (m_overlay && !m_overlay->isMoving()) {
                delete m_overlay;
                m_overlay = 0;
            }

            break;

        default:
            break;
        }
    }

    return AbstractGroup::sceneEventFilter(watched, event);
}

#include "gridlayout.moc"
