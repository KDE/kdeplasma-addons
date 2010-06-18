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

#include "gridgroup.h"

#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsLinearLayout>
#include <QtGui/QGraphicsGridLayout>
#include <QtGui/QGraphicsSceneDragDropEvent>
#include <QPainter>

#include <Plasma/Applet>
#include <Plasma/IconWidget>
#include <Plasma/PaintUtils>

#include "groupingcontainment.h"

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

        GridGroup *parent;
        bool m_visible;
        GridGroup::Orientation lastOrientation;

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

GridGroup::GridGroup(QGraphicsItem *parent, Qt::WindowFlags wFlags)
          : AbstractGroup(parent, wFlags),
            m_layout(new QGraphicsGridLayout(this)),
            m_spacer(new Spacer(this))
{
    resize(200,200);
    setMinimumSize(100,50);
    setGroupType(AbstractGroup::ConstrainedGroup);

    m_layout->setContentsMargins(10, 10, 10, 10);
    m_layout->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));
    setLayout(m_layout);

    m_spacer->parent = this;
    m_spacer->setZValue(1000);
    m_spacer->hide();

    connect(this, SIGNAL(initCompleted()), this, SLOT(onInitCompleted()));
    connect(this, SIGNAL(appletRemovedFromGroup(Plasma::Applet*,AbstractGroup*)),
            this, SLOT(onAppletRemoved(Plasma::Applet*,AbstractGroup*)));
    connect(this, SIGNAL(subGroupRemovedFromGroup(AbstractGroup*,AbstractGroup*)),
            this, SLOT(onSubGroupRemoved(AbstractGroup*, AbstractGroup*)));
}

GridGroup::~GridGroup()
{

}

void GridGroup::onInitCompleted()
{
    connect(containment(), SIGNAL(widgetStartsMoving(QGraphicsWidget*)),
            this, SLOT(onWidgetStartsMoving(QGraphicsWidget*)));
}

void GridGroup::onWidgetStartsMoving(QGraphicsWidget *widget)
{
    if (children().contains(widget)) {
        removeItem(widget);
    }
}

void GridGroup::onAppletRemoved(Plasma::Applet *applet, AbstractGroup *group)
{
    Q_UNUSED(group)

    removeItem(applet);
}

void GridGroup::onSubGroupRemoved(AbstractGroup *subGroup, AbstractGroup *group)
{
    Q_UNUSED(group)

    removeItem(subGroup);
}

QString GridGroup::pluginName() const
{
    return QString("grid");
}

void GridGroup::showDropZone(const QPointF &pos)
{
    if (pos.isNull() || !contentsRect().contains(pos)) {
        if (m_spacer->isVisible()) {
            m_spacer->hide();
            removeItem(m_spacer);
        }

        return;
    }

    if ((m_spacer->isVisible()) && (m_spacer->geometry().contains(pos))) {
        return;
    }

    Position itemPos = itemPosition(m_spacer);
    if (itemPos.isValid()) {
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

    const qreal x = pos.x();
    const qreal y = pos.y();

    const int i = x / columnWidth;
    const int j = y / rowHeight;

    int n;
    if ((n = nearestBoundair(x, columnWidth)) != -1) {
        if (itemPosition(m_spacer).isValid()) {
            removeItem(m_spacer);
        }
        m_spacer->lastOrientation = Horizontal;
        insertItemAt(m_spacer, j, n, Horizontal);
    } else if ((n = nearestBoundair(y, rowHeight)) != -1) {
        if (itemPosition(m_spacer).isValid()) {
            removeItem(m_spacer);
        }
        m_spacer->lastOrientation = Vertical;
        insertItemAt(m_spacer, n, i, Vertical);
    }
}

QGraphicsLayoutItem *GridGroup::removeItemAt(const Position &position, bool fillLayout)
{
    return removeItemAt(position.row, position.column, fillLayout);
}

void GridGroup::removeItem(QGraphicsWidget *item, bool fillLayout)
{
    Position pos = itemPosition(item);
    if (!pos.isValid()) {
        kDebug()<<"The item"<<item<<"is not in the layout!";
        return;
    }

    removeItemAt(pos, fillLayout);
}

QGraphicsLayoutItem *GridGroup::removeItemAt(int row, int column, bool fillLayout)
{
    QGraphicsLayoutItem *item = m_layout->itemAt(row, column);
    int index = -1;
    for (int i = 0; i < m_layout->count(); ++i) {
        if (item == m_layout->itemAt(i)) {
            index = i;
            break;
        }
    }

    if (index > -1) {
        m_layout->removeAt(index);
        if (fillLayout) {
            if (m_layout->columnCount() > column + 1) {
                QGraphicsLayoutItem *movingWidget = removeItemAt(row, column + 1);
                if (movingWidget) {
                    m_layout->addItem(movingWidget, row, column);
                }
            }
            if (m_layout->rowCount() > row + 1) {
                QGraphicsLayoutItem *movingWidget = removeItemAt(row + 1, column);
                if (movingWidget) {
                    m_layout->addItem(movingWidget, row, column);
                }
            }
        }
        return item;
    }

    return 0;
}

void GridGroup::insertItemAt(QGraphicsWidget *item, int row, int column, Orientation orientation)
{
    if (!item) {
        return;
    }

    if (row < 0 || column < 0) {
        kDebug()<<"Warning: row ="<<row<<", column ="<<column;
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
}

Position GridGroup::itemPosition(QGraphicsItem *item) const
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

void GridGroup::layoutChild(QGraphicsWidget *child, const QPointF &pos)
{
    if (!m_spacer->isVisible()) {
        showDropZone(pos);
    }

    Position spacerPos = itemPosition(m_spacer);
    if ((spacerPos.row != -1) && (spacerPos.column != -1)) {
        m_spacer->hide();
        removeItemAt(spacerPos, false);
        insertItemAt(child, spacerPos.row, spacerPos.column, Horizontal);
    }
}

int GridGroup::nearestBoundair(qreal pos, qreal size) const
{
    const qreal gap = size / 3.0;

    int x = pos / size;
    qreal n = pos / size;
    while (n > 1) { //equivalent of "pos % size" that won't work
        --n;        //because they are qreal
    }

    if (n * size > size / 2.0) {
        ++x;
    }

    const qreal y = x * size;
    if (((pos < y) && (pos > y - gap)) || ((pos > y) && (pos < y + gap))) {
        return x;
    }

    return -1;
}

void GridGroup::saveChildGroupInfo(QGraphicsWidget *child, KConfigGroup group) const
{
    Position pos = itemPosition(child);
    group.writeEntry("Row", pos.row);
    group.writeEntry("Column", pos.column);
}

void GridGroup::restoreChildGroupInfo(QGraphicsWidget *child, const KConfigGroup &group)
{
    int row = group.readEntry("Row", -1);
    int column = group.readEntry("Column", -1);

    m_layout->addItem(child, row, column);
}

#include "gridgroup.moc"
