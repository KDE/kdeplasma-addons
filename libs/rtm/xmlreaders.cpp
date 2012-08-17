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

#include "xmlreaders.h"

#include "note.h"
#include "request.h"

#include "session.h"
#include "session_p.h"

#include "task.h"
#include "task_p.h"

#include <QCoreApplication>
#include <QtDebug>
#include <QNetworkReply>


struct TempProps {
  QString name;
  RTM::TaskSeriesId seriesId;
  RTM::ListId listId;
  QHash<RTM::NoteId, RTM::Note> notes;
  QList<RTM::Tag> tags;
};


RTM::TasksReader::TasksReader(RTM::Request* r, RTM::Session* s)
  : QXmlStreamReader(r),
    session(s),
    request(r)
{
  Q_ASSERT(r);
  Q_ASSERT(s);

  request->open(QIODevice::ReadOnly);
  request->seek(0);
}


QDateTime RTM::TasksReader::parseDateTime(const QString& datetime)
{
  QDateTime offsetTime =  QDateTime::fromString(datetime, Qt::ISODate);
  return localizedTime(offsetTime);
}

QDateTime RTM::TasksReader::localizedTime(const QDateTime& datetime)
{
  QDateTime dt = QDateTime(datetime.date(), datetime.time(), Qt::LocalTime);
#ifndef QTONLY
  KTimeZone utc = KSystemTimeZones::zone("UTC");
  KTimeZone rtm = session->d->timezone;
  //qDebug() << datetime << dt << utc.convert(rtm, dt);
  return utc.convert(rtm, dt);
#else
  return datetime;
#endif
}

bool RTM::TasksReader::read() {
  while (!atEnd()) {
    readNext();
    // if (isEndElement())
       // Do I need to close/save my task?
    if (isStartElement()) {
      if (name().toString() == "rsp")
        readResponse();
      else
        readUnknownElement();
    }
  }

  this->device()->close();
  return true; // !error();
}


bool RTM::TasksReader::readResponse() {
  if (attributes().value("stat") != "ok") {
    //TODO: Provide more meaningful error
    return false;
  }

  while (!atEnd()) {
    readNext();
    if (isEndElement()) {
      return true;
    }
      
    if (isStartElement()) {
      if (name() == "tasks")
        readTasksHeader();
      else if (name() == "lists")
        readListsHeader();
      else if (name() == "transaction")
        readTransaction();
      else
        readUnknownElement();
    }
  }
  qDebug() << "Reached the end of readResponse() where we shouldn't have" << name().toString() << text().toString();
  qDebug() << "Attributes:";
  
  for(int i=0; i < attributes().count(); i++)
    { qDebug() << attributes().at(i).name().toString() << attributes().at(i).value().toString(); }
  return false;
}

void RTM::TasksReader::readTransaction() {
  // If we're not using a getList method we need to jump to the correct spot
  QStringList splitMethod = request->method().split('.');

  readNext();
    
  if (splitMethod.at(splitMethod.count() - 2) == "tasks")
    readTasksHeader();
  else if (splitMethod.at(splitMethod.count() - 2) == "lists")
    readListsHeader();
  else {
    qDebug() << "Unknown Method: " << splitMethod.join(".");
    readUnknownElement();
  }
}


void RTM::TasksReader::readUnknownElement() {
  qDebug() << "Unknown Element: " << tokenString() << name().toString() << text().toString();
  qDebug() << "Attributes:";
  
  for(int i=0; i < attributes().count(); i++)
    { qDebug() << attributes().at(i).name().toString() << attributes().at(i).value().toString(); }
  
  while(!atEnd()) {
    readNext();
    
    if (isEndElement())
      break;
 
    if (isStartElement())
      readUnknownElement();
  }
}

void RTM::TasksReader::readFilter(RTM::List* list) {
  list->setFilter(readElementText());
  qDebug() << "Filter for list: " << list->name() << " is " << list->filter();
//   while (!atEnd()) {
//     readNext();
//     if (isEndElement())
//       return;
//     if (isStartElement())
//       readUnknownElement();
//   }
}

void RTM::TasksReader::readList() {
  RTM::List *list = session->listFromId(attributes().value("id").toString().toULong());
  if (!list)
    list = session->newBlankList(attributes().value("id").toString().toULong());
  list->setId(attributes().value("id").toString().toULong());
  list->setName(attributes().value("name").toString());
  list->setSmart(attributes().value("smart").toString() == "1" ? true : false);
  list->setSortOrder(RTM::List::SortOrder(attributes().value("sort_order").toString().toUInt()));
  list->setPosition(attributes().value("position").toString().toULong());
  
  if (!session->d->changedLists.contains(list)) {
    session->d->changedLists.append(list);
  }
  
  while (!atEnd()) {
    readNext();
    if (isEndElement()) {
      session->d->lists.insert(list->id(), list);
      return;
    }
    if (isStartElement()) {
      if (name() == "filter")
        readFilter(list);
      else
        readUnknownElement();
    }
  }
}

void RTM::TasksReader::readListsHeader() {
  while (!atEnd()) {
    readNext();
    if (isEndElement()) {
      return;
    }
      
    if (isStartElement()) {
      if (name() == "list")
        readList();
      else
        readUnknownElement();
    }
  }
}


