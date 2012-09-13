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

#ifndef RTM_SESSION_P_H
#define RTM_SESSION_P_H

#include "session.h"

#include "xmlreaders.h"
#include "request.h"
#include "auth.h"
#include "task.h"

#include <QUrl>
#include <QStringList>
#include <QString>
#include <QByteArray>
#include <QCryptographicHash>
#include <QBuffer>
#include <QCoreApplication>
#include <QDomDocument>
#include <QDomNode>
#include <QDomElement>
#include <QDomNodeList>
#include <QtDebug>
#include <QTimer>

#ifndef QTONLY
#include <KSystemTimeZones>
#include <KTimeZone>
#include <Solid/Networking>
#endif

class RTM::SessionPrivate {
  SessionPrivate(Session *parent)
    : q(parent),
      auth(0),
      online(true),
      tasksChanged(false),
      listsChanged(false)
  {
#ifndef QTONLY
      QObject::connect(Solid::Networking::notifier(), SIGNAL(statusChanged(Solid::Networking::Status)), q, SLOT(networkStatusChanged(Solid::Networking::Status)));
      if (Solid::Networking::status() == Solid::Networking::Unconnected) {
        online = false;
        qDebug() << "We are NOT Online :(";
      }
#endif
  }
  ~SessionPrivate() {
    if (auth)
      auth->deleteLater();
  }

#ifndef QTONLY
  void networkStatusChanged(Solid::Networking::Status status) {
    switch (status) {
    case Solid::Networking::Connected:
    case Solid::Networking::Unknown:
        if (online)
          return;

        online = true;
        q->checkToken();
        refreshSettings();
        break;
    case Solid::Networking::Unconnected:
    case Solid::Networking::Disconnecting:
    case Solid::Networking::Connecting:
        if (!online)
          return;

        online = false;
        break;
    }
  }
#endif

  void offlineError() {
    online = false;
    qDebug() << "retesting offline status in 60 seconds";
    QTimer::singleShot(60*1000, q, SLOT(retestOfflineStatus()));
  }

  void retestOfflineStatus() {
    online = true;
    qDebug() << "retesting offline status";
    q->checkToken();
  }

  void connectOfflineSignal(RTM::Request *request) {
    QObject::connect(request, SIGNAL(offlineError()), q, SLOT(offlineError()));
  }

  void populateSmartList(List * list)
  {
    if (!online)
      return;

    qDebug() << "Populating Smart List: " << list->name();
    // We do this next bit manually so it doesn't get auto-connected to taskUpdate()
    RTM::Request *smartListRequest = new RTM::Request("rtm.tasks.getList", q->apiKey(), q->sharedSecret());
    smartListRequest->addArgument("auth_token", q->token());
    smartListRequest->addArgument("list_id", QString::number(list->id()));
    smartListRequest->addArgument("filter", list->filter());

    QObject::connect(smartListRequest, SIGNAL(replyReceived(RTM::Request*)), q, SLOT(smartListReply(RTM::Request*)));

    smartListRequest->sendRequest();
  }

  void updateSmartLists() {
    if (!online)
      return;

    foreach (RTM::List* list, lists) {
      if (list->isSmart()) {
        populateSmartList(list);
      }
    }
  }

  void taskUpdate(RTM::Request* reply) {
    TasksReader reader(reply, q);
    reader.read();
    lastRefresh = QDateTime::currentDateTime();
    reply->deleteLater();
    applyTaskChanges();
    if (!reply->readOnly()) {
      updateSmartLists();
    } else {
      completeTaskChanges();
    }
  }

  void listUpdate(RTM::Request* reply) {
    TasksReader reader(reply, q);
    reader.read();
    reply->deleteLater();
    applyListChanges();
    if (reply->readOnly()) {
      completeListChanges();
    }
  }

  void applyTaskChanges() {
    foreach(RTM::Task* task, changedTasks) {
      tags.unite(task->tags().toSet());
      emit q->taskChanged(task);
    }

    if (changedTasks.count() > 0) {
      tasksChanged = true;
    }

    changedTasks.clear();
    qDebug() << "taskchanges complete, tags contains " << tags;
  }

  void completeTaskChanges() {
    if (tasksChanged) {
      emit q->tasksChanged();
    }

    tasksChanged = false;
  }

  void applyListChanges() {
    foreach(RTM::List* list, changedLists) {
      emit q->listChanged(list);
    }

    if (changedLists.count() > 0) {
      listsChanged = true;
    }

    changedLists.clear();
  }

   void completeListChanges() {
    if (listsChanged) {
      emit q->listsChanged();
    }

    listsChanged = false;
  }

  void applyChanges() {
    applyTaskChanges();
    completeTaskChanges();
    applyListChanges();
    completeListChanges();
  }

  void smartListReply(RTM::Request* reply) {
    QStringList parts = reply->requestUrl().split("&");
    RTM::ListId id = 0;
    foreach(const QString &part, parts)
      if (part.contains("list_id"))
        id = part.split("=").last().toLongLong();

    qDebug() << id;
    TasksReader reader(reply, q);
    reader.read();
    RTM::List* list = lists.value(id);
    if (list) {
      list->tasks.clear();
      foreach(RTM::Task* task, changedTasks) {
        list->tasks.insert(task->id(), task);
      }
      changedLists.push_back(list);
    }
    applyChanges();

    reply->deleteLater();
  }

  void settingsReply(RTM::Request* request) {
    QString reply = request->data(); // Get the full data of the reply, readAll() doesn't guarentee that.

    // We're basically assuming no error here.... FIXME
    QString timezone = reply.remove(0, reply.indexOf("<timezone>")+10);
    timezone.truncate(timezone.indexOf("</timezone>"));
    QString dateformat = reply.remove(0, reply.indexOf("<dateformat>"+12));
    dateformat.truncate(dateformat.indexOf("</dateformat>"));
    QString timeformat = reply.remove(0, reply.indexOf("<timeformat>"+12));
    timeformat.truncate(timeformat.indexOf("</timeformat>"));
    QString defaultlist = reply.remove(0, reply.indexOf("<defaultlist>"+13));
    defaultlist.truncate(defaultlist.indexOf("</defaultlist>"));

#ifndef QTONLY
    this->timezone = KSystemTimeZones::zone(timezone);
    qDebug() << "Timezone Set To: " << timezone << " i.e. " << this->timezone.name();
#endif

    request->deleteLater();
    emit q->settingsUpdated();
  }
  void refreshSettings() {
    if (!online)
      return;

    RTM::Request *settingsRequest = new RTM::Request("rtm.settings.getList", q->apiKey(), q->sharedSecret());
    settingsRequest->addArgument("auth_token", q->token());

    QObject::connect(settingsRequest, SIGNAL(replyReceived(RTM::Request*)), q, SLOT(settingsReply(RTM::Request*)));
    settingsRequest->sendRequest();
  }


  friend class TasksReader;
  friend class Session;
  Session *q;
  RTM::Auth *auth;
  QString authUrl;
  QString frob;
  QString apiKey;
  QString sharedSecret;
  QString token;
  QDateTime lastRefresh;
  bool online;
  RTM::Permissions permissions;
#ifndef QTONLY
  KTimeZone timezone;
#endif

  RTM::Timeline timeline;

  QHash<RTM::TaskId,RTM::Task*> tasks;
  QHash<RTM::ListId,RTM::List*> lists;
  QSet<QString> tags;

  QList<RTM::Task*> changedTasks;
  QList<RTM::List*> changedLists;

  bool tasksChanged;
  bool listsChanged;
};

#endif
