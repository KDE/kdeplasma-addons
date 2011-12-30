/***************************************************************************
 *   Copyright (C) 2010 by Anton Kreuzkamp <akreuzkamp@web.de>             *
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


#ifndef APPLAUNCHERITEM_H
#define APPLAUNCHERITEM_H

#include "abstracttaskitem.h"
// Own
#include "taskmanager/taskmanager.h"
#include "taskmanager/launcheritem.h"

/**
 * A launcheritem to quickly launch applications.
 */
class AppLauncherItem : public AbstractTaskItem
{
    Q_OBJECT

public:
    /** Constructs a new representation for a launcher. */
    AppLauncherItem(QGraphicsWidget *parent, Tasks *applet, TaskManager::LauncherItem *launcher);
    ~AppLauncherItem();

    TaskManager::LauncherItem* launcher() {
        return m_launcher;
    }

    virtual bool isWindowItem() const {
        return false;
    }
    virtual bool isActive() const {
        return false;
    }
    virtual void setAdditionalMimeData(QMimeData* mimeData);
    virtual void close();
    virtual void updateTask(TaskManager::TaskChanges) {}

    static QString mimetype();

    QString appName() const {
        return m_launcher->name();
    }
    KUrl launcherUrl() const {
        return m_launcher->launcherUrl();
    }
    QString windowClass() const;

private:
    void close(bool hide);

public slots:
    virtual void activate() {}

protected:
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    void updateToolTip();


private:
    TaskManager::LauncherItem *m_launcher;

};

#endif
