// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Harald Sitter <sitter@kde.org>

#include "qmk.h"
#include "kameleon_debug.h"

#include <QColor>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>

using namespace Qt::StringLiterals;

QMK::QMK(QObject *parent)
    : Backend(parent)
{
    auto message =
        QDBusMessage::createMethodCall(u"org.kde.kameleon.qmk.helper"_s, u"/org/kde/kameleon/qmk/helper"_s, u"org.kde.kameleon.qmk.helper"_s, u"HasDevices"_s);
    auto watcher = new QDBusPendingCallWatcher(QDBusConnection::systemBus().asyncCall(message), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *watcher) {
        watcher->deleteLater();
        QDBusPendingReply<bool> reply = *watcher;
        if (reply.isError()) {
            qCWarning(KAMELEON) << "Failed to call helper:" << reply.error().message();
            return;
        }
        m_supported = reply.value();
        if (m_supported) {
            qCDebug(KAMELEON) << "QMK support detected";
        } else {
            qCDebug(KAMELEON) << "No QMK support detected";
        }
        Q_EMIT supportedChanged();
    });
}

bool QMK::isSupported()
{
    return m_supported;
}

void QMK::applyColor(const QColor &color)
{
    auto message =
        QDBusMessage::createMethodCall(u"org.kde.kameleon.qmk.helper"_s, u"/org/kde/kameleon/qmk/helper"_s, u"org.kde.kameleon.qmk.helper"_s, u"ApplyColor"_s);
    message << color.name(QColor::HexRgb);
    auto watcher = new QDBusPendingCallWatcher(QDBusConnection::systemBus().asyncCall(message), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [](QDBusPendingCallWatcher *watcher) {
        watcher->deleteLater();
        QDBusPendingReply<void> reply = *watcher;
        if (reply.isError()) {
            qCWarning(KAMELEON) << "Failed to call helper:" << reply.error().message();
            return;
        }
    });
}
