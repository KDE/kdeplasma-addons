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

#ifndef RTM_LIST_H
#define RTM_LIST_H

// Qt Includes
#include <QString>

// Local Includes
#include "rtm.h"
#include "session.h"

namespace RTM {

class ListPrivate;

class RTM_EXPORT List : public QObject{
  Q_OBJECT

  Q_PROPERTY(QString name READ name WRITE setName)
  Q_PROPERTY(qulonglong id READ id WRITE setId)
  Q_PROPERTY(bool smart READ isSmart WRITE setSmart)
  Q_PROPERTY(QString filter READ filter WRITE setFilter)
  
public:
  enum SortOrder {
      ByPriority,
      ByDueDate,
      ByName
  };

  static List* uninitializedList(RTM::Session* session) { return new List(session); }
  ~List();
  
  QString name() const;
  RTM::ListId id() const;
  bool isSmart() const;
  QString filter() const;
  SortOrder sortOrder() const;
  int position() const;

  /**
   * Get the number of incomplete tasks of a given priority.
   *
   * @param priority the priority to get the count for.
   * @returns the number of incomplete tasks.
   */
  int incompleteTasks(int priority) const;

  /**
   * Get the total number of tasks for this list.
   *
   * @returns the number of tasks in this list.
   */
  int taskCount() const;

  /**
   * Get a task for a given row.
   *
   * @param row the index of the task in the list.
   * @returns the task at the given row.
   */
  RTM::Task* task(int row);

  void setName(const QString &name);
  void setId(qulonglong id);
  void setSmart(bool smart);
  void setFilter(const QString &filter);
  void setSortOrder(SortOrder order);
  void setPosition(int position);

  /**
   * Set the tasks for this list.
   *
   * @param tasks the tasks for this list.
   */
  void setTasks(QList<RTM::Task*>& tasks);

  /**
   * Add a task to this list.
   *
   * @param task the task to add to the list.
   */
  void addTask(RTM::Task* task);

  /**
   * Remove a task from the list.
   *
   * @param task the task to remove.
   */
  void removeTask(RTM::Task* task);

protected:
  List(RTM::Session* session);
  
private:
  friend class TasksReader;
  friend class ListPrivate;
  ListPrivate * const d;
};

} // rtm namespace

#endif
