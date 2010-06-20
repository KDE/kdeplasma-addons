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
        {}

        ~Spacer()
        {}

        GridGroup *parent;
        bool m_visible;
        GridGroup::Orientation lastOrientation;

    protected:
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
            m_spacer(new Spacer(this))
{
    resize(200,200);
    setMinimumSize(100,50);
    setGroupType(AbstractGroup::ConstrainedGroup);

    setContentsMargins(10, 10, 10, 10);

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
    if (children().contains(widget) && immutability() == Plasma::Mutable) {
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
    if (pos.isNull() || !boundingRect().contains(pos)) {
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

    const int rows = rowCount();
    const int columns = columnCount();

    if (columns == 0) {
        insertItemAt(m_spacer, 0, 0);
        return;
    }

    const qreal rowHeight = contentsRect().height() / rows;
    const qreal columnWidth = contentsRect().width() / columns;

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

QGraphicsWidget *GridGroup::removeItemAt(const Position &position, bool fillLayout)
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

QGraphicsWidget *GridGroup::removeItemAt(int row, int column, bool fillLayout)
{
    LayoutItem lItem;
    lItem.row = -1;
    lItem.widget = 0;
    foreach (const LayoutItem &item, m_layoutItems) {
        if (item.row == row && item.column == column) {
            lItem = item;
            break;
        }
    }

    if (lItem.row != -1) {
        m_layoutItems.removeOne(lItem);
        if (fillLayout) {
            if (columnCount() > column + 1) {
                QGraphicsWidget *movingWidget = removeItemAt(row, column + 1);
                if (movingWidget) {
                    insertItemAt(movingWidget, row, column);
                }
            }
            if (rowCount() > row + 1) {
                QGraphicsWidget *movingWidget = removeItemAt(row + 1, column);
                if (movingWidget) {
                    insertItemAt(movingWidget, row, column);
                }
            }
        }
        adjustCells();
        return lItem.widget;
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

    const int rows = rowCount();
    const int columns = columnCount();

    item->show();
    if ((rows > row) && (columns > column) && itemAt(row, column)) {
        if (orientation == Horizontal) {
            for (int i = columns - 1; i >= column; --i) {
                QGraphicsWidget *nextItem = removeItemAt(row, i, false);
                if (nextItem) {
                    insertItemAt(nextItem, row, i + 1);
                }
            }
        } else {
            for (int i = rows - 1; i >= row; --i) {
                QGraphicsWidget *nextItem = removeItemAt(i, column, false);
                if (nextItem) {
                    insertItemAt(nextItem, i + 1, column);
                }
            }
        }
    }

    insertItemAt(item, row, column);
}

QGraphicsWidget *GridGroup::itemAt(int row, int column) const
{
    foreach (const LayoutItem &item, m_layoutItems) {
        if (item.row == row && item.column == column) {
            return item.widget;
        }
    }

    return 0;
}

void GridGroup::insertItemAt(QGraphicsWidget *item, int row, int column)
{
    item->show();
    item->setAcceptDrops(false);

    LayoutItem i;
    i.row = row;
    i.column = column;
    i.widget = item;
    m_layoutItems << i;

    adjustCells();
}

Position GridGroup::itemPosition(QGraphicsWidget *widget) const
{
    for (int i = 0; i < rowCount(); ++i) {
        for (int j = 0; j < columnCount(); ++j) {
            QGraphicsWidget *w = itemAt(i, j);
            if (w == widget) {
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

    insertItemAt(child, row, column);
}

void GridGroup::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    AbstractGroup::resizeEvent(event);

    adjustCells();
}

int GridGroup::columnCount() const
{
    int columns = 0;
    foreach (const LayoutItem &item, m_layoutItems) {
        if (item.column + 1 > columns) {
            columns = item.column + 1;
        }
    }

    return columns;
}

int GridGroup::rowCount() const
{
    int rows = 0;
    foreach (const LayoutItem &item, m_layoutItems) {
        if (item.row + 1 > rows) {
            rows = item.row + 1;
        }
    }

    return rows;
}

void GridGroup::adjustCells()
{
    QRectF rect(contentsRect());

    const int columns = columnCount();
    const int rows = rowCount();
    qreal width = (columns != 0 ? rect.width() / columns : 1);
    qreal height = (rows != 0 ? rect.height() / rows : 1);

    foreach (const LayoutItem &item, m_layoutItems) {
        item.widget->setPos(QPointF(width * item.column, height * item.row) + rect.topLeft());
        item.widget->resize(width, height);
    }
}

#include "gridgroup.moc"
