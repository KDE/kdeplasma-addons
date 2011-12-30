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

#include "dockmanager.h"
#include "dockmanageradaptor.h"
#include "dockitem.h"
#include "dockhelper.h"
#include "dockconfig.h"
#include "tasks.h"
#include "abstracttaskitem.h"
#include "windowtaskitem.h"
#include <KDE/KGlobal>
#include <KDE/KStandardDirs>
#include <KDE/KDirWatch>
#include <KDE/KConfigDialog>
#include <QtCore/QDir>
#include <QtCore/QTimer>
#include <QtCore/QThread>
#include <QtDBus/QDBusServiceWatcher>
#include <sys/types.h>
#include <signal.h>

static const QString constDbusService = "net.launchpad.DockManager";
static const QString constDbusObject = "/net/launchpad/DockManager";

static QString appFromPid(uint pid)
{
    QFile f(QString("/proc/%1/cmdline").arg(pid));

    if (f.open(QIODevice::ReadOnly)) {
        QByteArray bytes = f.read(1024);

        if (bytes.length() > 2) {
            return QString(bytes);
        }
    }

    return QString();
}

K_GLOBAL_STATIC(DockManager, dockMgr)

DockManager * DockManager::self()
{
    return dockMgr;
}

DockManager::DockManager()
    : m_enabled(false)
    , m_connected(false)
    , m_timer(0)
    , m_config(0)
    , m_watcher(0)
{
    new DockManagerAdaptor(this);
}

void DockManager::setEnabled(bool en)
{
    if (en != m_enabled) {
        m_enabled = en;
        if (m_enabled) {
            if (QDBusConnection::sessionBus().registerService(constDbusService)) {
                if (QDBusConnection::sessionBus().registerObject(constDbusObject, this)) {
                    if (stopDaemon()) {
                        m_connected = true;
                        reloadItems();
                        QTimer::singleShot(500, this, SLOT(updateHelpers()));
                        QStringList dirList = dirs();
                        foreach (QString dir, dirList) {
                            KDirWatch::self()->addDir(dir + "/scripts");
                            KDirWatch::self()->addDir(dir + "/metadata");
                        }
                        connect(KDirWatch::self(), SIGNAL(dirty(const QString&)), this, SLOT(updateHelpersDelayed()));
                    } else {
                        kDebug() << "Cannot start dock mamanger interface, failed to terminate dockamanger-daemon";
                    }
                } else {
                    kDebug() << "Failed to register dock mamanger object";
                }
            } else {
                kDebug() << "Failed to register dock mamanger service";
            }
        } else {
            if (m_connected) {
                QDBusConnection::sessionBus().unregisterService(constDbusService);
                QDBusConnection::sessionBus().unregisterObject(constDbusObject, QDBusConnection::UnregisterTree);
                // Allow dockmanager-daemon to run...
                QDBusConnection::sessionBus().unregisterService(constDbusService + ".Daemon");

                QStringList dirList = dirs();
                foreach (QString dir, dirList) {
                    KDirWatch::self()->removeDir(dir + "/scripts");
                    KDirWatch::self()->removeDir(dir + "/metadata");
                }
                disconnect(KDirWatch::self(), SIGNAL(dirty(const QString&)), this, SLOT(updateHelpersDelayed()));
                if (m_timer) {
                    m_timer->stop();
                }
            }
            foreach (DockHelper * helper, m_helpers) {
                delete helper;
            }
            m_helpers.clear();
            QMap<KUrl, DockItem *>::ConstIterator it(m_items.constBegin()),
                 end(m_items.constEnd());

            for (; it != end; ++it) {
                delete(*it);
            }
            m_items.clear();
            m_itemService.clear();
            if (m_watcher) {
                disconnect(m_watcher, SIGNAL(serviceOwnerChanged(QString, QString, QString)), this, SLOT(serviceOwnerChanged(QString, QString, QString)));
                m_watcher->deleteLater();
                m_watcher = 0;
            }
            // **Don't clear tasks** these will be neeaded if re-enable...
        }
    }
}

