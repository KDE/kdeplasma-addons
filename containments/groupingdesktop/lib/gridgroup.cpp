/*
 *   Copyright 2009-2010 by Giulio Camuffo <giuliocamuffo@gmail.com>
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
#include <QtGui/QGraphicsSceneResizeEvent>
#include <QPainter>

#include <Plasma/Applet>
#include <Plasma/IconWidget>
#include <Plasma/PaintUtils>

#include "groupingcontainment.h"
#include "gridhandle.h"

REGISTER_GROUP(grid, GridGroup)

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
            m_movingRow(-1),
            m_cursorOverriden(false),
            m_separator(new Plasma::Svg())
{
    resize(200,200);
    setMinimumSize(100,50);
    setGroupType(AbstractGroup::ConstrainedGroup);

    setContentsMargins(10, 10, 10, 10);

    m_spacer->parent = this;
    m_spacer->setZValue(1000);
    m_spacer->hide();
    m_spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_separator->setImagePath("widgets/line");
    m_separator->setContainsMultipleImages(true);

    connect(this, SIGNAL(childrenRestored()), this, SLOT(onChildrenRestored()));
    connect(this, SIGNAL(appletAddedInGroup(Plasma::Applet*,AbstractGroup*)),
            this, SLOT(onAppletAdded(Plasma::Applet*,AbstractGroup*)));
    connect(this, SIGNAL(subGroupAddedInGroup(AbstractGroup*,AbstractGroup*)),
            this, SLOT(onSubGroupAdded(AbstractGroup*, AbstractGroup*)));
    connect(this, SIGNAL(appletRemovedFromGroup(Plasma::Applet*,AbstractGroup*)),
            this, SLOT(onAppletRemoved(Plasma::Applet*,AbstractGroup*)));
    connect(this, SIGNAL(subGroupRemovedFromGroup(AbstractGroup*,AbstractGroup*)),
            this, SLOT(onSubGroupRemoved(AbstractGroup*, AbstractGroup*)));
}

GridGroup::~GridGroup()
{

}

void GridGroup::init()
{
    KConfigGroup group = config();

    m_columnWidths = group.readEntry("ColumnWidths", QList<qreal>());
    m_columnX = group.readEntry("ColumnX", QList<qreal>());
    m_rowHeights = group.readEntry("RowHeights", QList<qreal>());
    m_rowY = group.readEntry("RowY", QList<qreal>());

    for (int i = 0; i < m_rowHeights.size(); ++i) {
        QList<QGraphicsWidget *> row;
        for (int j = 0; j < m_columnWidths.size(); ++j) {
            row.append(0);
        }
        m_children.append(row);
    }

    connect(containment(), SIGNAL(widgetStartsMoving(QGraphicsWidget*)),
            this, SLOT(onWidgetStartsMoving(QGraphicsWidget*)));
}

void GridGroup::onChildrenRestored()
{
    bool modified = false;
    //check if we have empty rows
    for (int i = 0; i < m_children.size(); ++i) {
        bool removeRow = true;
        for (int j = 0; j < m_children.at(i).size(); ++j) {
            if (m_children.at(i).at(j)) {
                removeRow = false;
                break;
            }
        }

        if (removeRow) {
            modified = true;
            removeRowAt(i);
        }
    }

    //check if we have empty columns
    for (int i = 0; i < m_columnWidths.size(); ++i) {
        bool removeCol = true;
        for (int j = 0; j < m_children.size(); ++j) {
            if (m_children.at(j).at(i)) {
                removeCol = false;
                break;
            }
        }

        if (removeCol) {
            modified = true;
            removeColumnAt(i);
        }
    }

    if (modified) {
        adjustCells();
    }
}

void GridGroup::onWidgetStartsMoving(QGraphicsWidget *widget)
{
    if (children().contains(widget) && immutability() == Plasma::Mutable) {
        removeItem(widget);
        widget->removeEventFilter(this);
    }
}

void GridGroup::onAppletAdded(Plasma::Applet *applet, AbstractGroup *)
{
    setChildBorders(applet, true);
    applet->installEventFilter(this);
}

void GridGroup::onSubGroupAdded(AbstractGroup *subGroup, AbstractGroup *)
{
    setChildBorders(subGroup, true);
    subGroup->installEventFilter(this);
}

void GridGroup::onAppletRemoved(Plasma::Applet *applet, AbstractGroup *)
{
    removeItem(applet);
    setChildBorders(applet, false);
    applet->removeEventFilter(this);
}

void GridGroup::onSubGroupRemoved(AbstractGroup *subGroup, AbstractGroup *)
{
    removeItem(subGroup);
    setChildBorders(subGroup, false);
    subGroup->removeEventFilter(this);
}

void GridGroup::setChildBorders(Plasma::Applet *a, bool added)
{
    if (added) {
        m_savedAppletsHints.insert(a, a->backgroundHints());
        a->setBackgroundHints(Plasma::Applet::NoBackground);
    } else {
        if (m_savedAppletsHints.contains(a)) {
            a->setBackgroundHints(m_savedAppletsHints.value(a));
        }
    }
}

void GridGroup::setChildBorders(AbstractGroup *g, bool added)
{
    if (added) {
        m_savedGroupsHints.insert(g, g->backgroundHints());
        g->setBackgroundHints(AbstractGroup::PlainBackground);
    } else {
        if (m_savedGroupsHints.contains(g)) {
            g->setBackgroundHints(m_savedGroupsHints.value(g));
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

    if (m_children.size() == 0) {
        insertRowAt(0);
        insertColumnAt(0);
        addItem(m_spacer, 0, 0);
        return;
    }

    const qreal x = pos.x();
    const qreal y = pos.y();

    int column = 0;
    qreal p = 0;
    for (int i = 0; i < m_columnWidths.size(); ++i) {
        qreal next = m_columnWidths.at(i) * contentsRect().width();
        if (x >= p && x < p + next) {
            column = i;
            break;
        }

        p += next;
    }

    int row = 0;
    p = 0;
    for (int i = 0; i < m_rowHeights.size(); ++i) {
        qreal next = m_rowHeights.at(i) * contentsRect().height();
        if (y >= p && y < p + next) {
            row = i;
            break;
        }

        p += next;
    }

    int n;
    if ((n = isOnAColumnBorder(x)) != -1) {
        insertColumnAt(n);
        addItem(m_spacer, row, n);
    } else if ((n = isOnARowBorder(y)) != -1) {
        insertRowAt(n);
        addItem(m_spacer, n, column);
    }
}

void GridGroup::addItem(QGraphicsWidget *widget, int row, int column)
{
    widget->show();
    if (m_children.size() > row && m_children.at(row).size() > column &&
        m_children.at(row).at(column) == 0) {
        m_children[row].replace(column, widget);
    }

    adjustCells();
}

void GridGroup::removeItem(QGraphicsWidget *item, bool fillLayout)
{
    Position pos = itemPosition(item);
    if (!pos.isValid()) {
        kDebug()<<"The item"<<item<<"is not in the layout!";
        return;
    }

    m_children[pos.row].replace(pos.column, 0);

    if (fillLayout) {
        bool removeCol = true;
        for (int i = 0; i < m_children.size(); ++i) {
            if (m_children.at(i).at(pos.column)) {
                removeCol = false;
                break;
            }
        }
        if (removeCol) {
            removeColumnAt(pos.column);
        }

        bool removeRow = true;
        QList<QGraphicsWidget *> row = m_children.at(pos.row);
        foreach (QGraphicsWidget *widget, row) {
            if (widget) {
                removeRow = false;
                break;
            }
        }
        if (removeRow) {
            removeRowAt(pos.row);
        }
    }

    item->setMaximumSize(QSizeF());

    adjustCells();
}

void GridGroup::insertColumnAt(int column)
{
    int rows = m_children.size();
    for (int i = 0; i < rows; ++i) {
        m_children[i].insert(column, 0);
    }

    int cols = m_columnWidths.size();
    qreal width = 1. / (cols + 1);
    qreal w = (cols == 0 ? width : width / cols);
    m_columnWidths.insert(column, width + w);
    m_columnX.insert(column, 0);
    width = w;
    qreal x = 0;
    for (int i = 0; i < m_columnWidths.size(); ++i) {
        m_columnWidths.replace(i, m_columnWidths.at(i) - width);
        m_columnX.replace(i, x);
        x += m_columnWidths.at(i);
    }

    saveCellsInfo();
}

void GridGroup::removeColumnAt(int column)
{
    int rows = m_children.size();
    for (int i = 0; i < rows; ++i) {
        m_children[i].removeAt(column);
    }

    if (m_columnWidths.size() > column) {
        qreal width = m_columnWidths.at(column) / (m_columnWidths.size() - 1);
        m_columnWidths.removeAt(column);
        m_columnX.removeAt(column);
        qreal x = 0;
        for (int i = 0; i < m_columnWidths.size(); ++i) {
            m_columnWidths.replace(i, m_columnWidths.at(i) + width);
            m_columnX.replace(i, x);
            x += m_columnWidths.at(i);
        }
    }

    saveCellsInfo();
}

void GridGroup::insertRowAt(int row)
{
    int size = (m_children.size() != 0 ? m_children.at(0).size() : 0);
    QList<QGraphicsWidget *> newRow;
    for (int i = 0; i < size; ++i) {
        newRow.append(0);
    }
    m_children.insert(row, newRow);

    int rows = m_rowHeights.size();
    qreal height = 1. / (rows + 1);
    qreal h = (rows == 0 ? height : height / rows);
    m_rowHeights.insert(row, height + h);
    m_rowY.insert(row, 0);
    height = h;
    qreal y = 0;
    for (int i = 0; i < m_rowHeights.size(); ++i) {
        m_rowHeights.replace(i, m_rowHeights.at(i) - height);
        m_rowY.replace(i, y);
        y += m_rowHeights.at(i);
    }

    saveCellsInfo();
}

void GridGroup::removeRowAt(int row)
{
    m_children.removeAt(row);

    if (m_rowHeights.size() > row) {
        qreal height = m_rowHeights.at(row) / (m_rowHeights.size() - 1);
        m_rowHeights.removeAt(row);
        m_rowY.removeAt(row);
        qreal y = 0;
        for (int i = 0; i < m_rowHeights.size(); ++i) {
            m_rowHeights.replace(i, m_rowHeights.at(i) + height);
            m_rowY.replace(i, y);
            y += m_rowHeights.at(i);
        }
    }

    saveCellsInfo();
}

Position GridGroup::itemPosition(QGraphicsWidget *widget) const
{
    for (int i = 0; i < m_children.size(); ++i) {
        for (int j = 0; j < m_children.at(i).size(); ++j) {
            if (m_children.at(i).at(j) == widget) {
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
        m_children[spacerPos.row].replace(spacerPos.column, child);
        adjustCells();
    }
}

void GridGroup::saveCellsInfo()
{
    KConfigGroup group = config();

    group.writeEntry("ColumnWidths", m_columnWidths);
    group.writeEntry("ColumnX", m_columnX);
    group.writeEntry("RowHeights", m_rowHeights);
    group.writeEntry("RowY", m_rowY);

    emit configNeedsSaving();
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

    addItem(child, row, column);
}

void GridGroup::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    AbstractGroup::resizeEvent(event);

    if (!m_children.isEmpty()) {
        adjustCells();
    }
}

void GridGroup::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if (immutability() != Plasma::Mutable) {
        return;
    }

    int col = isOnAColumnBorder(event->pos().x());
    int row = isOnARowBorder(event->pos().y());
    if (col > 0 && col < m_columnWidths.size()) {
        m_cursorOverriden = true;
        QApplication::setOverrideCursor(QCursor(Qt::SplitHCursor));
    } else if (row > 0 && row < m_rowHeights.size()) {
        m_cursorOverriden = true;
        QApplication::setOverrideCursor(QCursor(Qt::SplitVCursor));
    } else if(m_cursorOverriden) {
        m_cursorOverriden = false;
        QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));
    }

    if (scene()->itemAt(event->scenePos()) != this) {
        m_cursorOverriden = false;
        QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));
    }
}

void GridGroup::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    if (m_cursorOverriden) {
        m_cursorOverriden = false;
        QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));
    }
}

void GridGroup::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        int col = isOnAColumnBorder(event->pos().x());
        int row = isOnARowBorder(event->pos().y());
        if (col > 0 && col < m_columnWidths.size()) {
            m_movingColumn = col;
            return;
        } else if (row > 0 && row < m_rowHeights.size()) {
            m_movingRow = row;
            return;
        }
    }

    AbstractGroup::mousePressEvent(event);
}

void GridGroup::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_movingColumn != -1) {
        qreal x = event->pos().x() / contentsRect().width();

        qreal pos = m_columnX.at(m_movingColumn - 1);
        qreal nextPos = m_columnX.at(m_movingColumn) +
                        m_columnWidths.at(m_movingColumn);
        const qreal MIN_WIDTH = 30. / contentsRect().width();

        if (x - pos > MIN_WIDTH && nextPos - x > MIN_WIDTH) {
            m_columnWidths.replace(m_movingColumn - 1, x - pos);
            m_columnWidths.replace(m_movingColumn, nextPos - x);
            m_columnX.replace(m_movingColumn, x);

            adjustCells();
        }
        return;
    } else if (m_movingRow != -1) {
        qreal y = event->pos().y() / contentsRect().height();

        qreal pos = m_rowY.at(m_movingRow - 1);
        qreal nextPos = m_rowY.at(m_movingRow) + m_rowHeights.at(m_movingRow);
        const qreal MIN_HEIGHT = 30. / contentsRect().height();

        if (y - pos > MIN_HEIGHT && nextPos - y > MIN_HEIGHT) {
            m_rowHeights.replace(m_movingRow - 1, y - pos);
            m_rowHeights.replace(m_movingRow, nextPos - y);
            m_rowY.replace(m_movingRow, y);

            adjustCells();
        }
        return;
    }

    AbstractGroup::mouseMoveEvent(event);
}

void GridGroup::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    m_movingColumn = -1;
    m_movingRow = -1;
    m_cursorOverriden = false;
    QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));
    saveCellsInfo();

    AbstractGroup::mouseReleaseEvent(event);
}

void GridGroup::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint) {
        if (containment()->formFactor() == Plasma::Vertical ||
            containment()->formFactor() == Plasma::Horizontal) {
            setMinimumSize(QSizeF());
            setContentsMargins(0, 0, 0, 0);
        }
    }
}

bool GridGroup::eventFilter(QObject *, QEvent *event)
{
    if (event->type() == QEvent::GraphicsSceneHoverMove) {
        m_movingColumn = -1;
        m_movingRow = -1;
        m_cursorOverriden = false;
        QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));
    }

    return false;
}

int GridGroup::isOnAColumnBorder(qreal x) const
{
    qreal pos = 0;
    int gap = 0;
    for (int i = 0; i < m_columnWidths.size(); ++i) {
        gap = m_columnWidths.at(i) * contentsRect().width() / 3.;
        if (pos > x - gap && pos < x + gap) {
            return i;
        }
        pos += (m_columnWidths.at(i) * contentsRect().width());
    }
    if (pos > x - gap && pos < x + gap) {
        return m_columnWidths.size();
    }

    return -1;
}

int GridGroup::isOnARowBorder(qreal y) const
{
    qreal pos = 0;
    int gap = 0;
    for (int i = 0; i < m_rowHeights.size(); ++i) {
        gap = m_rowHeights.at(i) * contentsRect().height() / 3.;
        if (pos > y - gap && pos < y + gap) {
            return i;
        }
        pos += (m_rowHeights.at(i) * contentsRect().height());
    }
    if (pos > y - gap && pos < y + gap) {
        return m_rowHeights.size();
    }

    return -1;
}

void GridGroup::adjustCells()
{
    QRectF rect(contentsRect());

    const int spacing = 3;

    for (int i = 0; i < m_children.size(); ++i) {
        qreal height = m_rowHeights.at(i) * contentsRect().height() - 2 * spacing;
        qreal y = m_rowY.at(i) * contentsRect().height() + spacing;
        for (int j = 0; j < m_children.at(i).size(); ++j) {
            QGraphicsWidget *widget = m_children.at(i).at(j);
            if (widget) {
                qreal width = m_columnWidths.at(j) * contentsRect().width() - 2 * spacing;
                widget->setPos(QPointF(m_columnX.value(j) * contentsRect().width() + spacing, y) +
                               rect.topLeft());
                widget->setMaximumSize(width, height);
                QSizePolicy p = widget->sizePolicy();
                QSizeF size;
                if (contentsRect().width() >= contentsRect().height()) {
                    size = QSizeF(height, height);
                } else {
                    size = QSizeF(width, width);
                }
                if (QSizePolicy::ExpandFlag & p.verticalPolicy()) {
                    size.setHeight(height);
                }
                if (QSizePolicy::ExpandFlag & p.horizontalPolicy()) {
                    size.setWidth(width);
                }
                widget->resize(size);
            }
        }
    }

    update();
}

Handle *GridGroup::createHandleForChild(QGraphicsWidget *child)
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

void GridGroup::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    AbstractGroup::paint(painter, option, widget);

    if (m_children.isEmpty() || immutability() != Plasma::Mutable) {
        return;
    }

    QRectF rect(contentsRect());

    QSizeF lineSize(m_separator->elementSize("vertical-line").width(), rect.height());
    for (int i = 1; i < m_columnX.size(); ++i) {
        const qreal x = m_columnX.at(i) * rect.width();
        QRectF r(rect.topLeft() + QPointF(x, 0), lineSize);
        m_separator->paint(painter, r, "vertical-line");
    }

    lineSize = QSizeF(rect.width(), m_separator->elementSize("horizontal-line").height());
    for (int i = 1; i < m_rowY.size(); ++i) {
        const qreal y = m_rowY.at(i) * rect.height();
        QRectF r(rect.topLeft() + QPointF(0, y), lineSize);
        m_separator->paint(painter, r, "horizontal-line");
    }
}

QString GridGroup::prettyName()
{
    return i18n("Grid Group");
}

QSet<Plasma::FormFactor> GridGroup::availableOnFormFactors()
{
    QSet<Plasma::FormFactor> set;
    set.insert(Plasma::Planar);
    set.insert(Plasma::MediaCenter);
    set.insert(Plasma::Horizontal);
    set.insert(Plasma::Vertical);

    return set;
}

#include "gridgroup.moc"
