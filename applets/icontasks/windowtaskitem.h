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


#ifndef WINDOWTASKITEM_H
#define WINDOWTASKITEM_H

#include "abstracttaskitem.h"
// Own
#include "taskmanager/taskmanager.h"
#include "taskmanager/taskitem.h"
#include <KDE/KUrl>

/**
 * A task item for a task which represents a window on the desktop.
 */
class WindowTaskItem : public AbstractTaskItem
{
    class BusyWidget;


    Q_OBJECT

public:
    /** Constructs a new representation for a window task. */
    WindowTaskItem(QGraphicsWidget *parent, Tasks *applet);
    virtual ~WindowTaskItem();

    /** Sets the window/startup represented by this task. */
    void setTask(TaskManager::TaskItem* taskItem);

    /** Returns the window represented by this task. */
    TaskManager::Task *windowTask() const;

    /** Tells the window manager the minimized task's geometry. */
    void publishIconGeometry() const;

    // used by the group; for efficiency this avoids multiple calls to
    // AbstractTaskItem::iconScreenGeometry
    void publishIconGeometry(const QRect &rect) const;

    virtual bool isWindowItem() const;
    virtual bool isActive() const;
    virtual void setAdditionalMimeData(QMimeData* mimeData);
    QGraphicsWidget *busyWidget() const;

    QString appName() const;
    KUrl launcherUrl() const;
    QString windowClass() const;
    void showContextMenu(const QPoint &pos, bool showAppMenu);
    virtual int pid() const;
    virtual void toCurrentDesktop();

signals:
    /** Emitted when a window is selected for activation, minimization, iconification */
    //void windowSelected(WindowTaskItem *); //what is it for?

public slots:
    void activate();
    void close();

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void updateToolTip();

private slots:
    void updateTask(::TaskManager::TaskChanges changes);
    void gotTaskPointer();

private:
    void close(bool hide);
    /** Sets the starting task represented by this item. */
    void setStartupTask(TaskManager::TaskItem* task);

    /** Sets the window represented by this task. */
    void setWindowTask(TaskManager::TaskItem* taskItem);

    QWeakPointer<TaskManager::TaskItem> m_task;
    BusyWidget *m_busyWidget;
};

#endif
