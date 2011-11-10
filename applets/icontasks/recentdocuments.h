#ifndef __RECENT_DOCUMENTS__
#define __RECENT_DOCUMENTS__

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
#include <QtCore/QList>
#include <QtGui/QAction>
#include "taskmanager/taskactions.h"

class KDirWatch;

class RecentDocuments : public QObject
{
    Q_OBJECT

    struct File {
        enum Type {
            Xbel,
            Office
        };

        File(Type t, const QString &p) : type(t), path(p), dirty(true) { }
        Type type;
        QString path;
        bool dirty;
    };

    struct App {
        App(const QString &n = QString(), const QString &e = QString()) : name(n), exec(e) { }
        QString name;
        QString exec;
    };

public:
    static RecentDocuments * self();

    RecentDocuments();
    ~RecentDocuments();

    void setEnabled(bool enabled);
    bool isEnabled() const {
        return m_enabled;
    }

    QList<QAction *> get(const QString &app);

private Q_SLOTS:
    void added(const QString& path);
    void removed(const QString& path);
    void modified(const QString& path);
    void sycocaChanged(const QStringList &types);
    void loadDoc();

private:
    void readCurrentDocs();
    void load();
    App officeAppForMimeType(const QString &mimeType);
    App appForExec(const QString &execString);
    void loadXbel(const QString &path, qulonglong now);
    void loadOffice(const QString &path, qulonglong now);
    void removeOld(qulonglong now, File::Type type);

private:

    bool m_enabled;
    QMap<QString, QList<QAction *> > m_docs;
    QMap<QString, App> m_apps;
    KDirWatch *m_watcher;
    QList<File> m_files;
    TaskManager::ToolTipMenu *m_menu;
};

#endif
