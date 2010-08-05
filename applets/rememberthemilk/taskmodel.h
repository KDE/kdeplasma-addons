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

#ifndef RTM_TASKMODEL_H
#define RTM_TASKMODEL_H

#include <QTimer>
#include <QStandardItemModel>
#include <Plasma/DataEngine>

#include "tasksortfilter.h"

namespace Plasma {
  class ServiceJob;
}

class TaskItem;

struct ListItem {
  qulonglong id;
  bool smart;
  QString name;
  QList<qulonglong> tasks;

};

class TaskModel : public QStandardItemModel
{
Q_OBJECT
public:
  explicit TaskModel(Plasma::DataEngine *engine, QObject *parent=0);
  QFlags< Qt::DropAction > supportedDropActions() const;
  QFlags< Qt::ItemFlag > flags(const QModelIndex& index) const;
  QStringList mimeTypes() const;
  QMimeData* mimeData(const QList< QModelIndex >& indexes) const;
  bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent);
  
  void setDropType(SortBy dropType);

  void listUpdate(qulonglong listId);
  void switchToList(qulonglong listId);
  const ListItem* currentList();

  ~TaskModel();
  
public slots:
  void dataUpdated(const QString &name, const Plasma::DataEngine::Data &data);
  void dayChanged();
  
signals:
  void listSwitched(qulonglong listId);
  void modelUpdated();
  void jobStarted(Plasma::ServiceJob *job);
  void jobFinished(Plasma::ServiceJob *job);
  
protected:
  void refreshToplevel();
  void insertTask(qulonglong task);
  QMap<qulonglong, TaskItem*> m_taskItems;
  QMap<qulonglong, ListItem*> m_listItems;
  int currentListIndex;
  QList<QStandardItem*>  m_priorityItems;
  QList<QStandardItem*>  m_dateItems;
  QStandardItem      *rootitem;
  qulonglong m_currentList;
  Plasma::DataEngine *engine;
  SortBy dropType;

private:
  QTimer midnightAlarm;
  TaskItem* taskFromId(qulonglong id);
  ListItem* listFromId(qulonglong id);
};

#endif // RTM_TASKMODEL_H
