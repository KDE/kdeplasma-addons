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

#ifndef RTM_TASK_P_H
#define RTM_TASK_P_H

#include "task.h"

#include "request.h"
#include "session.h"


class RTM::TaskPrivate {
    TaskPrivate(Task *parent)
      : q(parent)
    {

    }

    RTM::Request *standardRequest(const QString& method)
    {
      RTM::Request *request = session->request(method); // auth token is done for us
      request->addArgument("timeline", QString::number(session->getTimeline()));
      request->addArgument("list_id", QString::number(listId));
      request->addArgument("taskseries_id", QString::number(seriesId));
      request->addArgument("task_id", QString::number(taskId));
      return request;
    }

    friend class Task;
    friend class TasksReader;
    Task *q;

    RTM::Session *session;

    RTM::TaskId taskId;
    QList<Tag> tags;
    ListId listId;
    int priority;
    RTM::TaskSeriesId seriesId;
    QDateTime due;
    QDateTime completed;
    QDateTime deleted;
    QString estimate;
    LocationId locationId;
    QString name;
    QString repeatString;
    QString url;

    QHash<RTM::NoteId, RTM::Note> notes;

};

#endif // RTM_TASK_P_H
