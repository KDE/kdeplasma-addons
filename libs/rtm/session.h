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

#ifndef RTM_SESSION_H
#define RTM_SESSION_H

#include <QObject>
#include <QDateTime>

#include "rtm.h"
#include "task.h"

namespace RTM {
  class Request;
  class SessionPrivate;
  class TasksReader;
}


/** @file
 * This file is part of librtm. It defines
 * the RTM::Session which is the main high-level
 * interface to a Remember The Milk session
 *
 * @author Andrew Stromme <astromme@chatonka.com>
 */

/** 
 * Provides a set of classes for interacting with the Remember The Milk online todo management service
 */
namespace RTM {

/**
 * @brief The Session class provides a high level interface to a Remember The Milk session
 *
 * @author Andrew Stromme <astromme@chatonka.com>
 */
class RTM_EXPORT Session : public QObject
{
Q_OBJECT
  public:
    Session(QString apiKey, QString sharedSecret, RTM::Permissions permissions, QString token = QString(), QObject *parent = 0);
    virtual ~Session();

    QString getAuthUrl() const;
    void showLoginWindow();
    bool authenticated() const;

    RTM::Request* request(const QString& method);
    void connectTaskRequest(RTM::Request *request);
    void connectListRequest(RTM::Request *request);

    void setTimeline(const RTM::Timeline& timeline);
    RTM::Timeline getTimeline() const;
    void createTimeline();

    void checkToken();

    QString apiKey() const;
    QString sharedSecret() const;
    QString token() const;
    RTM::Permissions permissions() const;

    void refreshTasksFromServer();
    void refreshListsFromServer();
    QHash<RTM::TaskId,RTM::Task*> cachedTasks() const;
    QHash<RTM::ListId,RTM::List*> cachedLists() const;

    RTM::Task* taskFromId(RTM::TaskId id) const;
    RTM::Task* newBlankTask(RTM::TaskId id) const;
    
    RTM::List* listFromId(RTM::ListId id) const;
    RTM::List* newBlankList(RTM::ListId id) const;

  public Q_SLOTS:
    void setToken(const QString &token);
    void handleResponse();
    void continueAuthForToken();
    void addTask(const QString &task, RTM::ListId listId);
    RTM::Task* createTaskFromString(const QString& task);
    void tokenCheckReply(RTM::Request*);
    void handleValidToken(bool);
    void timelineReply(RTM::Request*);

  Q_SIGNALS:
    void tokenReceived(const QString& token);
    void tokenCheck(bool success);
    
    void timelineCreated(RTM::Timeline timeline);

    void taskChanged(RTM::Task* task);
    void listChanged(RTM::List* list);
  
    void tasksChanged();
    void listsChanged();

  private:
    friend class TasksReader;
    friend class SessionPrivate;
    SessionPrivate * const d;

    Q_PRIVATE_SLOT(d, void taskUpdate(RTM::Request* reply))
    Q_PRIVATE_SLOT(d, void listUpdate(RTM::Request* reply))
    Q_PRIVATE_SLOT(d, void smartListReply(RTM::Request* reply))
};

} // Namespace RTM
#endif
