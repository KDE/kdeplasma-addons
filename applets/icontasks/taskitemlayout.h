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


#ifndef TASKITEMLAYOUT_H
#define TASKITEMLAYOUT_H

//Own
#include "tasks.h"

// Qt
#include <QGraphicsGridLayout>
#include <QList>

class TaskGroupItem;
class AbstractTaskItem;
class LauncherSeparator;

/**
 * A Layout for the expanded group
 */
class TaskItemLayout : public QObject, public QGraphicsGridLayout
{
    Q_OBJECT

public:
    struct Insert {
        int index;
        QRectF geom;
    };

    TaskItemLayout(TaskGroupItem * parent, Tasks *applet);
    ~TaskItemLayout();
    /** insert the item on the index in TaskGroupItem::getMemberList */
    void addTaskItem(AbstractTaskItem*);
    void removeTaskItem(AbstractTaskItem*);
    /** insert the item on a specific index*/
    bool insert(int index, AbstractTaskItem* item);

    /** returns the insert index for a task drop on pos */
    Insert insertionIndexAt(const QPointF &pos);
    /** set the maximum number of rows */
    void setMaximumRows(int);
    /** force the layout to use maximumRows setting and fill rows before columns */
    void setForceRows(bool);

    /** the size including expanded groups*/
    int size();

    /** returns columnCount or rowCount depending on m_applet->formFactor() */
    int numberOfRows();
    /** returns columnCount or rowCount depending on m_applet->formFactor()*/
    int numberOfColumns();

    /** Returns the preferred number of rows based on the user settings but limited by calculation to honor AbstractGroupableItem::basicPreferredSize()*/
    int maximumRows();
    /** Returns the preferred number of columns calculated on base of AbstractGroupableItem::basicPreferredSize()*/
    int preferredColumns();

    /** Set the layout Orientation, normally set to formFactor of applet*/
    void setOrientation(Plasma::FormFactor orientation);

    bool separatorVisible() const;

public Q_SLOTS:
    /** Populates the actual QGraphicsGridLayout with items*/
    void layoutItems();
private:
    void adjustStretch();
    void updatePreferredSize();

private Q_SLOTS:
    bool remove(AbstractTaskItem* item);

private:
    TaskGroupItem *m_groupItem;
    QList <AbstractTaskItem*> m_itemPositions;
    /** Calculates the number of columns and rows for the layoutItems function and returns <columns/rows>*/
    QPair<int, int> gridLayoutSize();

    /** Limit before row is full, more columns are added if maxRows is exeeded*/
    int m_rowSize;
    /** How many rows should be used*/
    int m_maxRows;

    bool m_forceRows;

    Tasks *m_applet;

    Qt::Orientation m_layoutOrientation;
    LauncherSeparator *m_separator;
};

#endif
