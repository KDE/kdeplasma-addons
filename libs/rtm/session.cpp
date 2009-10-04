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

#include "session.h"
#include "session_p.h"

#include "xmlreaders.h"
#include "request.h"
#include "auth.h"
#include "task.h"

#include <QUrl>
#include <QStringList>
#include <QByteArray>
#include <QCryptographicHash>
#include <QBuffer>
#include <QCoreApplication>

#include <QDomDocument>
#include <QDomNode>
#include <QDomElement>
#include <QDomNodeList>

#include <KDebug>

RTM::Session::Session(QString apiKey, QString sharedSecret, RTM::Permissions permissions, QString token, QObject* parent)
  : QObject(parent),
    d(new SessionPrivate(this))
{
  d->apiKey = apiKey;
  d->sharedSecret = sharedSecret;
  d->permissions = permissions;
  
  connect(this, SIGNAL(tokenCheck(bool)), SLOT(handleValidToken(bool)));
  
  setToken(token);
}

RTM::Session::~Session()
{
  delete d;
}

void RTM::Session::showLoginWindow() {
  //FIXME: What happens when auth wasn't created?
  if (!d->auth)
    d->auth = new RTM::Auth(d->permissions, d->apiKey, d->sharedSecret);
  d->auth->showLoginWebpage();
}



QString RTM::Session::getAuthUrl() const {
  return d->authUrl;
}

bool RTM::Session::authenticated() const {
  return !d->token.isEmpty();
}

QString RTM::Session::apiKey() const {
  return d->apiKey;
}

QHash< RTM::ListId, RTM::List* > RTM::Session::cachedLists() const {
  return d->lists;
}

QHash< RTM::TaskId, RTM::Task* > RTM::Session::cachedTasks() const {
  return d->tasks;
}

RTM::Permissions RTM::Session::permissions() const {
  return d->permissions;
}

QString RTM::Session::sharedSecret() const {
  return d->sharedSecret;
}

RTM::Timeline RTM::Session::getTimeline() const {
  return d->timeline;
}
  
QString RTM::Session::token() const {
  return d->token;
}

void RTM::Session::setToken(const QString& token)
{
  d->token = token;
  d->tasks.clear(); //FIXME: Leak? Tasks/Lists are pointers.
  d->lists.clear();
  
  checkToken();
}

void RTM::Session::handleValidToken(bool valid)
{
  if (d->auth) {
    d->auth->deleteLater();
    d->auth = 0;
  }
  if (!valid) {
    d->token.clear();
    d->auth = new RTM::Auth(d->permissions, d->apiKey, d->sharedSecret);
    d->authUrl = d->auth->getAuthUrl();
    connect(d->auth, SIGNAL(tokenReceived(QString)), this, SLOT(setToken(QString)));
    connect(d->auth, SIGNAL(tokenReceived(QString)), this, SIGNAL(tokenReceived(QString)));
  }
  else {
    d->refreshSettings();
    createTimeline();
  }
}


void RTM::Session::setTimeline(const RTM::Timeline& timeline) {
  d->timeline = timeline;
}

void RTM::Session::continueAuthForToken()
{
  d->auth->continueAuthForToken();
}

void RTM::Session::checkToken() {
  if (d->token.isEmpty()) {
    emit tokenCheck(false);
    return;
  }
  RTM::Request *tokenRequest = new RTM::Request("rtm.auth.checkToken", d->apiKey, d->sharedSecret);
  tokenRequest->addArgument("auth_token", d->token);
  connect(tokenRequest, SIGNAL(replyReceived(RTM::Request*)), SLOT(tokenCheckReply(RTM::Request*)));
  connect(tokenRequest, SIGNAL(replyReceived(RTM::Request*)), tokenRequest, SLOT(deleteLater()));
  tokenRequest->sendRequest();
}


void RTM::Session::tokenCheckReply(RTM::Request* response)
{
  QString reply = response->data();
  
  if (!reply.contains(d->token)) {
    kDebug() << "Failed Token Check: " << reply;
    emit tokenCheck(false);
  }
  kDebug() << "Successful Token Check: " << reply;
  emit tokenCheck(true);
}