struct Thread : public QThread {
public:
    static void msleep(unsigned long ms) {
        QThread::msleep(ms);
    }
};

bool DockManager::stopDaemon()
{
    QDBusReply<uint> reply = QDBusConnection::sessionBus().interface()->servicePid(constDbusService + ".Daemon");

    if (reply.isValid()) {
        uint pid = reply.value();

        if (pid > 0) {
            if (appFromPid(pid).endsWith("dockmanager-daemon")) {
                kDebug() << "Stopping dockmanager-daemon, pid" << pid;
                if (::kill(pid, SIGTERM)) {
                    return false;
                } else {
                    Thread::msleep(250);
                }
            } else {
                return false;
            }
        }
    }
    // Now register the service for ourselces, to prevent it starting...
    QDBusConnection::sessionBus().registerService("net.launchpad.DockManager.Daemon");
    return true;
}

void DockManager::reloadItems()
{
    if (!m_connected || !m_enabled) {
        return;
    }

    QMap<KUrl, DockItem *> existing = m_items;
    QMap<AbstractTaskItem *, KUrl>::ConstIterator taskIt(m_tasks.constBegin()),
         taskEnd(m_tasks.constEnd());

    for (; taskIt != taskEnd; ++taskIt) {
        if (m_items.contains(taskIt.value())) {
            existing.remove(taskIt.value());
        } else {
            DockItem *item = new DockItem(taskIt.value());
            m_items.insert(taskIt.value(), item);
            emit ItemAdded(QDBusObjectPath(item->path()));
            item->registerTask(taskIt.key());
        }
    }

    QMap<KUrl, DockItem *>::ConstIterator it(existing.constBegin()),
         end(existing.constEnd());

    for (; it != end; ++it) {
        QStringList services = m_itemService.keys(it.value());
        foreach (QString srv, services) {
            if (m_watcher) {
                m_watcher->removeWatchedService(srv);
            }
            m_itemService.remove(srv);
        }
        emit ItemRemoved(QDBusObjectPath(it.value()->path()));
        delete it.value();
        m_items.remove(it.key());
    }
}

void DockManager::registerTask(AbstractTaskItem *item)
{
    if (!m_tasks.contains(item)) {
        KUrl url = item->launcherUrl();

        if (url.isValid()) {
            m_tasks.insert(item, url);

            if (m_connected) {
                if (!m_items.contains(url)) {
                    DockItem *item = new DockItem(url);
                    m_items.insert(url, item);
                    emit ItemAdded(QDBusObjectPath(item->path()));
                }

                m_items[url]->registerTask(item);
            }
        }
    }
}

void DockManager::unregisterTask(AbstractTaskItem *item)
{
    if (m_tasks.contains(item)) {
        KUrl url = m_tasks[item];

        if (m_connected) {
            // Remove the DockItem if this task was not associated with a launcher...
            if (url.isValid() && m_items.contains(url)) {
                m_items[url]->unregisterTask(item);
            }
        }
        m_tasks.remove(item);
    }
}

void DockManager::remove(DockItem *item)
{
    if (item) {
        emit ItemRemoved(QDBusObjectPath(item->path()));
        if (m_items.contains(item->url())) {
            m_items.remove(item->url());
        }
        item->deleteLater();
        if (m_watcher) {
            foreach (QString srv, m_itemService.keys(item)) {
                m_watcher->removeWatchedService(srv);
            }
        }
    }
}

