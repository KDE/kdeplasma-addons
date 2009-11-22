/*
 *   Copyright 2009 by Giulio Camuffo <giuliocamuffo@kde.org>
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

#include <QtGui/QGraphicsGridLayout>
#include <QtGui/QGraphicsSceneDragDropEvent>
#include <QPainter>

#include <Plasma/Applet>
#include <Plasma/PaintUtils>

class Position {
    public:
        Position(int r = -1, int c = -1)
        : row(r),
        column(c)
        {
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
          : AbstractGroup(parent, wFlags)
{
    resize(200,200);
    m_layout = new QGraphicsGridLayout(this);
    setLayout(m_layout);

    m_spacer = new Spacer(this);
    m_spacer->parent = this;
    m_spacer->hide();

    connect(this, SIGNAL(appletRemoved(Plasma::Applet*)),
            this, SLOT(onAppletRemoved(Plasma::Applet *)));
}

GridLayout::~GridLayout()
{

}

void GridLayout::onAppletRemoved(Plasma::Applet *applet)
{
    removeItem(applet);
}

QString GridLayout::pluginName() const
{
    return QString("gridlayout");
}

void GridLayout::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    if (!m_layout->geometry().contains(event->pos())) {
        m_spacer->hide();
        removeItem(m_spacer);
        return;
    }

//     if (immutability() == Plasma::Mutable) {
        showItemTo(m_spacer, event->pos());
//     }
}

void GridLayout::showItemTo(QGraphicsWidget *movingWidget, const QPointF &pos)
{
    const qreal x = pos.x();
    const qreal y = pos.y();

    if (movingWidget->geometry().contains(pos)) {
        return;
    }

    Position itemPos = itemPosition(movingWidget);

    if ((itemPos.row != -1) && (itemPos.column != -1)) {
        removeItemAt(itemPos, true);
        movingWidget->hide();
    }

    const int rows = m_layout->rowCount();
    const int columns = m_layout->columnCount();

    if (columns == 0) {
        insertItemAt(movingWidget, 0, 0, Horizontal);
        return;
    }

    const qreal rowHeight = boundingRect().height() / rows;
    const qreal columnWidth = boundingRect().width() / columns;

    const int i = x / columnWidth;
    const int j = y / rowHeight;

    int n;
    if ((n = nearestBoundair(x, columnWidth)) != -1) {
        insertItemAt(movingWidget, j, n, Horizontal);
        return;
    }
    if ((n = nearestBoundair(y, rowHeight)) != -1) {
        insertItemAt(movingWidget, n, i, Vertical);
        return;
    }
}

QGraphicsLayoutItem* GridLayout::removeItemAt(Position position, bool fillLayout)
{
    return removeItemAt(position.row, position.column, fillLayout);
}

void GridLayout::removeItem(QGraphicsWidget* item, bool fillLayout)
{
    Position pos = itemPosition(item);
    if ((pos.row != -1) && (pos.column != -1)) {
        kDebug()<<"The item is not in the layout";
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

void GridLayout::layoutApplet(Plasma::Applet *applet)
{
    QPointF pos = mapFromItem(parentItem(), applet->pos());
    kDebug()<<pos;
    if (m_spacer->geometry().contains(mapToItem(this, pos))) {
        Position spacerPos = itemPosition(m_spacer);
        if ((spacerPos.row != -1) && (spacerPos.column != -1)) {
            m_spacer->hide();
            removeItemAt(spacerPos, false);
            insertItemAt(applet, spacerPos.row, spacerPos.column, Horizontal);
        }
    } else {
        if (m_spacer->isVisible()) {
            m_spacer->hide();
            removeItemAt(itemPosition(m_spacer), true);
        }
        showItemTo(applet, pos);
    }

    kDebug()<<m_layout->columnCount();
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

void GridLayout::saveAppletLayoutInfo(Plasma::Applet *applet, KConfigGroup group) const
{
    Position pos = itemPosition(applet);
    group.writeEntry("Row", pos.row);
    group.writeEntry("Column", pos.column);
}

void GridLayout::restoreAppletLayoutInfo(Plasma::Applet *applet, const KConfigGroup &group)
{
    bool isOwnApplet = false;
    foreach (Plasma::Applet *ownApplet, assignedApplets()) {
        if (applet == ownApplet) {
            isOwnApplet = true;
            break;
        }
    }

    if (isOwnApplet) {
        int row = group.readEntry("Row", -1);
        int column = group.readEntry("Column", -1);

        m_layout->addItem(applet, row, column);
    }
}

#include "gridlayout.moc"
