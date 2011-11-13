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

#include "dockhelper.h"
#include "dockmanager.h"
#include <KDE/KConfig>
#include <KDE/KConfigGroup>
#include <QtCore/QFile>
#include <QtDBus/QDBusServiceWatcher>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#include <KDE/KDebug>
#include <KDE/KStandardDirs>

DockHelper::DockHelper(const QString &dir, const QString &fn)
    : m_fileName(fn)
    , m_dir(dir)
    , m_valid(false)
    , m_proc(0)
{

    if (QFile::exists(m_dir + "/metadata/" + m_fileName + ".info") && QFile::exists(m_dir + "/scripts/" + m_fileName)) {
        KConfig cfg(m_dir + "/metadata/" + m_fileName + ".info", KConfig::NoGlobals);

        if (cfg.hasGroup("DockmanagerHelper")) {
            KConfigGroup grp(&cfg, "DockmanagerHelper");
            QString appName = grp.readEntry("AppName", QString());
            m_dBusName = grp.readEntry("DBusName", QString());
            m_valid = appName.isEmpty() || !KStandardDirs::findExe(appName).isEmpty();

            if (m_valid) {
                if (m_dBusName.isEmpty()) {
                    start();
                } else {
                    QDBusServiceWatcher *watcher = new QDBusServiceWatcher(m_dBusName,
                            QDBusConnection::sessionBus(),
                            QDBusServiceWatcher::WatchForOwnerChange, this);
                    connect(watcher, SIGNAL(serviceOwnerChanged(QString, QString, QString)), SLOT(serviceOwnerChanged(QString, QString, QString)));
                    QDBusReply<bool> reply = QDBusConnection::sessionBus().interface()->isServiceRegistered(m_dBusName);
                    if (reply.isValid() && reply.value()) {
                        start();
                    }
                }
            }
        }
    }
}

DockHelper::~DockHelper()
{
    stop();
}

void DockHelper::serviceOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner)
{
    Q_UNUSED(name)
    Q_UNUSED(oldOwner)

    if (newOwner.isEmpty()) {
        stop();
    } else {
        start();
    }
}

void DockHelper::start()
{
    if (m_valid && !m_proc) {
        m_proc = new QProcess(this);
        m_proc->start(m_dir + "/scripts/" + m_fileName);
    }
}

void DockHelper::stop()
{
    if (m_proc) {
        m_proc->close();
        m_proc->deleteLater();
        m_proc = 0;
    }
}

#include "dockhelper.moc"