void DockManager::itemService(DockItem *item, const QString &serviceName)
{
    if (m_watcher && m_watcher->watchedServices().contains(serviceName)) {
        return;
    }

    QDBusReply<uint> reply = QDBusConnection::sessionBus().interface()->servicePid(serviceName);
    uint servicePid = reply.isValid() ? reply.value() : 0;
    bool watchService = false;

    if (0 != servicePid) {
        foreach (DockHelper * helper, m_helpers) {
            if (helper->pid() == servicePid) {
                watchService = true;
                break;
            }
        }
    }

    if (!watchService) { // .desktop
        return;
    }
    if (m_watcher) {
        QStringList old = m_itemService.keys(item);
        if (old.count()) {
            foreach (QString srv, old) {
                m_watcher->removeWatchedService(srv);
            }
        }
    }

    if (!m_watcher) {
        m_watcher = new QDBusServiceWatcher(this);
        m_watcher->setConnection(QDBusConnection::sessionBus());
        m_watcher->setWatchMode(QDBusServiceWatcher::WatchForOwnerChange);
        connect(m_watcher, SIGNAL(serviceOwnerChanged(QString, QString, QString)), this, SLOT(serviceOwnerChanged(QString, QString, QString)));
    }

    m_watcher->addWatchedService(serviceName);
    m_itemService[serviceName] = item;
}

QStringList DockManager::GetCapabilities()
{
    return QStringList()
//                          << "dock-item-message"
//                          << "dock-item-tooltip"
           << "dock-item-badge"
           << "dock-item-progress"
//                          << "dock-item-visible"
           << "dock-item-icon-file"
//                          << "dock-item-attention"
//                          << "dock-item-waiting"
           << "x-kde-dock-item-overlay"
           << "menu-item-with-label"
//                          << "menu-item-with-uri"
           << "menu-item-icon-name"
           << "menu-item-icon-file"
           << "menu-item-container-title";
}

QDBusObjectPath DockManager::GetItemByXid(qlonglong xid)
{
    QMap<AbstractTaskItem *, KUrl>::ConstIterator it(m_tasks.constBegin()),
         end(m_tasks.constEnd());

    for (; it != end; ++it) {
        if (TaskManager::TaskItemType == it.key()->abstractItem()->itemType()) {
            WindowTaskItem *item = static_cast<WindowTaskItem *>(it.key());
            if (item->windowTask() && item->windowTask()->window() == xid) {
                if (m_items.contains(it.value())) {
                    return QDBusObjectPath(m_items[it.value()]->path());
                }
            }
        }
    }
    return QDBusObjectPath();
}

QList<QDBusObjectPath> DockManager::GetItems()
{
    QList<QDBusObjectPath> items;

    QMap<KUrl, DockItem *>::ConstIterator it(m_items.constBegin()),
         end(m_items.constEnd());

    for (; it != end; ++it) {
        items.append(QDBusObjectPath((*it)->path()));
    }
    return items;
}

QList<QDBusObjectPath> DockManager::GetItemsByDesktopFile(const QString &desktopFile)
{
    QList<QDBusObjectPath> items;

    QMap<KUrl, DockItem *>::ConstIterator it(m_items.constBegin()),
         end(m_items.constEnd());

    for (; it != end; ++it) {
        if ((*it)->DesktopFile() == desktopFile) {
            items.append(QDBusObjectPath((*it)->path()));
        }
    }
    return items;
}

QList<QDBusObjectPath> DockManager::GetItemsByName(QString name)
{
    QList<QDBusObjectPath> items;

    QMap<KUrl, DockItem *>::ConstIterator it(m_items.constBegin()),
         end(m_items.constEnd());

    for (; it != end; ++it) {
        if ((*it)->name() == name) {
            items.append(QDBusObjectPath((*it)->path()));
        }
    }
    return items;
}

QList<QDBusObjectPath> DockManager::GetItemsByPid(int pid)
{
    QList<QDBusObjectPath> items;

    QMap<AbstractTaskItem *, KUrl>::ConstIterator it(m_tasks.constBegin()),
         end(m_tasks.constEnd());

    for (; it != end; ++it) {
        if (TaskManager::TaskItemType == it.key()->abstractItem()->itemType()) {
            WindowTaskItem *item = static_cast<WindowTaskItem *>(it.key());

            if (item->windowTask() && item->windowTask()->pid() == pid) {
                if (m_items.contains(it.value())) {
                    items.append(QDBusObjectPath(m_items[it.value()]->path()));
                }
            }
        }
    }
    return items;
}

