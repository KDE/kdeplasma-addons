/***************************************************************************
 *   Copyright (C) 2007 by Robert Knight <robertknight@gmail.com>          *
 *   Copyright (C) 2008 by Alexis Ménard <darktears31@gmail.com>           *
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


#ifndef TASKGROUPITEM_H
#define TASKGROUPITEM_H

#include "abstracttaskitem.h"
#include "windowtaskitem.h"
// Own
#include "taskmanager/taskmanager.h"
#include "tasks.h"
#include <QMap>
#include <QHash>

using TaskManager::TaskGroup;
using TaskManager::GroupPtr;
using TaskManager::TaskItem;
using TaskManager::AbstractGroupableItem;

class TaskItemLayout;
class DropIndicator;
class QGraphicsLinearLayout;

namespace Plasma
{
class Dialog;
}
typedef QMap<int, AbstractTaskItem*> Order;

/**
 * A task item for a TaskGroup. It can be displayed collapsed as single item or expanded as group.
 */
class TaskGroupItem : public AbstractTaskItem
{
    Q_OBJECT

public:
    /** Constructs a new representation for a taskgroup. */
    TaskGroupItem(QGraphicsWidget *parent, Tasks *applet);
    virtual ~TaskGroupItem();

    /** Sets the group represented by this task. */
    void setGroup(TaskManager::GroupPtr);

    /** Returns the group represented by this task. */
    TaskManager::GroupPtr group() const;

    virtual void close();

    QHash<AbstractGroupableItem *, AbstractTaskItem*> members() const;
    int count() const;
    AbstractTaskItem * activeSubTask();

    virtual bool isWindowItem() const;
    virtual bool isActive() const;
    bool windowPreviewOpen() const;
    QString appName() const;
    KUrl launcherUrl() const;
    QString windowClass() const;
    bool collapsed() const;
    virtual void toCurrentDesktop();

    /** Returns Direct Member group if the passed item is in a subgroup */
    AbstractTaskItem *directMember(AbstractTaskItem *);

    /** Maximum number of Rows the group will have */
    int maxRows();
    void setMaxRows(int);

    TaskItemLayout *tasksLayout();

    int indexOf(AbstractTaskItem *task, bool descendGroups = true);

    int optimumCapacity();

    AbstractTaskItem* abstractTaskItem(AbstractGroupableItem *);

    void setAdditionalMimeData(QMimeData* mimeData);
    void publishIconGeometry() const;
    void publishIconGeometry(const QRect &rect) const;
    QWidget *popupDialog() const;
    AbstractTaskItem *taskItemForWId(WId id);
    AbstractTaskItem *matchingItem(TaskManager::AbstractGroupableItem *from);

private:
    void close(bool hide);

signals:
    /** Emitted when a window is selected for activation, minimization, iconification */
    void groupSelected(TaskGroupItem *);
    void sizeHintChanged(Qt::SizeHint);
    /** informs the parent group about changes */
    void changed();

public slots:
    virtual void activate();

    /** Reload all tasks */
    void reload();

    void expand();
    void collapse();
    void updatePreferredSize();
    void clearGroup();
    bool isRootGroup() const;

public slots:
    void updateActive(AbstractTaskItem *);
    void relayoutItems();

protected:
    void activateOrIconify();
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    void dragLeaveEvent(QGraphicsSceneDragDropEvent *event);
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
    void dropEvent(QGraphicsSceneDragDropEvent *event);
    bool focusNextPrevChild(bool next);

    void handleDroppedId(WId id, AbstractTaskItem *targetTask, QGraphicsSceneDragDropEvent *event);

    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void resizeEvent(QGraphicsSceneResizeEvent *event);

    void updateToolTip();

protected slots:
    virtual void wheelEvent(QGraphicsSceneWheelEvent *event);

private Q_SLOTS:
    void checkUpdates();
    void constraintsChanged(Plasma::Constraints);
    void handleActiveWindowChanged(WId id);

    void updateTask(::TaskManager::TaskChanges changes);

    /** Stay informed about changes in group */
    void itemAdded(AbstractGroupableItem *);
    void itemRemoved(AbstractGroupableItem *);

    /** Update to new position*/
    void itemPositionChanged(AbstractGroupableItem *);

    void popupMenu();
    /** force a relayout of all items */
    void popupVisibilityChanged(bool visible);

private:
    AbstractTaskItem* createAbstractItem(AbstractGroupableItem * groupableItem);
    TaskGroupItem* createNewGroup(QList <AbstractTaskItem *> members);
    WindowTaskItem * createWindowTask(TaskManager::TaskItem* task);
    TaskGroupItem * createTaskGroup(GroupPtr);
    WindowTaskItem *createStartingTask(TaskManager::TaskItem* task);

    void removeItem(AbstractTaskItem *item);

    void layoutTaskItem(AbstractTaskItem* item, const QPointF &pos);
    void setSplitIndex(int position);

    int totalSubTasks();
    bool focusSubTask(bool next, bool activate);
    AbstractTaskItem * selectSubTask(int index);

    QWeakPointer<TaskGroup> m_group;

    QHash<AbstractGroupableItem *, AbstractTaskItem*> m_groupMembers;

    TaskItemLayout *m_tasksLayout;
    QTimer *m_popupMenuTimer;
    QHash<int, Order> m_taskOrder;
    int m_lastActivated;
    int m_activeTaskIndex;
    int m_maximumRows;
    QGraphicsWidget *m_offscreenWidget;
    QGraphicsLinearLayout *m_offscreenLayout;
    bool m_collapsed;
    QGraphicsLinearLayout *m_mainLayout;
    Plasma::Dialog *m_popupDialog;
    QTimer *m_updateTimer;
    TaskManager::TaskChanges m_changes;

    DropIndicator *m_dropIndicator;
};

#endif
