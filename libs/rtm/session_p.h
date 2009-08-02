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

#include <KSystemTimeZones>
#include <KTimeZone>
#include <KDebug>

class RTM::SessionPrivate {
  SessionPrivate(Session *parent)
    : q(parent),
      auth(0)
  {

  }
  ~SessionPrivate() {
    if (auth)
      auth->deleteLater();
  }

  void populateSmartList(List * list)
  {
    kDebug() << "Populating Smart List: " << list->name();
    // We do this next bit manually so it doesn't get auto-connected to taskUpdate()
    RTM::Request *smartListRequest = new RTM::Request("rtm.tasks.getList", q->apiKey(), q->sharedSecret());
    smartListRequest->addArgument("auth_token", q->token());
    smartListRequest->addArgument("rtm_internal_list_id", QString::number(list->id()));
    smartListRequest->addArgument("filter", list->filter());
    
    QObject::connect(smartListRequest, SIGNAL(replyReceived(RTM::Request*)), q, SLOT(smartListReply(RTM::Request*)));

    smartListRequest->sendRequest();
  }

  void taskUpdate(RTM::Request* reply) {
    TasksReader reader(reply, q);
    reader.read();
    lastRefresh = QDateTime::currentDateTime();
    reply->deleteLater();
  }

  void listUpdate(RTM::Request* reply) {
    TasksReader reader(reply, q);
    reader.read();
    reply->deleteLater();
  }

  void smartListReply(RTM::Request* reply) {
    QStringList parts = reply->requestUrl().split("&");
    RTM::ListId id = 0;
    foreach(const QString &part, parts)
      if (part.contains("rtm_internal_list_id"))
        id = part.split("=").last().toLongLong();
      
    kDebug() << id;
    TasksReader reader(reply, q);
    reader.read();
    RTM::List* list = lists.value(id);
    foreach(RTM::Task* task, reader.readTasks()) {
      list->tasks.insert(task->id(), task);
    }

    emit q->listChanged(list);
    emit q->listsChanged();
    
    reply->deleteLater();
  }
  void refreshSettings() {
    RTM::Request settingsRequest("rtm.settings.getList", q->apiKey(), q->sharedSecret());
    settingsRequest.addArgument("auth_token", q->token());
    
    QString reply = settingsRequest.sendSynchronousRequest();
    
    // We're basically assuming no error here.... FIXME
    QString timezone = reply.remove(0, reply.indexOf("<timezone>")+10);
    timezone.truncate(timezone.indexOf("</timezone>"));
    QString dateformat = reply.remove(0, reply.indexOf("<dateformat>"+12));
    dateformat.truncate(dateformat.indexOf("</dateformat>"));
    QString timeformat = reply.remove(0, reply.indexOf("<timeformat>"+12));
    timeformat.truncate(timeformat.indexOf("</timeformat>"));
    QString defaultlist = reply.remove(0, reply.indexOf("<defaultlist>"+13));
    defaultlist.truncate(defaultlist.indexOf("</defaultlist>"));
    
    this->timezone = KSystemTimeZones::zone(timezone);
    kDebug() << "Timezone Set To: " << timezone << " i.e. " << this->timezone.name();
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
  RTM::Permissions permissions;
  KTimeZone timezone;

  RTM::Timeline timeline;

  QHash<RTM::TaskId,RTM::Task*> tasks;
  QHash<RTM::ListId,RTM::List*> lists;
};

#endif
