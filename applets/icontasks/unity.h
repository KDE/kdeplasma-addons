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

#ifndef __UNITY_H__
#define __UNITY_H__

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtDBus/QtDBus>
#include <KDE/KUrl>

class AbstractTaskItem;
class UnityItem;
class QDBusServiceWatcher;

class Unity : public QObject, protected QDBusContext
{
    Q_OBJECT

public:
    static Unity * self();

    Unity();
    void setEnabled(bool en);
    bool isEnabled() const {
        return m_enabled;
    }
    void reloadItems();
    void registerTask(AbstractTaskItem *item);
    void unregisterTask(AbstractTaskItem *item);
    void remove(UnityItem *item);
    void itemService(UnityItem *item, const QString &serviceName);

private Q_SLOTS:
    void update(QString uri, QMap<QString, QVariant> properties);
    void serviceOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner);
    void sycocaChanged(const QStringList &types);

private:
    bool m_enabled;
    bool m_connected;
    QMap<QString, UnityItem *> m_items;
    QMap<QString, UnityItem *> m_itemService;
    QMap<AbstractTaskItem *, KUrl> m_tasks;
    QDBusServiceWatcher *m_watcher;
};

#endif
