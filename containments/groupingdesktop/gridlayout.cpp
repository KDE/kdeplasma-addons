/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "gridlayout.h"

#include <QtGui/QGraphicsGridLayout>
#include <QtGui/QGraphicsSceneDragDropEvent>
#include <QPainter>

#include <Plasma/Applet>
#include <Plasma/PaintUtils>

K_EXPORT_PLASMA_APPLET(gridlayout, GridLayout)

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

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget * widget = 0) {

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

GridLayout::GridLayout(QObject *parent, const QVariantList &args)
          : AbstractGroup(parent, args)
{
    m_layout = new QGraphicsGridLayout(this);
    setLayout(m_layout);

    m_spacer = new Spacer(this);
    m_spacer->parent = this;
    m_spacer->hide();

    assignApplet(0, false);
}

GridLayout::~GridLayout()
{

}

void GridLayout::layoutApplet(Plasma::Applet* applet)
{
    //     QPointF pos = mapToItem(this, applet->pos());.
    QPointF pos = mapFromItem(parentItem(), applet->pos());
    kDebug()<<pos;
    if (m_spacer->geometry().contains(mapToItem(this, pos))) {
        applet->setParentItem(this);
        Position spacerPos = itemPosition(m_spacer);
        if ((spacerPos.row != -1) && (spacerPos.column != -1)) {
            m_spacer->hide();
            removeItemAt(spacerPos);
            insertItemAt(applet, spacerPos.row, spacerPos.column, Horizontal);
        }
    } else {
        if (m_spacer->isVisible()) {
            m_spacer->hide();
            removeItemAt(itemPosition(m_spacer));
        }
        showItemDropZone(applet, pos);
    }
}

Position GridLayout::itemPosition(QGraphicsWidget *widget)
{
    if (!widget) {
        return Position();
    }

    for (int i = 0; i < m_layout->rowCount(); ++i) {
        for (int j = 0; j < m_layout->columnCount(); ++j) {
            if (widget == m_layout->itemAt(i, j)) {
                return Position(i, j);
            }
        }
    }

    return Position();
}

void GridLayout::insertItemAt(QGraphicsWidget* item, int row, int column, Orientation orientation)
{
    kDebug()<<row<< column;
    if (!item) {
        return;
    }

    const int rows = m_layout->rowCount();
    const int columns = m_layout->columnCount();

    item->show();
    if ((rows > row) && (columns > column) && m_layout->itemAt(row, column)) {
        if (orientation == Horizontal) {
            for (int i = columns - 1; i >= column; --i) {
                QGraphicsLayoutItem *nextItem = takeItemAt(row, i);
                if (nextItem) {
                    m_layout->addItem(nextItem, row, i + 1);
                }
            }
        } else {
            for (int i = rows - 1; i >= row; --i) {
                QGraphicsLayoutItem *nextItem = takeItemAt(i, column);
                if (nextItem) {
                    m_layout->addItem(nextItem, i + 1, column);
                }
            }
        }
    }

    m_layout->addItem(item, row, column);
}

QGraphicsLayoutItem* GridLayout::takeItemAt(int row, int column)
{
    QGraphicsLayoutItem *item = m_layout->itemAt(row, column);
    if (item) {
        for (int i = 0; i < m_layout->count(); ++i) {
            QGraphicsLayoutItem *it = m_layout->itemAt(i);
            if (it && it == item) {
                m_layout->removeAt(i);
                return it;
            }
        }
    }

    return 0;
}

void GridLayout::removeItemAt(int row, int column)
{
    takeItemAt(row, column);
}

void GridLayout::removeItemAt(Position pos)
{
    removeItemAt(pos.row, pos.column);
}

void GridLayout::dragMoveEvent(QGraphicsSceneDragDropEvent* event)
{
    showItemDropZone(m_spacer, event->pos());
}

void GridLayout::showItemDropZone(QGraphicsWidget* widget, const QPointF& pos)
{
    const qreal x = pos.x();
    const qreal y = pos.y();

    if (widget->geometry().contains(pos)) {
        return;
    }

    Position itemPos = itemPosition(widget);

    if ((itemPos.row != -1) && (itemPos.column != -1)) {
        removeItemAt(itemPos.row, itemPos.column);
        widget->hide();
    }

    const int rows = m_layout->rowCount();
    const int columns = m_layout->columnCount();

    if (columns == 0) {
        insertItemAt(widget, 0, 0, Horizontal);
        return;
    }

    const qreal rowHeight = boundingRect().height() / rows;
    const qreal columnWidth = boundingRect().width() / columns;

    const int i = x / columnWidth;
    const int j = y / rowHeight;

    int n;
    if ((n = nearestBoundair(x, columnWidth)) != -1) {
        insertItemAt(widget, j, n, Horizontal);
        return;
    }
    if ((n = nearestBoundair(y, rowHeight)) != -1) {
        insertItemAt(widget, n, i, Vertical);
        return;
    }
}

int GridLayout::nearestBoundair(qreal pos, qreal size)
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

void GridLayout::saveAppletLayoutInfo(Plasma::Applet* applet, KConfigGroup group)
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
