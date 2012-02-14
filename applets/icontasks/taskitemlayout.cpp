/***************************************************************************
 *   Copyright (C) 2008 by Christian Mollekopf chrigi_1@fastmail.fm        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "taskitemlayout.h"

//Taskmanager
#include "taskmanager/taskmanager.h"
#include "taskmanager/abstractgroupableitem.h"
#include "taskmanager/groupmanager.h"

// Qt
#include <QGraphicsScene>
#include <QGraphicsGridLayout>

// KDE
#include <KDebug>

#include <math.h>

#include "windowtaskitem.h"
#include "taskgroupitem.h"

class LauncherSeparator : public QGraphicsWidget
{
public:

    LauncherSeparator(QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0)
        : QGraphicsWidget(parent, wFlags) {
        m_svg = new Plasma::Svg();
        m_svg->setImagePath("icontasks/launcherseparator");
        m_svg->setContainsMultipleImages(true);
        setOrientation(Qt::Horizontal);
    }

    ~LauncherSeparator() {
        delete m_svg;
    }

    void setOrientation(Qt::Orientation orientation) {
        m_orientation = orientation;

        if (m_orientation == Qt::Vertical) {
            setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        } else {
            setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
        }
    }

    Qt::Orientation orientation() {
        return m_orientation;
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
        Q_UNUSED(option);
        Q_UNUSED(widget);

        if (m_svg) {
            if (m_orientation == Qt::Horizontal) {
                m_svg->paint(painter, boundingRect(), "horizontal-separator");
            } else {
                m_svg->paint(painter, boundingRect(), "vertical-separator");
            }
        }
    }

    QSizeF sizeHint(Qt::SizeHint which, const QSizeF & constraint) const {
        QSizeF hint = QGraphicsWidget::sizeHint(which, constraint);

        if (m_orientation == Qt::Horizontal) {
            hint.setWidth(m_svg->elementSize("horizontal-separator").width());
        } else {
            hint.setHeight(m_svg->elementSize("vertical-separator").height());
        }

        return hint;
    }

private:
    Plasma::Svg *m_svg;
    Qt::Orientation m_orientation;
};

TaskItemLayout::TaskItemLayout(TaskGroupItem *parent, Tasks *applet)
    : QGraphicsGridLayout(0),
      m_groupItem(parent),
      m_rowSize(1),
      m_maxRows(1),
      m_forceRows(false),
      m_applet(applet),
      m_layoutOrientation(Qt::Horizontal),
      m_separator(parent->isRootGroup() ? new LauncherSeparator(parent) : 0L)
{
    setContentsMargins(0, 0, 0, 0);
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    setMaximumSize(INT_MAX, INT_MAX);
    //kDebug();
    foreach (AbstractTaskItem * item, m_groupItem->members()) {
        addTaskItem(item);
    }

    if (parent->isRootGroup()) {
        connect(&m_applet->groupManager(), SIGNAL(launchersChanged()), SLOT(layoutItems()));
    }
}

TaskItemLayout::~TaskItemLayout()
{
}

void TaskItemLayout::setOrientation(Plasma::FormFactor orientation)
{
    Qt::Orientation oldOrientation = m_layoutOrientation;

    if (orientation == Plasma::Vertical) {
        m_layoutOrientation = Qt::Vertical;
    } else {
        m_layoutOrientation = Qt::Horizontal;
    }

    if (m_separator) {
        m_separator->setOrientation(m_layoutOrientation);
    }

    if (m_layoutOrientation != oldOrientation) {
        layoutItems();
    }
}

bool TaskItemLayout::separatorVisible() const
{
    return m_separator && m_separator->isVisible();
}

void TaskItemLayout::addTaskItem(AbstractTaskItem * item)
{
    //kDebug();
    if (!item) {
        return;
    }

    if (item->isStartupWithTask()) {
        return;
    }

    if (m_itemPositions.contains(item)) {
        //kDebug() << "already in this layout";
        return;
    }

    if (m_groupItem->scene() && !item->scene()) {
        //kDebug() << "layout widget got scene"<<m_groupItem->scene()<< "add item to scene" <<item->scene();
        m_groupItem->scene()->addItem(item);
        //kDebug() << "itemScene" << item->scene();
    }

    if (!insert(m_groupItem->indexOf(item, false), item)) {
        return;
    }

    item->show();
    //kDebug() << "end";
}

void TaskItemLayout::removeTaskItem(AbstractTaskItem *item)
{
    if (!remove(item)) {
        return;
    }

    //kDebug();

    if (m_groupItem->scene()) {
        //kDebug() << "got scene";
        m_groupItem->scene()->removeItem(item);
    } else {
        kDebug() << "No Scene available";
    }
    //kDebug() << "done";
}

bool TaskItemLayout::insert(int index, AbstractTaskItem *item)
{
    //kDebug() << item->text() << index;
    if (!item) {
        kDebug() << "error";
        return false;
    }

    int listIndex;
    for (listIndex = 0; listIndex < m_itemPositions.size(); listIndex++) {
        if (index <= m_groupItem->indexOf(m_itemPositions.at(listIndex), false)) {
            break;
        }
    }

    if (m_itemPositions.removeAll(item) == 0) {
        connect(item, SIGNAL(destroyed(AbstractTaskItem*)), this, SLOT(remove(AbstractTaskItem*)));
    }

    m_itemPositions.insert(listIndex, item);

    layoutItems();
    return true;
}

bool TaskItemLayout::remove(AbstractTaskItem* item)
{
    if (!item) {
        kDebug() << "null Item";
        layoutItems();
        return false;
    }

    disconnect(item, 0, this, 0);
    m_itemPositions.removeAll(item);
    layoutItems();
    return true;
}


/** size including expanded groups*/
int TaskItemLayout::size()
{
    int groupSize = 0;

    foreach (AbstractTaskItem * item, m_groupItem->members()) {
        if (!item->abstractItem()) {
            // this item is a startup task or the task no longer exists
            kDebug() << "Error, invalid item in groupMembers";
            continue;
        }

        if (item->abstractItem()->itemType() == TaskManager::GroupItemType) {
            TaskGroupItem *group = static_cast<TaskGroupItem*>(item);
            if (!group->collapsed()) {
                TaskItemLayout *layout = dynamic_cast<TaskItemLayout*>(group->tasksLayout());
                if (!layout) {
                    kDebug() << "Error group has no layout";
                    continue;
                }

                // increase number of items since expanded groups occupy several spaces
                groupSize += layout->size();
                continue;
            }
        }

        ++groupSize;
    }

    //kDebug() << "group size" << groupSize;
    return groupSize;
}

