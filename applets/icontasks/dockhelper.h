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

#ifndef __DOCKHELPER_H__
#define __DOCKHELPER_H__

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QProcess>

class DockHelper : public QObject
{
    Q_OBJECT

public:
    DockHelper(const QString &dir, const QString &fn);
    virtual ~DockHelper();

    operator bool() const {
        return m_valid;
    }

    const QString & fileName() const {
        return m_fileName;
    }
    const QString & dirName() const {
        return m_dir;
    }
    Q_PID pid() const {
        return m_proc ? m_proc->pid() : 0;
    }

public Q_SLOTS:
    void serviceOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner);
    void start();
    void stop();

private:
    QString m_fileName;
    QString m_dir;
    QString m_app;
    QString m_dBusName;
    bool m_valid;
    QProcess *m_proc;
};

#endif
