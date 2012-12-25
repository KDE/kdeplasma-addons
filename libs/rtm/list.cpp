/*
 *   Copyright 2009 Andrew Stromme <astromme@chatonka.com>
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

#include "list.h"
#include "task.h"

namespace RTM {

class ListPrivate {
  ListPrivate(List *parent)
    : q(parent)
  {}
  
  friend class List;
  List *q;
  
  QString name;
  RTM::ListId listId;
  bool smart;
  QString filter;
  QHash<RTM::TaskId, RTM::Task*> tasks;
  QHash<int, int> incompleteTally;

  // none of the following are used yet. However, they exsist in the
  //  list example and so I thought that they should be included.
  bool deleted;
  bool locked;
  bool archived;
  int position;
  List::SortOrder sortOrder;
};

List::List(Session* session) 
  : QObject(session),
  d(new ListPrivate(this))
{
    for (int i = 0; i <= 4; ++i)
    {
        d->incompleteTally.insert(i, 0);
    }
}

List::~List()
{
  delete d;
}

QString List::name() const { 
  return d->name;
}

ListId List::id() const { 
  return d->listId;
}

bool List::isSmart() const { 
  return d->smart; 
}

QString List::filter() const { 
  return d->filter; 
}

List::SortOrder List::sortOrder() const
{
    return d->sortOrder;
}

int List::position() const
{
    return d->position;
}

int List::incompleteTasks(int priority) const
{
    if (priority >= 0 && priority < d->incompleteTally.size())
        return d->incompleteTally.value(priority);
    else
        return 0;
}

int List::taskCount() const
{
    return d->tasks.size();
}

RTM::Task *List::task(int row)
{
    RTM::Task *retval = 0;
    if (row >= 0 && row < d->tasks.size())
    {
        QHash<RTM::TaskId, Task*>::iterator i = d->tasks.begin();
        i += row;
        retval = i.value();
    }
    return retval;
}

QList<Task *> List::tasks() const
{
    return d->tasks.values();
}

void List::setName(const QString& name) { 
  d->name = name; 
}

void List::setId(qulonglong id) {
  d->listId = id; 
}

void List::setSmart(bool smart) { 
  d->smart = smart; 
}

void List::setFilter(const QString& filter) { 
  d->filter = filter; 
}

void List::setSortOrder(List::SortOrder order) {
    d->sortOrder = order;
}

void List::setPosition(int position) {
    d->position = position;
}

void List::setTasks(QList<Task *> &tasks)
{
    // Reset the list and tallies.
    d->tasks.clear();
    for (int i = 0; i <= 4; ++i)
    {
        d->incompleteTally.insert(i, 0);
    }

    foreach(RTM::Task* task, tasks)
    {
        addTask(task);
    }
}

void List::addTask(Task *task)
{
    if (!d->tasks.contains(task->id()))
    {
        d->tasks.insert(task->id(), task);
        if (!task->isCompleted() && !task->isDeleted())
        {
            d->incompleteTally[task->priority()]++;
        }
    }
}

void List::removeTask(Task *task)
{
    d->tasks.remove(task->id());
    int priority = task->priority();
    if (d->incompleteTally.value(priority) > 0 &&
            !task->isCompleted() &&
            !task->isDeleted())
    {
        d->incompleteTally[priority]--;
    }
}
}
#include "moc_list.cpp"
