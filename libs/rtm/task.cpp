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

#include "task.h"
#include "task_p.h"

#include <KDebug>

#include "request.h"
#include "session.h"
#include "xmlreaders.h"

RTM::Task::Task(RTM::Session* session)
  : QObject(session),
    d(new TaskPrivate(this))
{
  d->session = session;
}

RTM::Task::~Task() {
  delete d;
} 

QDateTime RTM::Task::completed() const {
  return d->completed;
}
QDateTime RTM::Task::deleted() const {
  return d->deleted;
}
QDateTime RTM::Task::due() const {
  return d->due;
}
QString RTM::Task::estimate() const {
  return d->estimate;
}
RTM::TaskId RTM::Task::id() const {
  return d->taskId;
}
bool RTM::Task::isCompleted() const {
  return d->completed.isValid();
}
bool RTM::Task::isDeleted() const {
  return d->deleted.isValid();
}
RTM::ListId RTM::Task::listId() const {
  return d->listId;
}
RTM::LocationId RTM::Task::locationId() const {
  return d->locationId;
}
QString RTM::Task::name() const {
  return d->name;
}
Notes RTM::Task::notes() const {
  return d->notes;
}
int RTM::Task::priority() const {
  return d->priority;
}
QString RTM::Task::repeatString() const {
  return d->repeatString;
}
RTM::TaskSeriesId RTM::Task::seriesId() const {
  return d->seriesId;
}
QStringList RTM::Task::tags() const {
  return QStringList(d->tags);
}
QString RTM::Task::url() const {
  return d->url;
}

