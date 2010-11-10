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

#include "tasksortfilter.h"
#include "taskitem.h"
#include "taskmodel.h"

#include "kdebug.h"

TaskSortFilter::TaskSortFilter(TaskModel *model, QObject* parent)
  : QSortFilterProxyModel(parent),
    m_model(model),
    filterOn(All),
    sortBy(SortPriority)
{
  setSourceModel(model);
  setDynamicSortFilter(true);
  setFilterCaseSensitivity(Qt::CaseInsensitive);
}

void TaskSortFilter::listChanged()
{
  invalidate();
}

void TaskSortFilter::setSortBy(SortBy sortBy)
{
  if (this->sortBy == sortBy)
    return;
  
  this->sortBy = sortBy;
  
  invalidate();
  emit layoutChanged();
}

bool TaskSortFilter::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
  uint leftTime = sourceModel()->data(left, Qt::RTMTimeTRole).toUInt();
  uint rightTime = sourceModel()->data(right, Qt::RTMTimeTRole).toUInt();
  
  int leftPriority = sourceModel()->data(left, Qt::RTMPriorityRole).toInt();
  int rightPriority = sourceModel()->data(right, Qt::RTMPriorityRole).toInt();
  
  if (sortBy == SortDue) {
    if (sourceModel()->data(left, Qt::RTMItemType).toInt() == RTMPriorityHeader)
      return  leftTime <= rightTime;
    else if (leftTime == rightTime)
      return leftPriority < rightPriority; // Subsort by priority
    else
      return leftTime < rightTime;
  }
  
  else if (sortBy == SortPriority) {

    
    if (left.data(Qt::RTMItemType).toInt() != RTMTaskItem)
      return leftPriority <= rightPriority;
    else if (leftPriority == rightPriority)
      return leftTime < rightTime; // Subsort by time
    else
      return leftPriority < rightPriority;
  }
  kDebug() << "Returning default";
  
  return TaskSortFilter::lessThan(left, right);
}

bool TaskSortFilter::filterAcceptsColumn(int source_column, const QModelIndex& source_parent) const { 
  Q_UNUSED(source_column)
  Q_UNUSED(source_parent)
  return true; 
}

bool TaskSortFilter::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const {
  
  QModelIndex item = sourceModel()->index(source_row, 0, source_parent);
  if (item.data(Qt::RTMItemType).toInt() == RTMPriorityHeader) { //Priority header
    return sortBy == SortPriority ? true : false; // same as an if (condition) true; else false; structure;
  }
  if (item.data(Qt::RTMItemType).toInt() == RTMDateHeader) {
    return sortBy == SortDue ? true : false;
  }
  

  const ListItem *currentList = m_model->currentList();
  if (!currentList->tasks.contains(item.data(Qt::RTMTaskIdRole).toULongLong()))
    return false;
  
  if (item.data(Qt::RTMCompletedRole).toBool())
    return false;
  
  if (item.isValid()) {
    switch (filterOn) {
      case Tags:
        foreach(const QString &tag, item.data(Qt::RTMTagsRole).toStringList()) {
          if (tag.contains(filterRegExp()))
            return true;
        }
        break;

      case Name:    
        if (item.data(Qt::RTMNameRole).toString().contains(filterRegExp()))
          return true;
        break;

      case Due:
        if (item.data(Qt::RTMDueRole).toDateTime().toString(Qt::LocalDate).contains(filterRegExp()))
          return true;
        break;

      case All:
        if ((item.data(Qt::RTMNameRole).toString().contains(filterRegExp())) ||
            (item.data(Qt::RTMDueRole).toDateTime().toString(Qt::LocalDate).contains(filterRegExp()))
            )
          return true;
        foreach(const QString &tag, item.data(Qt::RTMTagsRole).toStringList()) {
          if (tag.contains(filterRegExp()))
            return true;
        }
        break;
    }
  }
  return false;
}


void TaskSortFilter::setFilterWildcard(QString pattern) {
  if (pattern.startsWith(QLatin1String("tag:"))) {
    pattern.remove("tag:");
    filterOn = Tags;
  }
  else if (pattern.startsWith(QLatin1String("task:"))) {
    pattern.remove("task:");
    filterOn = Name;
  }
  else if (pattern.startsWith(QLatin1String("name:"))) {
    pattern.remove("name:");
    filterOn = Name;
  }
  else if (pattern.startsWith(QLatin1String("date:"))) {
    pattern.remove("date:");
    filterOn = Due;
  }
  else if (pattern.startsWith(QLatin1String("due:"))) {
    pattern.remove("due:");
    filterOn = Due;
  }
  else
    filterOn = All;
    
  QSortFilterProxyModel::setFilterWildcard(pattern);
}