//return maximum colums set by the user unless the setting is to high and the items would get unusable
int TaskItemLayout::maximumRows()
{
    int maxRows;
    if (m_itemPositions.isEmpty()) {
        return 1;
    }

    if (m_forceRows) {
        return m_maxRows;
    }

    // in this case rows are columns, columns are rows...
    //TODO basicPreferredSize isn't the optimal source here because  it changes because of margins probably
    QSizeF itemSize = m_itemPositions.first()->basicPreferredSize();
    if (m_layoutOrientation == Qt::Vertical) {
        maxRows = qMin(qMax(1, int(m_groupItem->geometry().width() / itemSize.width())), m_maxRows);
    } else {
        maxRows = qMin(qMax(1, int(m_groupItem->geometry().height() / itemSize.height())), m_maxRows);
    }

    //kDebug() << "maximum rows: " << maxRows << m_maxRows << m_groupItem->geometry().height() << itemSize.height();
    return maxRows;
}

//returns a reasonable amount of columns
int TaskItemLayout::preferredColumns()
{
    if (m_forceRows) {
        m_rowSize = 1;
    } else {
        if (m_itemPositions.isEmpty()) {
            return 1;
        }

        //TODO basicPreferredSize isn't the optimal source here because  it changes because of margins probably
        QSizeF itemSize = m_itemPositions.first()->basicPreferredSize();
        //kDebug() << itemSize.width() << m_groupItem->geometry().width();
        if (m_layoutOrientation == Qt::Vertical) {
            m_rowSize = qMax(1, int(m_groupItem->geometry().height() / itemSize.height()));
        } else {
            //Launchers doesn't need the same space as task- and groupitems on horizontal Layouts so the size needs to be adjusted
            qreal horizontalSpace = m_groupItem->geometry().width();
            m_rowSize = qMax(1, int(horizontalSpace / itemSize.width()));
        }
    }
    //kDebug() << "preferred columns: " << qMax(1, m_rowSize);
    return qMax(1, m_rowSize);
}

