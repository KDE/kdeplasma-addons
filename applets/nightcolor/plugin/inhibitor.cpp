/*
 * Copyright 2019 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "inhibitor.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(NIGHTCOLOR_CONTROL, "org.kde.plasma.nightcolorcontrol")

static const QString s_serviceName = QStringLiteral("org.kde.KWin");
static const QString s_path = QStringLiteral("/ColorCorrect");
static const QString s_interface = QStringLiteral("org.kde.kwin.ColorCorrect");

class Inhibitor::Private
{
public:
    uint cookie = 0;
    State state = Uninhibited;
    bool pendingUninhibit = false;
};

Inhibitor::Inhibitor(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

Inhibitor::~Inhibitor()
{
    uninhibit();
}

Inhibitor::State Inhibitor::state() const
{
    return d->state;
}

void Inhibitor::inhibit()
{
    if (d->state == Inhibited) {
        return;
    }

    d->pendingUninhibit = false;

    if (d->state == Inhibiting) {
        return;
    }

    QDBusMessage message = QDBusMessage::createMethodCall(s_serviceName,
                                                          s_path,
                                                          s_interface,
                                                          QStringLiteral("inhibit"));

    QDBusPendingReply<uint> cookie = QDBusConnection::sessionBus().asyncCall(message);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(cookie, this);

    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *self) {
        const bool wasPendingUninhibit = d->pendingUninhibit;
        d->pendingUninhibit = false;

        const QDBusPendingReply<uint> reply = *self;
        self->deleteLater();

        if (reply.isError()) {
            qCWarning(NIGHTCOLOR_CONTROL()) << "Could not inhibit Night Color:" << reply.error().message();
            d->state = Uninhibited;
            emit stateChanged();
            return;
        }

        d->cookie = reply.value();
        d->state = Inhibited;
        emit stateChanged();

        if (wasPendingUninhibit) {
            uninhibit();
        }
    });

    d->state = Inhibiting;
    emit stateChanged();
}

void Inhibitor::uninhibit()
{
    if (d->state == Uninhibiting || d->state == Uninhibited) {
        return;
    }

    if (d->state == Inhibiting) {
        d->pendingUninhibit = true;
        return;
    }

    QDBusMessage message = QDBusMessage::createMethodCall(s_serviceName,
                                                          s_path,
                                                          s_interface,
                                                          QStringLiteral("uninhibit"));
    message.setArguments({ d->cookie });

    QDBusPendingReply<void> reply = QDBusConnection::sessionBus().asyncCall(message);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);

    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *self) {
        self->deleteLater();

        if (d->state != Uninhibiting) {
            return;
        }

        const QDBusPendingReply<void> reply = *self;
        if (reply.isError()) {
            qCWarning(NIGHTCOLOR_CONTROL) << "Could not uninhibit Night Color:" << reply.error().message();
        }

        d->state = Uninhibited;
        emit stateChanged();
    });

    d->state = Uninhibiting;
    emit stateChanged();
}
