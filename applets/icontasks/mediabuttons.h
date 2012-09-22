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

#ifndef __MEDIABUTTONS_H__
#define __MEDIABUTTONS_H__

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QSet>

class QDBusServiceWatcher;
class OrgMprisMediaPlayer2PlayerInterface;
class OrgFreedesktopMediaPlayerInterface;

class MediaButtons : public QObject
{
    Q_OBJECT

public:
    class Interface
    {
    public:
        Interface(OrgFreedesktopMediaPlayerInterface *o) : v1(o), v2(0) { }
        Interface(OrgMprisMediaPlayer2PlayerInterface *t) : v1(0), v2(t) { }
        ~Interface();

        bool isV1() const {
            return 0 != v1;
        }
        bool isV2() const {
            return 0 != v2;
        }

        void next();
        void previous();
        void playPause();
        QString playbackStatus();
        QString service();
    private:
        OrgFreedesktopMediaPlayerInterface *v1;
        OrgMprisMediaPlayer2PlayerInterface *v2;
    };

    static MediaButtons * self();

    MediaButtons();

    void setEnabled(bool en);
    bool isEnabled() const {
        return m_enabled;
    }
    bool isMediaApp(const QString &desktopEntry) const {
        return m_mediaApps.contains(desktopEntry);
    }
    void next(const QString &name, int pid = 0);
    void previous(const QString &name, int pid = 0);
    void playPause(const QString &name, int pid = 0);
    QString playbackStatus(const QString &name, int pid = 0);

private Q_SLOTS:
    void sycocaChanged(const QStringList &types);
    void serviceOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner);

private:
    void readConfig();
    void updateApps();
    Interface * getInterface(const QString &name, int pid);
    Interface * getV2Interface(const QString &name);
    Interface * getV1Interface(const QString &name);

private:
    QDBusServiceWatcher *m_watcher;
    QMap<QString, Interface *> m_interfaces;
    QMap<QString, QString> m_aliases;
    QSet<QString> m_ignore;
    QSet<QString> m_mediaApps;
    QSet<QString> m_customMediaApps;
    bool m_enabled;
};

#endif
