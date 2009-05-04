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

#ifndef TASKSOURCE_H
#define TASKSOURCE_H

#include <Plasma/DataContainer>
#include <rtm/rtm.h>

namespace RTM {
class Task;
class Session;
}

class TaskSource : public Plasma::DataContainer
{
Q_OBJECT
public:
    TaskSource(RTM::TaskId id, RTM::Session *session, QObject* parent);
    ~TaskSource();

    Plasma::Service* createService();
  
public slots:
    void updateRequest(DataContainer *source);
    void update();

private:
  RTM::TaskId id;
  RTM::Session* session;
  RTM::Task* task;

};

#endif // TASKSOURCE_H
