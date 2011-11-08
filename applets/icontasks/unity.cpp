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

#include "unity.h"
#include "unityitem.h"
#include "tasks.h"
#include "abstracttaskitem.h"
#include <QtCore/QTimer>
#include <QtDBus/QDBusServiceWatcher>
#include <KDE/KGlobal>
#include <KDE/KSycoca>
#include <KDE/OrgKdeKLauncherInterface>

static const QString constDbusService = "com.canonical.Unity";
static const QString constDbusObject = "/Unity";
static const QString constDbusInterface = "com.canonical.Unity.LauncherEntry";

K_GLOBAL_STATIC(Unity, unity)

static QString urlToId(const KUrl &url)
{
    QString desktopEntry = url.fileName();
    if (desktopEntry.startsWith("kde4-")) {
        desktopEntry = desktopEntry.mid(5);
    }
    return QLatin1String("application://") + desktopEntry;
}

Unity * Unity::self()
{
    return unity;
}

Unity::Unity()
    : m_enabled(false)
    , m_connected(false)
    , m_watcher(0)
{
}

void Unity::setEnabled(bool en)
{
    if (en != m_enabled) {
        m_enabled = en;
        if (m_enabled) {
            if (QDBusConnection::sessionBus().registerService(constDbusService)) {
                if (QDBusConnection::sessionBus().registerObject(constDbusObject, this)) {
                    m_connected = true;
                    reloadItems();
                    QDBusConnection::sessionBus().connect(QString(), QString(), constDbusInterface, "Update",
                                                          this, SLOT(update(QString, QMap<QString, QVariant>)));
                    connect(KSycoca::self(), SIGNAL(databaseChanged(QStringList)), SLOT(sycocaChanged(QStringList)));
                } else {
                    kDebug() << "Failed to register unity object";
                }
            } else {
                kDebug() << "Failed to register unity service";
            }
        } else {
            if (m_connected) {
                QDBusConnection::sessionBus().unregisterService(constDbusService);
                QDBusConnection::sessionBus().unregisterObject(constDbusObject, QDBusConnection::UnregisterTree);
                QDBusConnection::sessionBus().disconnect(QString(), QString(), constDbusInterface, "Update",
                        this, SLOT(update(QString, QMap<QString, QVariant>)));
                disconnect(KSycoca::self(), SIGNAL(databaseChanged(QStringList)), this, SLOT(sycocaChanged(QStringList)));
            }
            QMap<QString, UnityItem *>::ConstIterator it(m_items.constBegin()),
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

void Unity::reloadItems()
{
    if (!m_connected || !m_enabled) {
        return;
    }

    QMap<QString, UnityItem *> existing = m_items;
    QMap<AbstractTaskItem *, KUrl>::ConstIterator taskIt(m_tasks.constBegin()),
         taskEnd(m_tasks.constEnd());

    for (; taskIt != taskEnd; ++taskIt) {
        QString id = urlToId(taskIt.value());
        if (m_items.contains(id)) {
            existing.remove(id);
        } else {
            UnityItem *item = new UnityItem(id, taskIt.value().toLocalFile());
            m_items.insert(id, item);
            item->registerTask(taskIt.key());
        }
    }

    QMap<QString, UnityItem *>::ConstIterator it(existing.constBegin()),
         end(existing.constEnd());

    for (; it != end; ++it) {
        QStringList services = m_itemService.keys(it.value());
        foreach (QString srv, services) {
            if (m_watcher) {
                m_watcher->removeWatchedService(srv);
            }
            m_itemService.remove(srv);
        }
        delete it.value();
        m_items.remove(it.key());
    }
}

void Unity::registerTask(AbstractTaskItem *item)
{
    if (!m_tasks.contains(item)) {
        KUrl url = item->launcherUrl();

        if (url.isValid()) {
            QString id = urlToId(url);

            m_tasks.insert(item, url);

            if (m_connected) {
                if (!m_items.contains(id)) {
                    m_items.insert(id, new UnityItem(id, url.toLocalFile()));
                }

                m_items[id]->registerTask(item);
            }
        }
    }
}

void Unity::unregisterTask(AbstractTaskItem *item)
{
    if (m_tasks.contains(item)) {
        QString id = urlToId(m_tasks[item]);

        if (m_connected) {
            // Remove the UnityItem if this task was not associated with a launcher...
            if (m_items.contains(id)) {
                m_items[id]->unregisterTask(item);
            }
        }
        m_tasks.remove(item);
    }
}

void Unity::remove(UnityItem *item)
{
    if (item) {
        if (m_items.contains(item->id())) {
            m_items.remove(item->id());
        }
        item->deleteLater();
        if (m_watcher) {
            foreach (QString srv, m_itemService.keys(item)) {
                m_watcher->removeWatchedService(srv);
            }
        }
    }
}

void Unity::itemService(UnityItem *item, const QString &serviceName)
{
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

void Unity::update(QString uri, QMap<QString, QVariant> properties)
{
    if (m_items.contains(uri)) {
        UnityItem *item = m_items[uri];
        QString sender = calledFromDBus() ? message().service() : QString();

        if (!sender.isEmpty()) {
            itemService(item, sender);
        }
        item->update(properties, sender);
    }
}

void Unity::serviceOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner)
{
    Q_UNUSED(oldOwner)

    if (newOwner.isEmpty() && m_itemService.contains(name)) {
        UnityItem *item = m_itemService[name];
        if (item) {
            item->reset();
        }
        m_itemService.remove(name);
    }
}

void Unity::sycocaChanged(const QStringList &types)
{
    if (types.contains("apps")) {
        QMap<QString, UnityItem *>::ConstIterator it(m_items.constBegin()),
             end(m_items.constEnd());

        for (; it != end; ++it) {
            (*it)->updateStaticMenu();
        }
    }
}

#include "unity.moc"
