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

#ifndef XMLREADERS_H
#define XMLREADERS_H

namespace RTM {
  class Request;
  class Session;
  class Task;
}

#include <QXmlStreamReader>

#include "rtm.h"
#include "list.h"

class TempProps;

namespace RTM {

class RTM_EXPORT TasksReader : public QXmlStreamReader
{
public:
  TasksReader(RTM::Request *request, RTM::Session *session);
  
  bool read();
  QList<RTM::List*> readLists() const;
  QList<RTM::Task*> readTasks() const;

private:
  Session * const session;
  Request * const request;
  
  QDateTime parseDateTime(const QString &datetime);
  QDateTime localizedTime(const QDateTime &datetime);

  void readUnknownElement();
  bool readResponse();
  void readTransaction();

  void readTasksHeader();
    void readTasksList();
      void readTaskSeries(RTM::ListId listId);
        void readTags(TempProps* props);
        void readParticipants(TempProps* props);
        void readNotes(TempProps* props);
        void readTask(TempProps* props);

  void readListsHeader();
    void readList();
      void readFilter(RTM::List* list);

  QList<RTM::Task*> changedTasks;
  QList<RTM::List*> changedLists;
};

} // Namespace RTM
#endif // TASKSREADER_H
