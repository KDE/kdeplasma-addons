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

#ifndef __DOCKMANAGER_H__
#define __DOCKMANAGER_H__

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtDBus/QtDBus>
#include <KDE/KUrl>

class DockItem;
class DockHelper;
class DockConfig;
class AbstractTaskItem;
class KConfigDialog;
class KConfigGroup;
class QTimer;
class QDBusServiceWatcher;

class DockManager : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "net.launchpad.DockManager")

public:
    static DockManager * self();

    DockManager();

    void setEnabled(bool en);
    bool isEnabled() const {
        return m_enabled;
    }
    void reloadItems();
    void registerTask(AbstractTaskItem *item);
    void unregisterTask(AbstractTaskItem *item);
    void remove(DockItem *item);
    void itemService(DockItem *item, const QString &serviceName);
    QStringList dirs() const;
    const QSet<QString> enabledHelpers() const {
        return m_enabledHelpers;
    }

    void addConfigWidget(KConfigDialog *parent);
    void readConfig(KConfigGroup &cg);
    void writeConfig(KConfigGroup &cg);

private:
    bool stopDaemon();

public Q_SLOTS:
    void removeConfigWidget();

    Q_SCRIPTABLE QStringList            GetCapabilities();
    Q_SCRIPTABLE QDBusObjectPath        GetItemByXid(qlonglong xid);
    Q_SCRIPTABLE QList<QDBusObjectPath> GetItems();
    Q_SCRIPTABLE QList<QDBusObjectPath> GetItemsByDesktopFile(const QString &desktopFile);
    Q_SCRIPTABLE QList<QDBusObjectPath> GetItemsByName(QString name);
    Q_SCRIPTABLE QList<QDBusObjectPath> GetItemsByPid(int pid);

Q_SIGNALS:
    void ItemAdded(const QDBusObjectPath &path);
    void ItemRemoved(const QDBusObjectPath &path);

private Q_SLOTS:
    void updateHelpers();
    void updateHelpersDelayed();
    void serviceOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner);

private:
    bool m_enabled;
    bool m_connected;
    QMap<KUrl, DockItem *> m_items;
    QMap<QString, DockItem *> m_itemService;
    QMap<AbstractTaskItem *, KUrl> m_tasks;
    QList<DockHelper *> m_helpers;
    QSet<QString> m_enabledHelpers;
    QTimer *m_timer;
    DockConfig *m_config;
    QDBusServiceWatcher *m_watcher;
};

#endif

