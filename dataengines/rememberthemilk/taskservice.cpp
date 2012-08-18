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

#include "taskservice.h"
#include "tasksource.h"

#include <KDebug>
#include <rtm/session.h>
#include <rtm/task.h>

TaskService::TaskService(RTM::Session* session, RTM::Task* task, TaskSource* parent)
  : Plasma::Service(parent)
{
    m_session = session;
    m_task = task;
    setName("rtmtask");
    setOperationEnabled("modify", true);
}

Plasma::ServiceJob* TaskService::createJob(const QString& operation, QMap< QString, QVariant >& parameters) {
    kDebug() << "Creating Job for " << operation;
    return new ModifyTaskJob(m_session, m_task, operation, parameters, this);
}


ModifyTaskJob::ModifyTaskJob(RTM::Session* session, RTM::Task* task, const QString& operation, QMap< QString, QVariant >& parameters, QObject* parent)
  : Plasma::ServiceJob("Tasks", operation, parameters, parent),
    m_session(session)
{
  m_task = task;
}

void ModifyTaskJob::start() {
  connect(m_session, SIGNAL(taskChanged(RTM::Task*)), SLOT(result(RTM::Task*)));
  
  //FIXME: error handling?
  if (operationName() == "setListId") {
    m_task->setList(parameters().value("listId").toULongLong());
  } 
  else if (operationName() == "setCompleted") {
    m_task->setCompleted(parameters().value("completed").toBool());
  }
  else if (operationName() == "setDeleted") {
    m_task->setDeleted(parameters().value("deleted").toBool());
  } 
  else if (operationName() == "setPriority") {
    m_task->setPriority(parameters().value("priority").toInt());
  } 
  else if (operationName() == "setDue") {
    m_task->setDue(parameters().value("due").toDateTime());
  }
  else if (operationName() == "setDueText") {
    m_task->setDue(parameters().value("dueText").toString());
  }
  else if (operationName() == "setName") {
    m_task->setName(parameters().value("name").toString());
  }
  else if (operationName() == "setEstimate") {
    m_task->setEstimate(parameters().value("estimate").toString());
  } 
  else if (operationName() == "setLocationId") {
    m_task->setLocationId(parameters().value("locationId").toULongLong());
  } 
  else if (operationName() == "setRepeatString") {
    m_task->setRepeatString(parameters().value("repeatString").toString());
  } 
  else if (operationName() == "setUrl") {
    m_task->setUrl(parameters().value("url").toString());
  }
  else if (operationName() == "setTags") {
    m_task->setTags(parameters().value("tags").toStringList());
  }
  else {
    kDebug() << "Operation not recognized";
  }
}

void ModifyTaskJob::result(RTM::Task* task) {
  //kDebug() << task->id() << m_task->id();
  if (task->id() != m_task->id())
    return;

  setError(false);
  setResult(true);
  this->deleteLater();
}
