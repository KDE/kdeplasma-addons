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

#include "taskmodel.h"
#include "taskitem.h"

#include <QMimeData>
#include <KDebug>
#include <limits.h>
#include <QDateTime>
#include <Plasma/Service>


TaskModel::TaskModel(Plasma::DataEngine* e, QObject* parent)
  : QStandardItemModel(parent),
  engine(e),
  dropType(SortPriority)
{ 
  currentListIndex = 0;

  rootitem = invisibleRootItem();

  refreshToplevel();
}


TaskModel::~TaskModel() {
}

void TaskModel::setDropType(SortBy dropType)
{
  this->dropType = dropType;
}

void TaskModel::dayChanged()
{ 
  QDateTime nextMidnight = QDateTime(QDate::currentDate().addDays(1));
  int secsUntilMidnight = QDateTime::currentDateTime().secsTo(nextMidnight);
  midnightAlarm.setInterval(1000*secsUntilMidnight+5000); // 5 secs over to ensure we're not under
  midnightAlarm.start();
  
  m_dateItems.at(0)->setData(0, Qt::RTMTimeTRole); // Overdue
  m_dateItems.at(0)->setData(0, Qt::RTMSortRole);
  
  m_dateItems.at(1)->setData(QDateTime(QDate::currentDate()).toTime_t(), Qt::RTMTimeTRole); // Today
  m_dateItems.at(1)->setData(QDateTime(QDate::currentDate()).toTime_t(), Qt::RTMSortRole);
  
  m_dateItems.at(2)->setData(QDateTime(QDate::currentDate()).addDays(1).toTime_t(), Qt::RTMTimeTRole); // Tomorrow
  m_dateItems.at(2)->setData(QDateTime(QDate::currentDate()).addDays(1).toTime_t(), Qt::RTMSortRole);
  
  m_dateItems.at(3)->setData(QDateTime(QDate::currentDate()).addDays(2).toTime_t(), Qt::RTMTimeTRole); // Past Tomorrow/Never
  m_dateItems.at(3)->setData(QDateTime(QDate::currentDate()).addDays(2).toTime_t(), Qt::RTMSortRole);
  
  emit modelUpdated();
}

void TaskModel::refreshToplevel()
{
  m_priorityItems.clear();
  m_dateItems.clear();
  rootitem->removeRows(0, rootitem->rowCount()); // FIXME: Crash candidate?
  
  QStringList priorityStrings;
  priorityStrings.append(i18n("Top Priority:"));
  priorityStrings.append(i18n("Medium Priority:"));
  priorityStrings.append(i18n("Low Priority:"));
  priorityStrings.append(i18n("No Priority:"));

  QStringList dateStrings;
  dateStrings.append(i18n("Overdue"));
  dateStrings.append(i18n("Today"));
  dateStrings.append(i18n("Tomorrow"));
  dateStrings.append(i18n("Anytime")); //FIXME: STRINGFREEZE: Consider a change to 'Later'
  
  for(int i=0;i<4;i++) {
    HeaderItem *priority = new HeaderItem(RTMPriorityHeader);
    priority->setData(i+1, Qt::RTMPriorityRole);
    priority->setData(i+1, Qt::RTMSortRole); // Put it in both places so both the coloring and the sorting work
    priority->setData(priorityStrings.at(i), Qt::DisplayRole);
    priority->setEditable(false);
    m_priorityItems.append(priority);
    rootitem->insertRow(rootitem->rowCount(), priority);
    
    HeaderItem *date = new HeaderItem(RTMDateHeader);
    date->setData(dateStrings.at(i), Qt::DisplayRole);
    date->setEditable(false);
    m_dateItems.append(date);
    rootitem->insertRow(rootitem->rowCount(), date);
  }
  dayChanged();
  connect(&midnightAlarm, SIGNAL(timeout()), SLOT(dayChanged()));
}
QFlags< Qt::DropAction > TaskModel::supportedDropActions() const {
  //kDebug() << "TaskModel::supportedDropActions()";
    
  return Qt::MoveAction;
}

