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

#include "mediabuttons.h"
#include "dbusstatus.h"
#include "playerv1interface.h"
#include "playerv2interface.h"
#include <KDE/KConfig>
#include <KDE/KConfigGroup>
#include <KDE/KGlobal>
#include <KDE/KStandardDirs>
#include <KDE/KSycoca>
#include <KDE/KService>
#include <KDE/KServiceTypeTrader>
#include <QtDBus/QDBusServiceWatcher>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>

K_GLOBAL_STATIC(MediaButtons, mediaBtns)

static const QString constV2Prefix = QLatin1String("org.mpris.MediaPlayer2.");
static const QString constV1Prefix = QLatin1String("org.mpris.");

static QString playbackStatus(OrgFreedesktopMediaPlayerInterface *iface)
{
    DBusStatus status = iface->GetStatus();

    switch (status.play) {
    case DBusStatus::Mpris_Playing: return "Playing";
    case DBusStatus::Mpris_Paused:  return "Paused";
    case DBusStatus::Mpris_Stopped: return "Stopped";
    }
    return QString();
}

MediaButtons::Interface::~Interface()
{
    if (v1) {
        delete v1;
    }
    if (v2) {
        delete v2;
    }
}

void MediaButtons::Interface::next()
{
    if (v2) {
        v2->Next();
    } else if (v1) {
        v1->Next();
    }
}

void MediaButtons::Interface::previous()
{
    if (v2) {
        v2->Previous();
    } else if (v1) {
        v1->Prev();
    }
}

void MediaButtons::Interface::playPause()
{
    if (v2) {
        v2->PlayPause();
    } else if (v1) {
        if ("Playing" ==::playbackStatus(v1)) {
            v1->Pause();
        } else {
            v1->Play();
        }
    }
}

QString MediaButtons::Interface::playbackStatus()
{
    if (v2) {
        return v2->playbackStatus();
    } else if (v1) {
        return ::playbackStatus(v1);
    }

    return QString();
}

QString MediaButtons::Interface::service()
{
    if (v2) {
        return v2->service();
    } else if (v1) {
        return v1->service();
    }

    return QString();
}

MediaButtons * MediaButtons::self()
{
    return mediaBtns;
}

MediaButtons::MediaButtons()
    : m_watcher(0)
    , m_enabled(false)
{
    qDBusRegisterMetaType<DBusStatus>();
}

void MediaButtons::setEnabled(bool en)
{
    if (en != m_enabled) {
        m_enabled = en;
        if (m_enabled) {
            m_watcher = new QDBusServiceWatcher(this);
            m_watcher->setConnection(QDBusConnection::sessionBus());
            m_watcher->setWatchMode(QDBusServiceWatcher::WatchForOwnerChange);
            connect(m_watcher, SIGNAL(serviceOwnerChanged(QString, QString, QString)), this, SLOT(serviceOwnerChanged(QString, QString, QString)));
            connect(KSycoca::self(), SIGNAL(databaseChanged(QStringList)), SLOT(sycocaChanged(QStringList)));
            readConfig();
            updateApps();
        } else if (m_watcher) {
            disconnect(m_watcher, SIGNAL(serviceOwnerChanged(QString, QString, QString)), this, SLOT(serviceOwnerChanged(QString, QString, QString)));
            disconnect(KSycoca::self(), SIGNAL(databaseChanged(QStringList)), this, SLOT(sycocaChanged(QStringList)));

            foreach (Interface * iface, m_interfaces.values()) {
                delete iface;
            }
            m_interfaces.clear();

            delete m_watcher;
        }
    }
}

void MediaButtons::sycocaChanged(const QStringList &types)
{
    if (types.contains("apps")) {
        updateApps();
    }
}

void MediaButtons::serviceOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner)
{
    bool isV2 = name.startsWith(constV2Prefix);
    QString n = QString(name).remove(isV2 ? constV2Prefix : constV1Prefix).toLower();
    QMap<QString, Interface *>::iterator it(m_interfaces.find(n)),
         end(m_interfaces.end());

    if (newOwner.isEmpty()) {
        if (it != end) {
            if ((*it)->isV2() == isV2) {
                delete(*it);
                m_interfaces.erase(it);
            }
        }
    } else if (oldOwner.isEmpty()) {
        if (isV2) {
            OrgMprisMediaPlayer2PlayerInterface *iface = new OrgMprisMediaPlayer2PlayerInterface(name, "/org/mpris/MediaPlayer2", QDBusConnection::sessionBus(), this);
            if (iface->canControl()) {
                if (it != end) {
                    delete(*it);
                    m_interfaces.erase(it);
                }
                m_interfaces.insert(n, new Interface(iface));
            } else {
                delete iface;
            }
        } else if (it == end || !(*it)->isV2()) {
            OrgFreedesktopMediaPlayerInterface *iface = new OrgFreedesktopMediaPlayerInterface(name, "/Player", QDBusConnection::sessionBus(), this);
            if (it != end) {
                delete(*it);
                m_interfaces.erase(it);
            }
            m_interfaces.insert(n, new Interface(iface));
        }
    }
}