QStringList DockManager::dirs() const
{
    return QStringList() << QString(KGlobal::dirs()->localxdgdatadir() + "/dockmanager").replace("//", "/")
           << "/usr/local/share/dockmanager"
           << "/usr/share/dockmanager";
}

void DockManager::addConfigWidget(KConfigDialog *parent)
{
    if (!m_config) {
        m_config = new DockConfig(parent);
        connect(parent, SIGNAL(cancelClicked()), this, SLOT(removeConfigWidget()));
    }
}

void DockManager::readConfig(KConfigGroup &cg)
{
    KConfigGroup dm(&cg, "DockManager");

    QSet<QString> prevHelpers = m_enabledHelpers;
    m_enabledHelpers = dm.readEntry("EnabledHelpers", QStringList()).toSet();
    setEnabled(dm.readEntry("Enabled", true));

    if (m_enabled && prevHelpers != m_enabledHelpers) {
        updateHelpers();
    }
}

void DockManager::writeConfig(KConfigGroup &cg)
{
    if (m_config) {
        KConfigGroup dm(&cg, "DockManager");
        QSet<QString> prevHelpers = m_enabledHelpers;

        m_enabledHelpers = m_config->enabledHelpers();
        setEnabled(m_config->isEnabled());
        dm.writeEntry("Enabled", m_enabled);
        dm.writeEntry("EnabledHelpers", m_enabledHelpers.toList());

        if (m_enabled && prevHelpers != m_enabledHelpers) {
            updateHelpers();
        }
        removeConfigWidget();
    }
}

void DockManager::removeConfigWidget()
{
    // Don't delete m_config, as its now owned ny the config dialog...
    m_config = 0;
}

void DockManager::updateHelpersDelayed()
{
    if (!m_timer) {
        m_timer = new QTimer(this);
        connect(m_timer, SIGNAL(timeout()), this, SLOT(updateHelpers()));
    }
    m_timer->start(500);
}

void DockManager::updateHelpers()
{
    if (m_timer) {
        m_timer->stop();
    }

    if (!m_enabled || !m_connected) {
        return;
    }
    QStringList dirList = dirs();
    QMap<QString, DockHelper *> previousHelpers;
    QList<DockHelper *> newHelpers;

    foreach (DockHelper * helper, m_helpers) {
        previousHelpers[helper->dirName() + "/scripts/" + helper->fileName()] = helper;
    }

    foreach (QString dir, dirList) {
        QDir d(dir + "/metadata");
        QStringList metas = QDir(QString(dir + "/metadata")).entryList(QStringList() << "*.info");

        foreach (QString m, metas) {
            QString name = m.left(m.length() - 5);
            QString script = dir + "/scripts/" + name;
            if (previousHelpers.contains(script)) {
                if (m_enabledHelpers.contains(script)) {
                    previousHelpers.remove(script);
                }
            } else if (m_enabledHelpers.contains(script)) {
                DockHelper *helper = new DockHelper(dir, name);

                if (*helper) {
                    newHelpers.append(helper);
                } else {
                    delete helper;
                }
            }
        }
    }

    QMap<QString, DockHelper *>::ConstIterator it(previousHelpers.constBegin()),
         end(previousHelpers.constEnd());

    for (; it != end; ++it) {
        it.value()->stop();
        it.value()->deleteLater();
        m_helpers.removeAll(it.value());
    }

    foreach (DockHelper * helper, newHelpers) {
        m_helpers.append(helper);
    }
}

void DockManager::serviceOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner)
{
    Q_UNUSED(oldOwner)

    if (newOwner.isEmpty() && m_itemService.contains(name)) {
        DockItem *item = m_itemService[name];
        if (item) {
            item->reset();
        }
        m_itemService.remove(name);
    }
}

#include "dockmanager.moc"
