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

#ifndef TASKSSERVICE_H
#define TASKSSERVICE_H

#include <Plasma/Service>
#include <Plasma/ServiceJob>

namespace RTM {
  class Session;
}

class TaskSource;

class TasksService : public Plasma::Service
{
public:
  TasksService(RTM::Session *session, QObject* parent);
protected:
  virtual Plasma::ServiceJob* createJob(const QString& operation, QMap< QString, QVariant >& parameters);
  TaskSource* m_source;
  RTM::Session* m_session;
};

class TasksJob : public Plasma::ServiceJob
{
  Q_OBJECT
public:
    TasksJob(RTM::Session* session, const QString& operation, QMap< QString, QVariant >& parameters, QObject* parent);
    void start();

private slots:
    void result();
    
protected:
    RTM::Session* m_session;
};

#endif // AUTHSERVICE_H