QFlags< Qt::ItemFlag > TaskModel::flags(const QModelIndex& index) const {
  Qt::ItemFlags defaultFlags = QStandardItemModel::flags(index);
  
  if (defaultFlags.testFlag(Qt::ItemIsDragEnabled))
    defaultFlags ^= Qt::ItemIsDragEnabled; // bitwise xor, i.e. remove this flag if it is there.

  if (index.data(Qt::RTMItemType).toInt() != RTMTaskItem) //header item
      return Qt::ItemIsDropEnabled | defaultFlags;
  else
      return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
}

QStringList TaskModel::mimeTypes() const {
  QStringList types;
  types << "application/vnd.text.list";
  return types;
}

QMimeData* TaskModel::mimeData(const QList< QModelIndex >& indexes) const {
  kDebug() << "TaskModel::mimeData";
  QMimeData *mimeData = new QMimeData();
  QByteArray encodedData;

  QDataStream stream(&encodedData, QIODevice::WriteOnly);

  foreach(const QModelIndex &index, indexes)
    if (index.isValid())
      stream << index.data(Qt::RTMTaskIdRole).toString();

  mimeData->setData("application/vnd.text.list", encodedData);
  return mimeData;
}

bool TaskModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) {
  Q_UNUSED(row);
  if (action == Qt::IgnoreAction)
      return true;

  if (!data->hasFormat("application/vnd.text.list"))
      return false;

  if (column > 0)
      return false;

  RTMItemType dropHeaderType;
  
  if (dropType == SortDue)
    dropHeaderType = RTMDateHeader;
  else if (dropType == SortPriority)
    dropHeaderType = RTMPriorityHeader;
  else
    dropHeaderType = RTMDateHeader; // default it

  QModelIndex parentItem = parent;

  while (parentItem.data(Qt::RTMItemType).toInt() != dropHeaderType && parentItem.row() >= 0)
    parentItem = index(parentItem.row()-1, 0, rootitem->index()); // This is buggy. going up rows doesn't always give the right headers...
  
  kDebug() << parentItem.data(Qt::RTMItemType).toInt();
  
  QByteArray encodedData = data->data("application/vnd.text.list");
  QDataStream stream(&encodedData, QIODevice::ReadOnly);
  QStringList newItems;
  int rows = 0;

  while (!stream.atEnd()) {
      QString text;
      stream >> text;
      newItems << text;
      ++rows;
  }

  //int beginRow = rowCount(priorityParent);
  foreach(const QString &id, newItems) {
    if (m_taskItems.contains(id.toULongLong())) {
      TaskItem *item = taskFromId(id.toULongLong());
      if (item) {
        Plasma::Service *service = engine->serviceForSource("Task:" + id);
        connect(service, SIGNAL(finished(Plasma::ServiceJob*)), SIGNAL(jobFinished(Plasma::ServiceJob*)));
        if (service) {
          if (dropType == SortDue) {
            QDate headerDate = QDateTime::fromTime_t(parentItem.data(Qt::RTMTimeTRole).toUInt()).date();
            kDebug() << headerDate;
            if (headerDate < QDate::currentDate())
              headerDate = QDate::currentDate().addDays(-1); // set to due yesterday
            else if (headerDate == QDate::currentDate().addDays(2)) // set to due never
              headerDate = QDate();
            kDebug() << "Setting Item to be due: " << headerDate.toString(Qt::SystemLocaleShortDate);
            KConfigGroup cg = service->operationDescription("setDueText");
            cg.writeEntry("dueText", headerDate.toString(Qt::SystemLocaleShortDate));
            emit jobStarted(service->startOperationCall(cg));
          }
          else if (dropType == SortPriority) {
            kDebug() << "Setting Item to priority: " << parentItem.data(Qt::RTMPriorityRole).toInt();
            KConfigGroup cg = service->operationDescription("setPriority");
            cg.writeEntry("priority", parentItem.data(Qt::RTMPriorityRole).toInt());
            emit jobStarted(service->startOperationCall(cg));
          }
        }
      }
    }
  }
  return false; // We don't actually process the drop right now. We just send off the request and let the library->dataengine->plasmoid take care of it.
}

