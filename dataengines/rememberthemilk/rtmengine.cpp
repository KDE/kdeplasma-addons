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

#include "rtmengine.h"

#include "taskssource.h"
#include "listssource.h"
#include "tasksource.h"

#include "authservice.h"
#include "tasksservice.h"

 

#include <Plasma/DataContainer>

#include <rtm/rtm.h> 
#include <rtm/session.h>

const QString RtmEngine::apiKey = "631e881f0e5671d237c1a2a0a64d5b98";
const QString RtmEngine::sharedSecret = "a1c48d8944bce414";


RtmEngine::RtmEngine(QObject* parent, const QVariantList& args)
    : Plasma::DataEngine(parent, args),
    session(0)
{
    // We ignore any arguments - data engines do not have much use for them
    Q_UNUSED(args)
 
    // This prevents applets from setting an unnecessarily high
    // Polling interval and breaking the RTM ToS that say 1 poll per sec
    setMinimumPollingInterval(1000);

    session = new RTM::Session(apiKey, sharedSecret, RTM::Delete, QString());
    connect(session, SIGNAL(tokenCheck(bool)), SLOT(tokenCheck(bool)));
}

RtmEngine::~RtmEngine() {
  if (session)
    session->deleteLater();
}

bool RtmEngine::tokenCheck(bool success)
{
  if (success) {
    session->refreshListsFromServer();
    session->refreshTasksFromServer();
    this->token = session->token();
  }
  return updateSourceEvent("Auth");
}


bool RtmEngine::authenticated() const
{
  return session->authenticated();
}

void RtmEngine::updateTaskSource(const QString &taskid) {
  updateSourceEvent("Task:" + taskid);
}

void RtmEngine::updateListSource(const QString &listid) {
  updateSourceEvent("List:" + listid);
}

bool RtmEngine::sourceRequestEvent(const QString &name)
{
    if (name == "Lists") {
      ListsSource *lsource = new ListsSource(this, session);
      addSource(lsource);
    } else if (name == "Tasks") {
      TasksSource *tsource = new TasksSource(this, session);
      addSource(tsource);
    } else if (name.startsWith("Task:") && authenticated()) {
      TasksSource *taskssource = dynamic_cast<TasksSource*>(containerForSource("Tasks")); 
      if (!taskssource) {
        sourceRequestEvent("Lists");
        sourceRequestEvent("Tasks"); // we need a Tasks source to get any info about an individual task
        taskssource = dynamic_cast<TasksSource*>(containerForSource("Tasks")); 
      }
      addSource(taskssource->setupTaskSource(name));
    } else if (name.startsWith("List:") && authenticated()) {
      ListsSource *listssource = dynamic_cast<ListsSource*>(containerForSource("Lists")); 
      if (!listssource) {
        sourceRequestEvent("Lists");
        sourceRequestEvent("Tasks");
        listssource = dynamic_cast<ListsSource*>(containerForSource("Lists")); 
      }
      addSource(listssource->setupListSource(name));
    }
    return updateSourceEvent(name);
}

bool RtmEngine::updateSourceEvent(const QString& source) {
  if (source == "Auth") {
    setData(source, "ValidToken", session->authenticated());
    setData(source, "Token", session->token());
    setData(source, "Permissions", session->permissions());
    return true;
  }
  else if (source.startsWith("Lists")) {
    ListsSource *listssoruce = static_cast<ListsSource*>(containerForSource(source)); 
    listssoruce->refresh();
    return true;
  }
  else if (source.startsWith("Tasks")) {
    TasksSource *taskssource = static_cast<TasksSource*>(containerForSource(source)); 
    taskssource->refresh();
    return true;
  }
  else if (source.startsWith("List:")) {
    ListSource *listsource = static_cast<ListSource*>(containerForSource(source)); 
    if (listsource)
      listsource->update();
    return true;
  }
  else if (source.startsWith("Task:")) {
    TaskSource *tasksource = static_cast<TaskSource*>(containerForSource(source)); 
    if (tasksource)
      tasksource->update();
    return true;
  }
  return false;
}

void RtmEngine::dataUpdate(const QString& source, const Plasma::DataEngine::Data& data) {
  setData(source, data);
}


Plasma::Service* RtmEngine::serviceForSource(const QString& source) {
  if (source.startsWith("Task:") && authenticated()) {
    TaskSource *tasksource = dynamic_cast<TaskSource*>(containerForSource(source)); 
    if (tasksource)
      return tasksource->createService();
  }
  else if (source == "Auth") {
    return new AuthService(session, this);
  }
  else if (source == "Tasks") {
    return new TasksService(session, this);
  }
  return 0; // Return 0 pointer if source is invalid
}


K_EXPORT_PLASMA_DATAENGINE(rtm, RtmEngine)
 
#include "rtmengine.moc"
