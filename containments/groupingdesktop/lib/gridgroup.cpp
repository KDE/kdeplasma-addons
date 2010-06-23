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
            m_spacer(new Spacer(this)),
            m_movingColumn(-1),
            m_cursorOverriden(false)
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
    connect(this, SIGNAL(appletAddedInGroup(Plasma::Applet*,AbstractGroup*)),
            this, SLOT(onAppletAdded(Plasma::Applet*,AbstractGroup*)));
    connect(this, SIGNAL(subGroupRemovedFromGroup(AbstractGroup*,AbstractGroup*)),
            this, SLOT(onSubGroupRemoved(AbstractGroup*, AbstractGroup*)));
    connect(this, SIGNAL(subGroupAddedInGroup(AbstractGroup*,AbstractGroup*)),
            this, SLOT(onSubGroupAdded(AbstractGroup*,AbstractGroup*)));
    connect(this, SIGNAL(immutabilityChanged(Plasma::ImmutabilityType)),
            this, SLOT(onImmutabilityChanged(Plasma::ImmutabilityType)));
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

void GridGroup::onAppletAdded(Plasma::Applet *applet, AbstractGroup *)
{
    applet->installSceneEventFilter(this);
}

void GridGroup::onSubGroupAdded(AbstractGroup *group, AbstractGroup *)
{
    group->installSceneEventFilter(this);
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

void GridGroup::onImmutabilityChanged(Plasma::ImmutabilityType)
{
    foreach (const LayoutItem &item, m_layoutItems) {
        setChildBorders(item.widget);
    }
}

void GridGroup::setChildBorders(QGraphicsWidget *widget)
{
    Plasma::Applet *a = qobject_cast<Plasma::Applet *>(widget);
    if (a) {
        if (immutability() == Plasma::Mutable) {
            if (m_savedHints.contains(widget)) {
                a->setBackgroundHints(m_savedHints.value(widget));
            }
        } else if (a->backgroundHints() != Plasma::Applet::NoBackground) {
                m_savedHints.insert(widget, a->backgroundHints());
                a->setBackgroundHints(Plasma::Applet::NoBackground);
        }
    }
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
    lItem.column = -1;
    lItem.row = -1;
    lItem.widget = 0;
    foreach (const LayoutItem &item, m_layoutItems) {
        if (item.row == row && item.column == column) {
            lItem.row = item.row;
            lItem.column = item.column;
            lItem.widget = item.widget;
            break;
        }
    }

    if (lItem.row != -1) {
        int columns = columnCount();
        int rows = rowCount();
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

            if (columnCount() < columns) {
                removeColumnAt(lItem.column);
            }
            if (rowCount() < rows) {
                removeRowAt(lItem.row);
            }
        }
        kDebug()<<row<<column;
        kDebug()<<m_columnWidths<<m_columnX;
        kDebug()<<m_rowHeights<<m_rowY;
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
    setChildBorders(item);

    insertColumnAt(column);
    insertRowAt(row);

    kDebug()<<row<<column;
    kDebug()<<m_columnWidths<<m_columnX;
    kDebug()<<m_rowHeights<<m_rowY;

    LayoutItem i;
    i.row = row;
    i.column = column;
    i.widget = item;
    m_layoutItems << i;

    adjustCells();
}

void GridGroup::insertColumnAt(int column)
{
    if (m_columnWidths.size() <= column) {
        if (m_columnWidths.size() == 0) {
            m_columnWidths.insert(0, contentsRect().width());
            m_columnX.insert(0, 0);
        } else {
            int otherCol = (column != 0 ? column - 1: column);
            qreal width = m_columnWidths.at(otherCol) / 2.;
            m_columnWidths.replace(otherCol, width);
            m_columnWidths.insert(column, width);
            if (column == 0) {
                m_columnX.insert(0, 0);
                m_columnX.replace(1, width);
            } else {
                m_columnX.insert(column, m_columnX.at(otherCol) + width);
            }
        }
    }
}

void GridGroup::removeColumnAt(int column)
{
    if (columnCount() > 0) {
        if (column == 0) {
            m_columnWidths.replace(1, m_columnWidths.at(0) + m_columnWidths.at(1));
            m_columnX.replace(1, 0);
        } else {
            m_columnWidths.replace(column - 1, m_columnWidths.at(column - 1) +
                                               m_columnWidths.at(column));
        }

        m_columnWidths.removeAt(column);
        m_columnX.removeAt(column);
    }
}

void GridGroup::insertRowAt(int row)
{
    if (m_rowHeights.size() <= row) {
        if (m_rowHeights.size() == 0) {
            m_rowHeights.insert(0, contentsRect().height());
            m_rowY.insert(0, 0);
        } else {
            int otherRow = (row != 0 ? row - 1: row);
            qreal height = m_rowHeights.at(otherRow) / 2.;
            m_rowHeights.replace(otherRow, height);
            m_rowHeights.insert(row, height);
            if (row == 0) {
                m_rowY.insert(0, 0);
                m_rowY.replace(1, height);
            } else {
                m_rowY.insert(row, m_rowY.at(otherRow) + height);
            }
        }
    }
}

void GridGroup::removeRowAt(int row)
{
    if (rowCount() > 0) {
        if (row == 0) {
            m_rowHeights.replace(1, m_rowHeights.at(0) + m_rowHeights.at(1));
            m_rowY.replace(1, 0);
        } else {
            m_rowHeights.replace(row - 1, m_rowHeights.at(row - 1) +
                                          m_rowHeights.at(row));
        }

        m_rowHeights.removeAt(row);
        m_rowY.removeAt(row);
    }
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

void GridGroup::save(KConfigGroup &group) const
{
    AbstractGroup::save(group);

    kDebug()<<m_columnWidths<<m_columnX;
    kDebug()<<m_rowHeights<<m_rowY;

    group.writeEntry("ColumnWidths", m_columnWidths);
    group.writeEntry("ColumnX", m_columnX);
    group.writeEntry("RowHeights", m_rowHeights);
    group.writeEntry("RowY", m_rowY);
}

void GridGroup::restore(KConfigGroup &group)
{
    AbstractGroup::restore(group);

    m_columnWidths = group.readEntry("ColumnWidths", QList<qreal>());
    m_columnX = group.readEntry("ColumnX", QList<qreal>());
    m_rowHeights = group.readEntry("RowHeights", QList<qreal>());
    m_rowY = group.readEntry("RowY", QList<qreal>());

    kDebug()<<m_columnWidths<<m_columnX;
    kDebug()<<m_rowHeights<<m_rowY;
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

    qreal widthRatio = (event->newSize().width() - 20) / (event->oldSize().width() - 20);
    qreal heightRatio = (event->newSize().height() - 20) / (event->oldSize().height() - 20);
    for (int i = 0; i < columnCount(); ++i) {
        m_columnWidths.insert(i, m_columnWidths.value(i) * widthRatio);
        m_columnX.insert(i, m_columnX.value(i) * widthRatio);
    }
    for (int i = 0; i < rowCount(); ++i) {
        m_rowHeights.insert(i, m_rowHeights.value(i) * heightRatio);
        m_rowY.insert(i, m_rowY.value(i) * heightRatio);
    }

    adjustCells();
}

bool GridGroup::sceneEventFilter(QGraphicsItem *item, QEvent *event)
{
//     if (!children().contains(qgraphicsitem_cast<QGraphicsWidget *>(item))) {
//         return false;
//     }

    switch (event->type()) {
        case QEvent::GraphicsSceneHoverMove: {
            int col = isOnAColumnBorder(mapFromItem(item, static_cast<QGraphicsSceneHoverEvent *>(event)->pos()).x());
            if (col != -1) {
                m_cursorOverriden = true;
                QApplication::setOverrideCursor(QCursor(Qt::SplitHCursor));
                return true;
            } else if (m_cursorOverriden) {
                m_cursorOverriden = false;
                QApplication::restoreOverrideCursor();
            }
        }

        break;

        case QEvent::GraphicsSceneMousePress: {
            int col = isOnAColumnBorder(mapFromItem(item, static_cast<QGraphicsSceneMouseEvent *>(event)->pos()).x());
            if (col != -1) {
                m_movingColumn = col;
                kDebug()<<col;
                return true;
            }
        }

        break;

        case QEvent::GraphicsSceneMouseMove:
            if (m_movingColumn != -1) {
                qreal x = mapFromItem(item, static_cast<QGraphicsSceneMouseEvent *>(event)->pos()).x();

                qreal pos = m_columnX.at(m_movingColumn - 1);
                qreal nextPos = m_columnX.at(m_movingColumn) +
                                m_columnWidths.at(m_movingColumn);
                m_columnWidths.replace(m_movingColumn - 1, x - pos);
                m_columnWidths.replace(m_movingColumn, nextPos - x);
                m_columnX.replace(m_movingColumn, x);

                adjustCells();

                return true;
            }

        break;

        case QEvent::GraphicsSceneMouseRelease:
            m_movingColumn = -1;
            m_cursorOverriden = false;
            QApplication::restoreOverrideCursor();

        break;

        default:
        break;
    }

    return false;
}

int GridGroup::isOnAColumnBorder(qreal x) const
{
    for (int i = 1; i < columnCount(); ++i) {
        qreal pos = m_columnX.value(i);
        if (pos > x - 20 && pos < x + 20) {
            return i;
        }
    }

    return -1;
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

    foreach (const LayoutItem &item, m_layoutItems) {
        kDebug()<<item.row<<item.column;
        kDebug()<<m_columnWidths<<m_columnX;
        kDebug()<<m_rowHeights<<m_rowY;
        qreal width = m_columnWidths.value(item.column);
        qreal height = m_rowHeights.value(item.row);

        item.widget->setPos(QPointF(m_columnX.value(item.column), m_rowY.value(item.row)) + rect.topLeft());
        item.widget->setMaximumSize(width, height);
        item.widget->resize(width, height);
    }
}

#include "gridgroup.moc"