void RTM::Session::createTimeline() {
  RTM::Request *request = new RTM::Request("rtm.timelines.create", d->apiKey, d->sharedSecret);
  request->addArgument("auth_token", d->token);
  connect(request, SIGNAL(replyReceived(RTM::Request*)), SLOT(timelineReply(RTM::Request*)));
  connect(request, SIGNAL(replyReceived(RTM::Request*)), request, SLOT(deleteLater()));
  request->sendRequest();
}

void RTM::Session::timelineReply(RTM::Request* response)
{
  QString reply = response->data();
  QString timeline = reply.remove(0, reply.indexOf("<timeline>")+10);
  timeline.truncate(timeline.indexOf("</timeline>"));
  kDebug() << "Timeline: " << timeline;
  d->timeline = timeline.toLong();
  d->lastRefresh = QDateTime();
  emit timelineCreated(getTimeline());
}



void RTM::Session::handleResponse()
{
    //TODO: Move data from buffer to response, then clear buffer.
}

void RTM::Session::refreshTasksFromServer() {
  RTM::Request *allTasks = request("rtm.tasks.getList");
  if (d->lastRefresh.isValid())
    allTasks->addArgument("last_sync", d->lastRefresh.toUTC().toString(Qt::ISODate));
  connectTaskRequest(allTasks);
  allTasks->sendRequest();
}

void RTM::Session::refreshListsFromServer() {
  RTM::Request *allLists = new RTM::Request("rtm.lists.getList", d->apiKey, d->sharedSecret);
  allLists->addArgument("auth_token", d->token);
  connectListRequest(allLists);
  allLists->sendRequest();
}

RTM::Task* RTM::Session::taskFromId(RTM::TaskId id) const {
  if (cachedTasks().contains(id))
    return cachedTasks().value(id);
  return 0;
}

RTM::Task* RTM::Session::newBlankTask(RTM::TaskId id) const {
  RTM::Task *newTask = Task::uninitializedTask(d->q); //HACK d->q == this but not const
  d->tasks.insert(id, newTask);
  return newTask;
}

RTM::List* RTM::Session::listFromId(RTM::ListId id) const {
  if (cachedLists().contains(id))
    return cachedLists().value(id);
  return 0;
}

RTM::List* RTM::Session::newBlankList(RTM::ListId id) const {
  RTM::List *newList = List::uninitializedList(d->q); //HACK d->q == this but not const
  d->lists.insert(id, newList);
  return newList;
}

void RTM::Session::connectTaskRequest(RTM::Request* request) {
  connect(request, SIGNAL(replyReceived(RTM::Request*)), this, SLOT(taskUpdate(RTM::Request*)));
}

void RTM::Session::connectListRequest(RTM::Request* request) {
  connect(request, SIGNAL(replyReceived(RTM::Request*)), this, SLOT(listUpdate(RTM::Request*)));
}

RTM::Request* RTM::Session::request(const QString& method) {
  RTM::Request *request = new RTM::Request(method, apiKey(), sharedSecret());
  request->addArgument("auth_token", token());
  connectTaskRequest(request);
  return request;
}

void RTM::Session::addTask(const QString& task, RTM::ListId listId)
{
  kDebug() << "Adding Task: " << task << "to list with id: " << listId;
  RTM::Request *newTask = request("rtm.tasks.add"); // auth token is done for us
  newTask->addArgument("name", task);
  newTask->addArgument("parse", "1");
  newTask->addArgument("list_id", QString::number(listId));
  newTask->addArgument("timeline", QString::number(getTimeline()));
  
  connectTaskRequest(newTask);
  newTask->sendRequest();
}

RTM::Task* RTM::Session::createTaskFromString(const QString& task) {
  RTM::Task *newTask = RTM::Task::createSyncTaskFromString(this, task);
  d->tasks.insert(newTask->id(), newTask);
  return newTask;
}

#include "session.moc"