// <columns,rows>
QPair<int, int> TaskItemLayout::gridLayoutSize()
{
    int groupSize = size();
    //the basic settings
    int columns = preferredColumns();
    int maxRows = maximumRows();

    //check for adjustments on columns because there isnt room enough yet for all of the items
    while (ceil(static_cast<float>(groupSize) / static_cast<float>(columns)) > maxRows) {
        columns++;  // more rows needed than allowed so we add some columns instead
    }
    //kDebug() << "groupWidth" << columns << maxRows << m_maxRows;
    int rows;
    if (m_forceRows) {
        rows = maxRows;
    } else {
        rows = ceil(static_cast<float>(groupSize) / static_cast<float>(columns)); //actually needed rows
    }

    return QPair<int, int>(columns, rows);
}


void TaskItemLayout::layoutItems()
{
    //kDebug();

    QPair<int, int> grid = gridLayoutSize();
    int columns = qMax(grid.first, 1);

    //FIXME: resetting column preferred sizesthey shouldn't be taken into account for inexistent ones but they are, probably upstream issue
    for (int i = 0; i < columnCount(); ++i) {
        setColumnMaximumWidth(i, 0);
        setColumnPreferredWidth(i, 0);
    }

    for (int i = 0; i < rowCount(); ++i) {
        setRowMaximumHeight(i, 0);
        setRowPreferredHeight(i, 0);
    }

    //clearLayout
    if (m_separator) {
        m_separator->setVisible(false);
    }
    while (count()) {
        removeAt(0);
    }

    QRectF groupRect(m_groupItem->boundingRect());
    qreal cellSize(qMin(m_applet->launcherIcons() || !m_applet->autoIconScaling() ? qreal(272) : qreal(80), qMin(groupRect.width(), groupRect.height())));
    QSizeF maximumCellSize(cellSize, cellSize);

    setHorizontalSpacing(m_applet->spacing());
    setVerticalSpacing(m_applet->spacing());

    //go through all items of this layoutwidget and populate the layout with items
    int numberOfItems = 0;
    foreach (AbstractTaskItem * item, m_itemPositions) {
        int row;
        int col;
        if (m_layoutOrientation == Qt::Vertical) {
            row = numberOfItems % columns;
            col = numberOfItems / columns;
        } else {
            row = numberOfItems / columns;
            col = numberOfItems % columns;
        }

        if (m_separator && 1 == m_maxRows && Tasks::Sep_Never != m_applet->showSeparator() &&
                TaskManager::GroupManager::ManualSorting == m_applet->groupManager().sortingStrategy() &&
                m_applet->groupManager().launcherCount() && numberOfItems >= m_applet->groupManager().launcherCount() &&
                !m_separator->isVisible()) {

            // If a group associated with a launcher is split, then there will be more entries than launchers!
            // So, we need to check if this item is associated with a launcher...
            if (!(item->abstractItem() && m_applet->groupManager().isItemAssociatedWithLauncher(item->abstractItem()))) {
                addItem(m_separator, row, col, 1, 1);
                m_separator->setVisible(true);
                numberOfItems++;
                if (m_layoutOrientation == Qt::Vertical) {
                    row = numberOfItems % columns;
                    col = numberOfItems / columns;
                } else {
                    row = numberOfItems / columns;
                    col = numberOfItems % columns;
                }
            }
        }
        //not good if we don't recreate the layout every time
        //m_layout->setColumnPreferredWidth(col, columnWidth);//Somehow this line is absolutely crucial
        //m_layout->setRowPreferredHeight(row, rowHeight);//Somehow this line is absolutely crucial


        //FIXME: this is a glorious hack
        if (maximumCellSize.isValid()) {
            if (m_layoutOrientation == Qt::Vertical) {
                setRowMaximumHeight(row, maximumCellSize.height());
                setColumnMaximumWidth(col, QWIDGETSIZE_MAX);
            } else {
                setColumnMaximumWidth(col, maximumCellSize.width());
                setRowMaximumHeight(row, QWIDGETSIZE_MAX);
            }
            setRowPreferredHeight(row, maximumCellSize.height());
            setColumnPreferredWidth(col, maximumCellSize.width());
        }

        if (item->abstractItem() &&
                item->abstractItem()->itemType() == TaskManager::GroupItemType) {

            TaskGroupItem *group = static_cast<TaskGroupItem*>(item);
            if (group->collapsed()) {
//                 group->unsplitGroup();
                addItem(item, row, col, 1, 1);
                numberOfItems++;
            } else {
                TaskItemLayout *layout = group->tasksLayout();
                if (!layout) {
                    kDebug() << "group has no valid layout";
                    continue;
                }

                int groupRowWidth = m_layoutOrientation == Qt::Vertical ? layout->numberOfRows() : layout->numberOfColumns();

                if ((columns - col) < groupRowWidth) {
                    //we need to split the group
                    int splitIndex = columns - col;//number of items in group that are on this row
                    if (m_layoutOrientation == Qt::Vertical) {
                        addItem(item, row, col, splitIndex, 1);
                    } else {
                        addItem(item, row, col, 1, splitIndex);
                    }

                } else  {
                    if (m_layoutOrientation == Qt::Vertical) {
                        addItem(item, row, col, groupRowWidth, 1);
                    } else {
                        addItem(item, row, col, 1, groupRowWidth);
                    }
                }

                numberOfItems += groupRowWidth;
            }
        } else {
            addItem(item, row, col, 1, 1);
            numberOfItems++;
        }

        //kDebug() << "addItem at: " << row  <<  col;
    }

    if (m_separator && 1 == m_maxRows && Tasks::Sep_Always == m_applet->showSeparator() && !m_separator->isVisible() &&
            TaskManager::GroupManager::ManualSorting == m_applet->groupManager().sortingStrategy() &&
            m_applet->groupManager().launcherCount()) {
        if (m_layoutOrientation == Qt::Vertical) {
            addItem(m_separator, numberOfItems % columns, numberOfItems / columns, 1, 1);
        } else {
            addItem(m_separator, numberOfItems / columns, numberOfItems % columns, 1, 1);
        }
        m_separator->setVisible(true);
    }

    updatePreferredSize();
    //m_groupItem->setLayout(m_layout);
}


