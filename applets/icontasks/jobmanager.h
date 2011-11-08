#ifndef _JOB_MANAGER_H__
#define _JOB_MANAGER_H__

/*
 * Icon Task Manager
 *
 * Copyright 2011 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QSet>
#include <Plasma/DataEngine>

class AbstractTaskItem;

class JobManager : public QObject
{
    Q_OBJECT

public:

    static JobManager * self();

    JobManager();
    ~JobManager();

    void setEnabled(bool enabled);
    bool isEnabled() const {
        return 0 != m_engine;
    }
    void registerTask(AbstractTaskItem *task);
    void unregisterTask(AbstractTaskItem *task);

private Q_SLOTS:

    void addJob(const QString &job);
    void dataUpdated(const QString &job, const Plasma::DataEngine::Data &data);
    void removeJob(const QString &job);

private:

    int appProgress(const QString &app);
    void update(const QString &app);

private:

    Plasma::DataEngine *m_engine;
    QMap<QString, QSet<QString> > m_appJobs; // Map from appName to list of job names
    QMap<QString, int> m_jobs;            // Map from job name to job percentage
    QMap<QString, QList<AbstractTaskItem *> > m_tasks;
};

#endif