void RTM::TasksReader::readNotes(TempProps* props) {
  //qDebug() << "Notes not supported yet";
  if (isEndElement())
    return;

  while (!atEnd()) {
    readNext();
    if ((isEndElement()) && (name().toString() == "notes"))
      break;

    if (isEndElement())
      continue; // end of a note

    if ((isStartElement()) && (name().toString() == "note")) {
      RTM::Note note(attributes().value("id").toString().toULong(), attributes().value("title").toString(), readElementText());
      props->notes.insert(note.id(), note);
      }
    else
      readUnknownElement();
  }
}

void RTM::TasksReader::readParticipants(TempProps* props) {
  Q_UNUSED(props);
  //qDebug() << "Participants not supported yet";
  if (isEndElement())
    return;

  while (!atEnd()) {
    readNext();
    if ((isEndElement()) && (name().toString() == "participants"))
      break;
  }
}

void RTM::TasksReader::readTags(TempProps* props) {
  if (isEndElement())
    return;

  while (!atEnd()) {
    readNext();
    if ((isEndElement()) && (name().toString() == "tags"))
      break;
    if (isEndElement())
      continue;

    if ((isStartElement()) && (name().toString() == "tag"))
      props->tags.append(readElementText());
    else
      readUnknownElement();
  }
}

void RTM::TasksReader::readTask(TempProps *props) {  
  RTM::Task *task = session->taskFromId(attributes().value("id").toString().toLongLong());
  if (!task)
    task = session->newBlankTask(attributes().value("id").toString().toLongLong());

  task->d->name = props->name;
  task->d->seriesId = props->seriesId;
  task->d->listId = props->listId;

  task->d->notes = props->notes;
  task->d->tags = props->tags;
  
  RTM::List *list = session->listFromId(props->listId);
  if (!list)
    list = session->newBlankList(props->listId);

  if (!session->d->changedTasks.contains(task)) {
    session->d->changedTasks.append(task);
  }

  if (!session->d->changedLists.contains(list)) {
    session->d->changedLists.append(list);
  }

  // Grab ID
  task->d->taskId = attributes().value("id").toString().toULong();

  // Grab Priority
  if (attributes().value("priority") == "N")
    task->d->priority = 4;
  else
    task->d->priority = attributes().value("priority").toString().toInt();

  // Grab Due Date/Time
  task->d->due = parseDateTime(attributes().value("due").toString());
  // if (attributes().value("has_due_time") == "0") BUG: FIXME: Re-Implement time support

  // Grab Estimate
  task->d->estimate = attributes().value("estimate").toString();

  // Grab Completed/Deleted
  task->d->completed = parseDateTime(attributes().value("completed").toString());
  task->d->deleted = parseDateTime(attributes().value("deleted").toString());
  if (!task->d->deleted.isNull())
    task->setDeleted(true);

  // TODO:: Grab Postponed
  // TODO: Parse rest of fields

  //qDebug() << "Adding Task: " << task->id() << " to list " << list->id() << "(" << list << ")";
  if (!task->isDeleted())
    list->tasks.insert(task->id(), task);
  else
    list->tasks.remove(task->id());

  session->d->tasks.insert(task->id(), task);

  while (!atEnd()) {
    readNext();
    if (isEndElement())
      break;
    if (isStartElement()) 
      { qDebug() << "readTask().readNext(): " << name().toString(); }
    }
}

void RTM::TasksReader::readTaskSeries(RTM::ListId listId) {
  
  TempProps props;
  props.name = attributes().value("name").toString();
  props.seriesId = attributes().value("id").toString().toULong();
  props.listId = listId;

  while(!atEnd()) {
    readNext();
    
    if ((isEndElement()) && (name().toString() == "taskseries")) {
      break;
    }
    if (isEndElement()) {
      qDebug() << "Error in readTaskSeries() with end element: " << name().toString();
      break;
    }
    
    if (isStartElement()) {
      if (name().toString() == "tags")
        readTags(&props);
      else if (name().toString() == "participants")
        readParticipants(&props);
      else if (name().toString() == "notes")
        readNotes(&props);
      else if (name().toString() == "task")
        readTask(&props);
      else if (name().toString() == "tasks") // in case 'deleted' branch
        readTask(&props);
      else
        readUnknownElement();
    }
  }
  
}

void RTM::TasksReader::readTasksHeader() {
  while (!atEnd()) {
    readNext();
    if (isEndElement()) {
      return;
    }
    if (isStartElement()) {
      if (name() == "list")
        readTasksList();
      else
        readUnknownElement();
    }
  }
}

void RTM::TasksReader::readTasksList() {
  RTM::ListId currentListId = attributes().value("id").toString().toULong();
  while(!atEnd()) {
    readNext();
    if ((isEndElement()) && (name() == "list")) {
      break;
    }
    if (isEndElement()) {
      //Error in readTasksList() with end element: name()
      break;
    }
    
    if (isStartElement()) {
      if (name() == "taskseries")
        readTaskSeries(currentListId);
      else if (name() == "deleted")
        readDeleted(currentListId);
      else
        readUnknownElement();
    }
  }
}

void RTM::TasksReader::readDeleted(RTM::ListId listId) {
  while(!atEnd()) {
    readNext();
    if ((isEndElement()) && (name() == "deleted")) {
      break;
    }
    if (isEndElement()) {
      //Error in readDeleted() with end element: name()
      break;
    }

    if (isStartElement()) {
      if (name() == "taskseries")
        readTaskSeries(listId);
      else
        readUnknownElement();
    }
  }
}
