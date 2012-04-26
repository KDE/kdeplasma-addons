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

#ifndef __UNITY_ITEM_H__
#define __UNITY_ITEM_H__

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QSet>
#include <QtCore/QMap>
#include <QtCore/QVariant>
#include <QtCore/QString>

#include <KDE/KConfigGroup>

class AbstractTaskItem;
class QTimer;
class QAction;
class DBusMenuImporter;

class UnityItem : public QObject
{
    Q_OBJECT

public:
    UnityItem(const QString &i, const QString &desktopFile = QString());
    ~UnityItem();

    void reset();
    void updateStaticMenu();
    void update(QMap<QString, QVariant> properties, const QString &sender);
    QString id() const {
        return m_id;
    }
    double progress() const {
        return m_progress;
    }
    bool progressVisible() const {
        return m_progressVisible;
    }
    unsigned int count() const {
        return m_count;
    }
    bool countVisible() const {
        return m_countVisible;
    }
    void registerTask(AbstractTaskItem *item);
    void unregisterTask(AbstractTaskItem *item);
    QList<QAction *> menu();

public Q_SLOTS:
    void check();
    void menuActivated();

private:
    void readStaticMenu();
    void parseDesktopAction(const KConfigGroup& grp);

private:
    QString m_id;
    QString m_desktopFile;
    int m_staticDirty;
    bool m_progressVisible;
    bool m_countVisible;
    unsigned int m_count;
    int m_progress;
    QSet<AbstractTaskItem *> m_tasks;
    QTimer *m_timer;
    QList<QAction *> m_staticMenu;
    QAction *m_separator;
    QString m_dbusMenuService;
    QString m_dbusMenuPath;
    DBusMenuImporter *m_dbusMenu;
};

#endif