void TaskItemLayout::updatePreferredSize()
{
    //kDebug() << "column count: " << m_layout->columnCount();
    bool haveSep = m_separator && m_separator->isVisible();

    if (count() > (haveSep ? 1 : 0)) {
        bool vertical = m_layoutOrientation == Qt::Vertical;
        QSizeF s = itemAt(0)->preferredSize();
        QSizeF sepSize = m_separator && m_separator->isVisible()
                         ? QSizeF(vertical
                                  ? 0 : m_separator->preferredSize().width(),
                                  vertical
                                  ? m_separator->preferredSize().height() : 0)
                             : QSizeF(0, 0);
        //kDebug() << s << columnCount();
        setPreferredSize((s.width() * (columnCount() - (!vertical && haveSep ? 1 : 0))) + sepSize.width(),
                         (s.height() * (rowCount() - (vertical && haveSep ? 1 : 0))) + sepSize.height());
    } else {
        //Empty taskbar, arbitrary small value
        kDebug() << "Empty layout!!!!!!!!!!!!!!!!!!";
        if (m_layoutOrientation == Qt::Vertical) {
            setPreferredSize(/*m_layout->preferredSize().width()*/10, 10); //since we recreate the layout we don't have the previous values
        } else {
            setPreferredSize(10, /*m_layout->preferredSize().height()*/10);
        }
    }
    //kDebug() << "preferred size: " << m_layout->preferredSize();
    m_groupItem->updatePreferredSize();
}

