/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *   Copyright (C) 2007 Robert Knight <robertknight@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser/Library General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser/Library General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser/Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "OpenDocuments.h"
#include <KDebug>

namespace Lancelot {
namespace Models {

OpenDocuments::OpenDocuments()
  : m_rx("([^-]+) - ([^-]*)")
{
    m_classes << "kate" << "kwrite" << "kedit" << "VCLSalFrame" << "gimp" << "krita";

    load();
}

OpenDocuments::~OpenDocuments()
{
}

void OpenDocuments::connectTask(Task::TaskPtr task)
{
    Q_ASSERT(task);
    connect(
        task.constData(), SIGNAL(changed()),
        this, SLOT(taskChanged())
    );
}

void OpenDocuments::load()
{
    foreach (Task::TaskPtr task, TaskManager::self()->tasks().values()) {
        if (setDataForTask(task)) {
            connectTask(task);
        }
    }

    connect(
        TaskManager::self(), SIGNAL(taskAdded(Task::TaskPtr)),
        this, SLOT(taskAdded(Task::TaskPtr))
    );

    connect(
        TaskManager::self(), SIGNAL(taskRemoved(Task::TaskPtr)),
        this, SLOT(taskRemoved(Task::TaskPtr))
    );
}

void OpenDocuments::taskAdded(Task::TaskPtr task)
{
    connectTask(task);
    setDataForTask(task);
}

void OpenDocuments::taskRemoved(Task::TaskPtr task)
{
    Q_ASSERT(task);
    int index = indexOf(task->window());
    if (index != -1) {
        removeAt(index);
        m_tasks.remove(task->window());
    }
}

void OpenDocuments::taskChanged()
{
    Task* task = qobject_cast<Task*>(sender());
    Q_ASSERT(task);

    setDataForTask(Task::TaskPtr(task));
}

bool OpenDocuments::setDataForTask(Task::TaskPtr task)
{
    Q_ASSERT(task);

    if (!m_classes.contains(task->className())) return false;

    int index = indexOf(task->window());
    if (index == -1) {
        index = m_items.size();
        add (
            "", "", NULL, uint(task->window())
        );
        m_tasks[task->window()] = task;
    }

    QString title = task->visibleName();
    QString description;
    if (m_rx.exactMatch(task->visibleName())) {
        title = m_rx.cap(1);
        description = m_rx.cap(2);
    }

    kDebug() << task->className();
    kDebug() << task->classClass();

    KIcon * icon = new KIcon(QIcon(task->icon(32, 32)));

    set(index, title, description, icon, uint(task->window()));

    return true;
}

int OpenDocuments::indexOf(WId wid)
{
    QListIterator<Item> i(m_items);
    int index = 0;

    while (i.hasNext()) {
        const Item & item = i.next();
        if (item.data.toUInt() == wid) {
            return index;
        }
        ++index;
    }

    return -1;
}

void OpenDocuments::activate(int index)
{
    bool valid = true;
    WId wid = m_items.at(index).data.toUInt(&valid);
    if (valid && m_tasks.contains(wid)) {
        m_tasks[wid]->activate();
    }
    LancelotApplication::application()->hide(true);
}

}
}

#include "OpenDocuments.moc"