void MediaButtons::next(const QString &name, int pid)
{
    if (m_enabled) {
        Interface *iface = getInterface(name, pid);
        if (iface) {
            iface->next();
        }
    }
}

void MediaButtons::previous(const QString &name, int pid)
{
    if (m_enabled) {
        Interface *iface = getInterface(name, pid);
        if (iface) {
            iface->previous();
        }
    }
}

void MediaButtons::playPause(const QString &name, int pid)
{
    if (m_enabled) {
        Interface *iface = getInterface(name, pid);
        if (iface) {
            iface->playPause();
        }
    }
}

QString MediaButtons::playbackStatus(const QString &name, int pid)
{
    if (m_enabled) {
        Interface *iface = getInterface(name, pid);
        if (iface) {
            return iface->playbackStatus();
        }
    }

    return QString();
}

void MediaButtons::readConfig()
{
    m_aliases.clear();
    m_ignore.clear();

    QStringList files(KGlobal::dirs()->findAllResources("data", "kdeplasma-addons/mediabuttonsrc"));

    foreach (QString file, files) {
        KConfig cfg(file);
        KConfigGroup ag(&cfg, "Aliases");
        KConfigGroup gen(&cfg, "General");

        m_ignore += gen.readEntry("Ignore", QStringList()).toSet();
        m_customMediaApps = gen.readEntry("CustomMediaApps", QStringList()).toSet();
        foreach (const QString & key, ag.keyList()) {
            foreach (const QString & alias, ag.readEntry(key, QStringList())) {
                m_aliases[alias.toLower()] = key.toLower();
            }
        }
    }
}

void MediaButtons::updateApps()
{
    if (!m_enabled) {
        return;
    }

    KService::List services = KServiceTypeTrader::self()->query("Application", QString("exist Exec and (exist Categories and ( ('AudioVideo' ~subin Categories) or ('Music' ~subin Categories) ) )"));
    QStringList prefixes = QStringList() << constV2Prefix << constV1Prefix;

    m_mediaApps.clear();
    m_mediaApps = m_aliases.keys().toSet();
    foreach (const KSharedPtr<KService> srv, services) {
        QString name = srv->desktopEntryName();

        if (name.startsWith("kde4-")) {
            name = name.mid(5);
        }

        if (m_aliases.contains(name)) {
            name = m_aliases[name];
        }

        if (m_ignore.contains(name)) {
            continue;
        }

        m_mediaApps.insert(name.toLower());
    }
    m_mediaApps += m_customMediaApps;
}

MediaButtons::Interface * MediaButtons::getInterface(const QString &name, int pid)
{
    QStringList names;

    if (m_aliases.contains(name)) {
        QString alias = m_aliases[name];
        names << alias << alias + "." + QString().setNum(pid) << alias + "-" + QString().setNum(pid);
    }
    names << name << name + "." + QString().setNum(pid) << name + "-" + QString().setNum(pid);

    foreach (QString n, names) {
        if (m_interfaces.contains(n)) {
            return m_interfaces[n];
        }
    }

    foreach (QString n, names) {
        MediaButtons::Interface *i = getV2Interface(n);
        if (i) {
            return i;
        }
    }

    foreach (QString n, names) {
        MediaButtons::Interface *i = getV1Interface(n);
        if (i) {
            return i;
        }
    }

    return 0;
}

MediaButtons::Interface * MediaButtons::getV2Interface(const QString &name)
{
    QDBusReply<bool> reply = QDBusConnection::sessionBus().interface()->isServiceRegistered(constV2Prefix + name);

    if (reply.isValid() && reply.value()) {
        serviceOwnerChanged(constV2Prefix + name, QString(), QLatin1String("X"));
        if (m_interfaces.contains(name)) {
            m_watcher->addWatchedService(constV2Prefix + name);
            return m_interfaces[name];
        }
    }

    return 0;
}

MediaButtons::Interface * MediaButtons::getV1Interface(const QString &name)
{
    QDBusReply<bool> reply = QDBusConnection::sessionBus().interface()->isServiceRegistered(constV1Prefix + name);

    if (reply.isValid() && reply.value()) {
        serviceOwnerChanged(constV1Prefix + name, QString(), QLatin1String("X"));
        if (m_interfaces.contains(name)) {
            m_watcher->addWatchedService(constV1Prefix + name);
            return m_interfaces[name];
        }
    }

    return 0;
}

#include "mediabuttons.moc"
