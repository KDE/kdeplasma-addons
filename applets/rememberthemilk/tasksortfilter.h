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

#ifndef RTM_TASKSORTFILTER_H
#define RTM_TASKSORTFILTER_H

class TaskModel;

#include <QSortFilterProxyModel>

enum FilterOn { All, Name, Tags, Due };
enum SortBy { SortDue = 0, SortPriority = 1  };

class TaskSortFilter : public QSortFilterProxyModel
{
Q_OBJECT
public:
  explicit TaskSortFilter(TaskModel* model, QObject* parent = 0);
  bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const;
  bool filterAcceptsColumn(int source_column, const QModelIndex& source_parent) const;
  bool lessThan(const QModelIndex& left, const QModelIndex& right) const;
  
  void setSortBy(SortBy sortBy);
  
public slots:
  void setFilterWildcard(QString filter);
  void listChanged();
  
private:
  TaskModel *m_model;
  FilterOn filterOn;
  SortBy sortBy;
};

#endif
