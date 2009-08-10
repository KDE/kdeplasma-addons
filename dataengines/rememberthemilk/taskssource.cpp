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

#include "taskssource.h"

#include <rtm/rtm.h>
#include <rtm/task.h>
#include <rtm/session.h>
#include <rtm/request.h>


#include <KIO/Job>
#include "rtmengine.h"
#include "tasksource.h"
#include "taskservice.h"


TasksSource::TasksSource(RtmEngine* engine, RTM::Session* session)
    : Plasma::DataContainer(engine),
      m_engine(engine),
      m_session(session)
{
  connect(session, SIGNAL(taskChanged(RTM::Task*)), this, SLOT(taskChanged(RTM::Task*)));
  connect(session, SIGNAL(tasksChanged()), this, SLOT(tasksChanged()));
  connect(&timer, SIGNAL(timeout()), this, SLOT(refresh()));
  timer.setInterval(1000*60*2); // 5 minute refresh. TODO: Make Configurable.
  timer.start();
  setObjectName("Tasks");
  tasksChanged();
}

TasksSource::~TasksSource()
{
}

TaskSource* TasksSource::setupTaskSource(const QString& source) {
  QString id = source;
  TaskSource *tasksource = new TaskSource(id.remove("Task:").toULongLong(), m_session, this);
  return tasksource;
}

void TasksSource::refresh()
{
  if (!m_session->authenticated())
    return; // We can't do anything with a non-authenticated session
  m_session->createTimeline(); // Allow us to get new information that people entered on the website or in another api
  m_session->refreshTasksFromServer();
}

void TasksSource::tasksChanged() {
  removeAllData();
  kDebug() << "Updating Tasks. " << m_session->cachedTasks().count();
  foreach(RTM::Task* task, m_session->cachedTasks())
    setData(QString::number(task->id()), task->name()); // ids are unique, names are not
}


void TasksSource::taskChanged(RTM::Task* task) {
  setData(QString::number(task->id()), task->name());
  m_engine->updateTaskSource(QString::number(task->id()));
}

#include "taskssource.moc"