void TaskItemLayout::setMaximumRows(int rows)
{
    if (rows != m_maxRows) {
        m_maxRows = rows;
        layoutItems();
    }
}

void TaskItemLayout::setForceRows(bool forceRows)
{
    m_forceRows = forceRows;
}

TaskItemLayout::Insert TaskItemLayout::insertionIndexAt(const QPointF &pos)
{
    Insert insert;
    int nRows = numberOfRows();
    int nCols = numberOfColumns();
    int row = nRows;
    int col = nCols;
    bool vertical = Qt::Vertical == m_layoutOrientation;

    insert.index = -1;

    //if pos is (-1,-1) insert at the end of the panel
    if (pos.toPoint() == QPoint(-1, -1)) {
        kDebug() << "Error";
        return insert;
    } else {
        QRectF siblingGeometry;
        int border = 1 + (m_applet->spacing() / 2.0);

        //get correct row
        for (int i = 0; i < nRows; i++) {
            if (vertical) {
                siblingGeometry = itemAt(0, i)->geometry();//set geometry of single item
                if (pos.x() <= (siblingGeometry.right() + border)) {
                    row = i;
                    break;
                }
            } else {
                siblingGeometry = itemAt(i, 0)->geometry();//set geometry of single item
                if (pos.y() <= (siblingGeometry.bottom() + border)) {
                    row = i;
                    break;
                }
            }
        }

        //and column
        for (int i = 0; i < nCols; i++) {
            if (vertical) {
                siblingGeometry = itemAt(i, 0)->geometry();//set geometry of single item
                qreal vertMiddle = (siblingGeometry.top() + siblingGeometry.bottom()) / 2.0;
                if (pos.y() < vertMiddle) {
                    col = i;
                    break;
                }

            } else if (itemAt(0, i)) {
                siblingGeometry = itemAt(0, i)->geometry();//set geometry of single item
                qreal horizMiddle = (siblingGeometry.left() + siblingGeometry.right()) / 2.0;
                if (pos.x() < horizMiddle) {
                    col = i;
                    break;
                }
            }
        }
    }

    insert.index = row * nCols + col;

    // Calculate geometry - used for drop indicator...
    if (nCols > 0 && nRows > 0) {
        int rowAdjust = row >= nRows ? 1 : 0;
        int colAdjust = col >= nCols ? 1 : 0;
        QGraphicsLayoutItem *item = itemAt(vertical ? (col - colAdjust) : (row - rowAdjust), vertical ? (row - rowAdjust) : (col - colAdjust));

        if (item) {
            insert.geom = item->geometry();
            if ((rowAdjust && vertical) || (colAdjust && !vertical)) {
                insert.geom.adjust(insert.geom.width(), 0, insert.geom.width(), 0);
            }
            if ((rowAdjust && !vertical) || (colAdjust && vertical)) {
                insert.geom.adjust(0, insert.geom.height(), 0, insert.geom.height());
            }
        }
    }

    if (separatorVisible() && insert.index > m_applet->groupManager().launcherCount()) {
        insert.index--;
    }

    //kDebug() << "insert Index" << insertIndex;
    return insert;
}

int TaskItemLayout::numberOfRows()
{
    if (m_layoutOrientation == Qt::Vertical) {
        return columnCount();
    } else {
        return rowCount();
    }
}

int TaskItemLayout::numberOfColumns()
{
    if (m_layoutOrientation == Qt::Vertical) {
        return rowCount();
    } else {
        return columnCount();
    }
}

#include "taskitemlayout.moc"