TaskItem* TaskModel::taskFromId(qulonglong id) {
  if (m_taskItems.contains(id))
    return m_taskItems.value(id);

  TaskItem *item = new TaskItem();
  item->setEditable(false); // We override the normal Qt editing structure because we provide TaskEditor overlay
  m_taskItems.insert(id, item);
  return item;
}

ListItem* TaskModel::listFromId(qulonglong id) {
  if (m_listItems.contains(id))
    return m_listItems.value(id);

  ListItem *item = new ListItem();
  m_listItems.insert(id, item);
  listUpdate(id);
  return item;
}

void TaskModel::listUpdate(qulonglong listId)
{
  if (!m_listItems.contains(listId)) {
    engine->connectSource("List:" + QString::number(listId), this);
  }
  else {
    foreach(const qulonglong &taskid, m_listItems.value(listId)->tasks) {
      engine->connectSource("Task:" + QString::number(taskid), this);
    }
  }
}

void TaskModel::switchToList(qulonglong listId) {
  m_currentList = listId;
  emit listSwitched(listId);
  emit modelUpdated();
}

const ListItem* TaskModel::currentList() {
  return listFromId(m_currentList);
}


void TaskModel::dataUpdated(const QString& name, const Plasma::DataEngine::Data& data) {
  //kDebug() << name;
  if (name.startsWith(QLatin1String("List:"))) {
    //kDebug() << data.value("id");
    qulonglong id = data.value("id").toULongLong();
    if (id == 0)
      return;
    ListItem *item = listFromId(id);

    item->id = id;
    item->name = data.value("name").toString();
    item->smart = data.value("smart").toBool();

    item->tasks.clear();

    foreach(const QString &key, data.keys()) {
      if (key != "id" && key != "name" && key != "smart")
        item->tasks.append(key.toULongLong());
    }
    if (!item->tasks.count()) {
      kDebug() << "No tasks for: " << item->name << item->id << data.keys() << item->tasks;
    }
    if (id == m_currentList)
      switchToList(m_currentList);
  }
  else if (name.startsWith(QLatin1String("Task:"))) {
    qulonglong id = data.value("id").toULongLong();
    if (id == 0) // Seems to happen when multiple applets are running
      return;
    TaskItem *item = taskFromId(id);

    item->setData(data.value("id"), Qt::RTMTaskIdRole);
    item->setData(data.value("priority"), Qt::RTMPriorityRole);
    item->setData(data.value("name"), Qt::RTMNameRole);
    item->setData(data.value("tags"), Qt::RTMTagsRole);
    item->setData(data.value("due"), Qt::RTMDueRole);
    item->setData(data.value("isCompleted"), Qt::RTMCompletedRole);

    QDateTime due = data.value("due").toDateTime();
    if (due.isValid())
      item->setData(due.toTime_t(), Qt::RTMTimeTRole);
    else
      item->setData(UINT_MAX, Qt::RTMTimeTRole);

    //item->setData(data.value("priority"), Qt::RTMSortRole);

    if (item->parent()) {
      item->parent()->takeRow(item->row());
    }
    insertTask(id);
  }
  else {
    kDebug() << "Error, unknown source: " << name;
  }
    
  emit modelUpdated();
}

void TaskModel::insertTask(qulonglong taskid)
{
  TaskItem *task = taskFromId(taskid);
  if (!task->model())
    rootitem->insertRow(rootitem->rowCount(), task);
}

#include "taskmodel.moc"
