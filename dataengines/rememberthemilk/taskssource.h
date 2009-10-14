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

#ifndef TASKSSOURCE_H
#define TASKSSOURCE_H

#include <rtm/rtm.h>
#include <rtm/task.h>
#include <rtm/list.h>

#include <QTimer>

#include <Plasma/DataContainer>
#include <Plasma/Service>
#include <Plasma/ServiceJob>

// forward declarations
class TaskSource;
class TasksSource;
class RtmEngine;

class TasksSource : public Plasma::DataContainer
{
    Q_OBJECT

public:

    TasksSource(RtmEngine* engine, RTM::Session* session);
    ~TasksSource();

    TaskSource* setupTaskSource(const QString& source);
    Plasma::Service *createService();
    RTM::Session* session() const { return m_session; }

public slots:
    void refresh();

private slots:
    void taskChanged(RTM::Task* task);
    void tasksChanged();

private:
    RtmEngine *m_engine;
    RTM::Session *m_session;
    QString m_cdata;
    Plasma::DataEngine::Data m_tempData;
    QString m_id;
    QTimer timer;
};

#endif

