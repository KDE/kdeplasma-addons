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

#ifndef __DOCKITEM_H__
#define __DOCKITEM_H__

#include <QtCore/QObject>
#include <QtCore/QSet>
#include <QtCore/QMap>
#include <QtGui/QIcon>
#include <QtDBus/QtDBus>
#include <KDE/KUrl>

class AbstractTaskItem;
class QAction;
class QTimer;
class QMenu;

class DockItem : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "net.launchpad.DockItem")
    Q_PROPERTY(QString DesktopFile READ DesktopFile)
    Q_PROPERTY(QString Uri READ Uri)

public:

    DockItem(const KUrl &desktopFile);
    ~DockItem();

    QString DesktopFile() const;
    QString Uri() const;
    const QString & path() const {
        return m_path;
    }
    QString name() const;
    const KUrl & url() const {
        return m_url;
    }
    const QIcon & icon() const {
        return m_icon;
    }
    const QIcon & overlayIcon() const {
        return m_overlayIcon;
    }
    const QString & badge() const {
        return m_badge;
    }
    int progress() const {
        return m_progress;
    }
    QList<QAction *> menu() const;

    void registerTask(AbstractTaskItem *item);
    void unregisterTask(AbstractTaskItem *item);
    void reset();

public Q_SLOTS:
    Q_SCRIPTABLE unsigned int AddMenuItem(QMap<QString, QVariant> hints);
    Q_NOREPLY void RemoveMenuItem(unsigned int id);
    Q_NOREPLY void UpdateDockItem(QMap<QString, QVariant> hints);

private Q_SLOTS:
    void menuActivated();
    void check();

Q_SIGNALS:
    void MenuItemActivated(unsigned int id);

private:
    KUrl m_url;
    QString m_path;
    mutable QString m_name;
    QSet<AbstractTaskItem *> m_tasks;
    QMap<unsigned int, QAction *> m_menu;
    QTimer *m_timer;
    QString m_remoteService;
    QMap<QString, QMenu *> m_actionMenus;

    QString m_badge;
    QIcon m_icon;
    QIcon m_overlayIcon;
    int m_progress;
    unsigned int m_menuIdCount;
};

#endif
