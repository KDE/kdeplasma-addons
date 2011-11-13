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

#include "jobmanager.h"
#include "abstracttaskitem.h"
#include "tasks.h"
#include <KGlobal>
#include <Plasma/DataEngineManager>

K_GLOBAL_STATIC(JobManager, jobMgr)

static const char *constEngineName = "applicationjobs";

JobManager * JobManager::self()
{
    return jobMgr;
}

JobManager::JobManager()
    : m_engine(0)
{
}

JobManager::~JobManager()
{
}

void JobManager::setEnabled(bool enabled)
{
    if ((m_engine && !enabled) || (enabled && !m_engine)) {
        if (enabled) {
            m_engine = Plasma::DataEngineManager::self()->loadEngine(constEngineName);

            if (!m_engine->isValid()) {
                Plasma::DataEngineManager::self()->unloadEngine(constEngineName);
                m_engine = 0;
                return;
            }

            connect(m_engine, SIGNAL(sourceAdded(const QString)), this, SLOT(addJob(const QString)));
            connect(m_engine, SIGNAL(sourceRemoved(const QString)), this, SLOT(removeJob(const QString)));
            m_engine->connectAllSources(this);
        } else if (m_engine) {
            disconnect(m_engine, SIGNAL(sourceAdded(const QString)), this, SLOT(addJob(const QString)));
            disconnect(m_engine, SIGNAL(sourceRemoved(const QString)), this, SLOT(removeJob(const QString)));

            QMap<QString, QSet<QString> >::Iterator it(m_appJobs.begin()),
                 end(m_appJobs.end());

            for (; it != end; ++it) {
                foreach (const QString & job, *it) {
                    m_engine->disconnectSource(job, this);
                }
            }

            Plasma::DataEngineManager::self()->unloadEngine(constEngineName);
            m_appJobs.clear();
            m_jobs.clear();
            m_engine = 0;
        }
    }
}

void JobManager::registerTask(AbstractTaskItem *task)
{
    QString appName(task->appName());

    if (!appName.isEmpty()) {
        m_tasks[appName].append(task);

        if (m_appJobs.contains(appName)) {
            task->updateProgress(appProgress(appName));
        }
    }
}

void JobManager::unregisterTask(AbstractTaskItem *task)
{
    // Remove each reference to task...
    QMap<QString, QList<AbstractTaskItem *> >::Iterator it(m_tasks.begin()),
         end(m_tasks.end());
    QStringList                                         emptied;

    for (; it != end; ++it) {
        if ((*it).contains(task)) {
            (*it).removeAll(task);
            if (0 == (*it).count()) {
                emptied.append(it.key());
            }
        }
    }

    foreach (const QString & app, emptied) {
        m_tasks.remove(app);
    }
}

void JobManager::addJob(const QString &job)
{
    m_engine->connectSource(job, this);
}

void JobManager::dataUpdated(const QString &job, const Plasma::DataEngine::Data &data)
{
    QString appName = data["appName"].toString();

    if (appName.isEmpty()) {
        return;
    }

    int percentage = data.contains("percentage") ? data["percentage"].toInt() : -1;

    if (m_appJobs.contains(appName)) {
        m_appJobs[appName].insert(job);
    }

    m_jobs[job] = percentage;
    update(appName);
}

void JobManager::removeJob(const QString &job)
{
    m_jobs.remove(job);
    QMap<QString, QSet<QString> >::Iterator it(m_appJobs.begin()),
         end(m_appJobs.end());
    QStringList                             updated,
                                            emptied;

    for (; it != end; ++it) {
        if ((*it).contains(job)) {
            (*it).remove(job);
            if (0 == (*it).count()) {
                emptied.append(it.key());
            } else {
                updated.append(it.key());
            }
        }
    }

    foreach (const QString & app, emptied) {
        m_appJobs.remove(app);
        update(app);
    }

    foreach (const QString & app, updated) {
        update(app);
    }
}

int JobManager::appProgress(const QString &app)
{
    int numJobs = 0,
        total = 0;

    foreach (const QString & job, m_appJobs[app]) {
        int p = m_jobs[job];
        if (-1 != p) {
            numJobs++;
            total += p;
        }
    }

    return 0 == numJobs ? -1 : total / numJobs;
}

void JobManager::update(const QString &app)
{
    if (m_tasks.contains(app)) {
        int p = appProgress(app);

        foreach (AbstractTaskItem * item, m_tasks[app]) {
            item->updateProgress(p);
        }
    }
}

#include "jobmanager.moc"
