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

#ifndef TASKSERVICE_H
#define TASKSERVICE_H

#include <Plasma/Service>
#include <Plasma/ServiceJob>

class TaskSource;

namespace RTM {
  class Session;
  class Task;
}

class TaskService : public Plasma::Service
{
public:
  TaskService(RTM::Session *session, RTM::Task* task, TaskSource* parent);

protected:
  virtual Plasma::ServiceJob* createJob(const QString& operation, QMap< QString, QVariant >& parameters);
  public:
  TaskSource* m_source;
  RTM::Task* m_task;
  RTM::Session* m_session;
};

class ModifyTaskJob : public Plasma::ServiceJob
{
  Q_OBJECT
    RTM::Session* m_session;
    RTM::Task* m_task;

public:
    ModifyTaskJob(RTM::Session* session, RTM::Task* source, const QString& operation, QMap< QString, QVariant >& parameters, QObject* parent);
    void start();

private slots:
    void result(RTM::Task *task);
};

#endif // TASKSERVICE_H
