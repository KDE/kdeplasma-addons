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

#include <QtDebug>

RTM::Session::Session(QString apiKey, QString sharedSecret, RTM::Permissions permissions, QString token, QObject* parent)
  : QObject(parent),
    d(new SessionPrivate(this))
{
  d->apiKey = apiKey;
  d->sharedSecret = sharedSecret;
  d->permissions = permissions;

  connect(this, SIGNAL(tokenCheck(bool)), SLOT(handleValidToken(bool)));
  connect(this, SIGNAL(settingsUpdated()), SLOT(createTimeline()));

  setToken(token);
}

RTM::Session::~Session()
{
  delete d;
}

bool RTM::Session::currentlyOnline() const {
  return d->online;
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

QStringList RTM::Session::allTags() const {
  return d->tags.toList();
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
  if (d->token.isEmpty() || !currentlyOnline()) {
    emit tokenCheck(false);
    return;
  }

  RTM::Request *tokenRequest = new RTM::Request("rtm.auth.checkToken", d->apiKey, d->sharedSecret);
  d->connectOfflineSignal(tokenRequest);
  tokenRequest->addArgument("auth_token", d->token);
  connect(tokenRequest, SIGNAL(replyReceived(RTM::Request*)), SLOT(tokenCheckReply(RTM::Request*)));
  connect(tokenRequest, SIGNAL(replyReceived(RTM::Request*)), tokenRequest, SLOT(deleteLater()));
  tokenRequest->sendRequest();
}


void RTM::Session::tokenCheckReply(RTM::Request* response)
{
  QString reply = response->data();

  if (!reply.contains(d->token)) {
    qDebug() << "Failed Token Check: " << reply;
    emit tokenCheck(false);
  } else {
    qDebug() << "Successful Token Check: " << reply;
    emit tokenCheck(true);
  }
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
  qDebug() << "Timeline: " << timeline;
  d->timeline = timeline.toLong();
  d->lastRefresh = QDateTime();
  emit timelineCreated(getTimeline());
}



void RTM::Session::handleResponse()
{
    //TODO: Move data from buffer to response, then clear buffer.
}

void RTM::Session::refreshTasksFromServer() {
  if (!currentlyOnline())
    return;

  RTM::Request *allTasks = request("rtm.tasks.getList");
  if (d->lastRefresh.isValid())
    allTasks->addArgument("last_sync", d->lastRefresh.toUTC().toString(Qt::ISODate));
  allTasks->setReadOnly(false);
  allTasks->sendRequest();
}

void RTM::Session::refreshListsFromServer() {
  if (!currentlyOnline())
    return;

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
  d->connectOfflineSignal(request);
  request->addArgument("auth_token", token());
  connectTaskRequest(request);
  return request;
}

void RTM::Session::addTask(const QString& task, RTM::ListId listId)
{
  if (!currentlyOnline())
    return;

  RTM::List* list = listFromId(listId);

  qDebug() << "Adding Task: " << task << "to list with id: " << listId;
  RTM::Request *newTask = request("rtm.tasks.add"); // auth token is done for us
  newTask->addArgument("name", task);
  newTask->addArgument("parse", "1");
  if (list && !list->isSmart())
    newTask->addArgument("list_id", QString::number(listId));
  newTask->addArgument("timeline", QString::number(getTimeline()));
  newTask->setReadOnly(false);

  newTask->sendRequest();
}

void RTM::Session::addList(const QString &list, const QString &filter)
{
    if (!currentlyOnline())
        return;

    qDebug() << "Adding List: " << list << " to lists";
    RTM::Request *newList = request("rtm.lists.add");
    newList->addArgument("name", list);
    newList->addArgument("timeline", QString::number(getTimeline()));
    if (!filter.isEmpty())
        newList->addArgument("filter", filter);
    newList->setReadOnly(false);

    newList->sendRequest();
}

#include "moc_session.cpp"
