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

#include "tasksource.h"
#include <rtm/task.h>
#include <qmetaobject.h>
#include "taskservice.h"

#include <KDebug>
#include <rtm/session.h>

TaskSource::TaskSource(RTM::TaskId i, RTM::Session* s, QObject* parent)
  : DataContainer(parent),
  id(i),
  session(s),
  task(0)
{
  connect(this, SIGNAL(updateRequested(DataContainer*)), SLOT(updateRequest(DataContainer*)));
  setObjectName("Task:" + QString::number(i));
  update();
}

TaskSource::~TaskSource() 
{
}

void TaskSource::updateRequest(Plasma::DataContainer* source) {
  Q_UNUSED(source)
  kDebug() << "Update request of task: " << task->id();
  update();
}

Plasma::Service* TaskSource::createService() {
  kDebug();
  return new TaskService(session, task, this); 
}


void TaskSource::update() {
  if (!task) {
    task = session->taskFromId(id);
    if (!task)
      return;
  }
  
  for (int i = 0; i < task->metaObject()->propertyCount(); i++) { 
    QMetaProperty prop = task->metaObject()->property(i);
    setData(prop.name(), prop.read(task));
  }

  checkForUpdate();
}
