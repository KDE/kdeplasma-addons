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

#include "tasksservice.h"

#include <KDebug>
#include <rtm/session.h>

TasksService::TasksService(RTM::Session* session, QObject* parent)
  : Plasma::Service(parent)
{
    m_session = session;
    setName("rtmtasks");
    setOperationEnabled("create", true);
}

Plasma::ServiceJob* TasksService::createJob(const QString& operation, QMap< QString, QVariant >& parameters) {
    kDebug() << "Creating Job for" << operation;
    return new TasksJob(m_session, operation, parameters, this);
}


TasksJob::TasksJob(RTM::Session* session, const QString& operation, QMap< QString, QVariant >& parameters, QObject* parent)
  : Plasma::ServiceJob("Auth", operation, parameters, parent),
    m_session(session)
{
}

void TasksJob::start() {
  connect(m_session, SIGNAL(tasksChanged()), SLOT(result()));
  //FIXME: error handling?
  if (operationName() == "create") {
    m_session->addTask(parameters().value("task").toString(), parameters().value("listid").toULongLong());
  } 
}

void TasksJob::result() {

  setError(false);
  setResult(true);
  
  this->deleteLater();
}