int RTM::Task::decreasePriority() {
  d->priority--;
  RTM::Request *request = d->standardRequest("rtm.tasks.movePriority");
  request->addArgument("direction", "down");
  request->sendRequest();
  return d->priority;
}
int RTM::Task::increasePriority() {
  d->priority++;
  RTM::Request *request = d->standardRequest("rtm.tasks.movePriority");
  request->addArgument("direction", "up");
  request->sendRequest();
  return d->priority;
}
void RTM::Task::setCompleted(bool completed) {
  if (isCompleted() == completed)
    return;

  RTM::Request *request;
  if (completed) {
    d->completed = QDateTime::currentDateTime();
    request = d->standardRequest("rtm.tasks.complete");
  }
  else {
    d->completed = QDateTime();
    request = d->standardRequest("rtm.tasks.uncomplete");
  }

  request->sendRequest();
}
void RTM::Task::setDeleted(bool deleted) {
  if (isCompleted() == deleted)
    return;

  RTM::Request *request;
  if (deleted) {
    d->deleted = QDateTime::currentDateTime();
    request = d->standardRequest("rtm.tasks.delete");
  }
  else {
    kDebug() << "ERROR: RTM Does not allow undeleting tasks!";
    d->deleted = QDateTime();
    request = d->standardRequest("rtm.tasks.undelete");
  }

  request->sendRequest();
}
void RTM::Task::setList(RTM::ListId listId) {
  RTM::ListId oldlist = d->listId;
  d->listId = listId;

  RTM::Request *request = d->session->request("rtm.tasks.moveTo");
  request->addArgument("timeline", QString::number(d->session->getTimeline()));
  request->addArgument("from_list_id", QString::number(oldlist));
  request->addArgument("to_list_id", QString::number(listId));
  request->addArgument("taskseries_id", QString::number(d->seriesId));
  request->addArgument("task_id", QString::number(d->taskId));
}
void RTM::Task::postpone() {
  //WARNING: Doesn't update task, only sends network request
  RTM::Request *request = d->standardRequest("rtm.tasks.postpone");
  request->sendRequest();
}
void RTM::Task::addTag(const RTM::Tag& tag) {
  if (d->tags.contains(tag))
    return;
  d->tags.append(tag);

  RTM::Request *request = d->standardRequest("rtm.tasks.addTags");
  request->addArgument("tags", tag);
  request->sendRequest();
}
void RTM::Task::removeAllTags() {
  d->tags.clear();
  RTM::Request *request = d->standardRequest("rtm.tasks.setTags");
  request->addArgument("tags", "");
  request->sendRequest();
}
bool RTM::Task::removeTag(const RTM::Tag& tag) {
  if (!d->tags.contains(tag))
    return false;
  d->tags.removeAll(tag);

  RTM::Request *request = d->standardRequest("rtm.tasks.removeTags");
  request->addArgument("tags", tag);
  request->sendRequest();
  return true;
}
void RTM::Task::setTags(const QStringList &tags) {
  RTM::Request *request = d->standardRequest("rtm.tasks.setTags");
  request->addArgument("tags", tags.join(","));
  request->sendRequest();
}
void RTM::Task::setNotes(const Notes& notes) {
  Q_UNUSED(notes)
  kError() << "NOT IMPLEMENTED"; //FIXME Implement
}
void RTM::Task::addNote(const QString& title, const QString& text) {
  //notes.insert(note.getId(), note);

  RTM::Request *request = d->standardRequest("rtm.tasks.notes.add");
  request->addArgument("note_title", title);
  request->addArgument("note_text", text);
  request->sendRequest();
}
bool RTM::Task::editNote(RTM::NoteId noteid, const QString& newTitle, const QString& newText) {
  if (!d->notes.contains(noteid))
    return false;
  
  RTM::Request *request = d->session->request("rtm.tasks.notes.edit"); // Only a simple request is needed
  request->addArgument("timeline", QString::number(d->session->getTimeline()));
  request->addArgument("note_id", QString::number(noteid));
  request->addArgument("note_title", newTitle);
  request->addArgument("note_text", newText);
  request->sendRequest();  
  return true;
}
void RTM::Task::removeAllNotes() {
  foreach(const RTM::NoteId &id, d->notes.keys())
    removeNote(id);
}
bool RTM::Task::removeNote(RTM::NoteId noteid) {
  if (!d->notes.contains(noteid))
    return false;
  
  RTM::Request *request = d->session->request("rtm.tasks.notes.delete"); // Only a simple request is needed
  request->addArgument("timeline", QString::number(d->session->getTimeline()));
  request->addArgument("note_id", QString::number(noteid));
  request->sendRequest();
  return true;
}
void RTM::Task::setDue(const QDateTime& due) {
  if (d->due == due)
    return;
  d->due = due;

  RTM::Request *request = d->standardRequest("rtm.tasks.setDueDate");
  if (due.isValid()) {
    request->addArgument("due", d->due.toString(Qt::ISODate));
    if (due.time().isValid())
      request->addArgument("has_due_time", "1");
  }
  request->sendRequest();
}
void RTM::Task::setDue(const QString& date) {
  RTM::Request *request = d->standardRequest("rtm.tasks.setDueDate");
  request->addArgument("due", date);
  request->addArgument("parse", "1");
  request->sendRequest();
}
void RTM::Task::setEstimate(const QString& estimate) {
  if (d->estimate == estimate)
    return;
  d->estimate == estimate;

  kDebug() << "Setting Estimate to: " << estimate;

  RTM::Request *request = d->standardRequest("rtm.tasks.setEstimate");
  request->addArgument("estimate", estimate);
  request->sendRequest();
}
void RTM::Task::setLocationId(RTM::LocationId locationid) {
  d->locationId = locationid;

  RTM::Request *request = d->standardRequest("rtm.tasks.setLocation");
  request->addArgument("location_id", QString::number(locationid));
}
void RTM::Task::setName(const QString& name) {
  if (d->name == name)
    return;
  d->name = name;

  RTM::Request *request = d->standardRequest("rtm.tasks.setName");
  request->addArgument("name", name);
  request->sendRequest();
}
void RTM::Task::setPriority(int priority) {
  if (d->priority == priority)
    return;
  d->priority = priority;

  RTM::Request *request = d->standardRequest("rtm.tasks.setPriority");
  request->addArgument("priority", QString::number(priority));
  request->sendRequest();
}
void RTM::Task::setRepeatString(const QString& repeatString) {
  if (d->repeatString == repeatString)
    return;
  d->repeatString = repeatString;

  RTM::Request *request = d->standardRequest("rtm.tasks.setRecurrence");
  request->addArgument("repeat", repeatString);
}
void RTM::Task::setUrl(const QString& url) {
  if (d->url == url)
    return;
  d->url = url;

  RTM::Request *request = d->standardRequest("rtm.tasks.setUrl");
  request->addArgument("url", url);
}

void RTM::Task::undoLastAction() {
  //TODO: Implement
}

RTM::Task* RTM::Task::createSyncTaskFromString(RTM::Session* session, const QString& task) {
  RTM::Request *request = session->request("rtm.tasks.add"); // auth token is done for us
  request->addArgument("name", task);
  request->addArgument("parse", "1");
  request->addArgument("timeline", QString::number(session->getTimeline()));
  request->sendSynchronousRequest();
  RTM::TasksReader reader(request, session);
  reader.read();
  //return reader.getReadTasks()->values().first();
  return new Task(session); //TODO: rewrite async.
}

RTM::Task* RTM::Task::uninitializedTask(RTM::Session* session) {
  return new Task(session);
}

#include "task.moc"
