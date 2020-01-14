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

#include "monitor.h"
#include "monitor_p.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusServiceWatcher>

static const QString s_serviceName = QStringLiteral("org.kde.KWin");
static const QString s_nightColorPath = QStringLiteral("/ColorCorrect");
static const QString s_nightColorInterface = QStringLiteral("org.kde.kwin.ColorCorrect");
static const QString s_propertiesInterface = QStringLiteral("org.freedesktop.DBus.Properties");

MonitorPrivate::MonitorPrivate(QObject *parent)
    : QObject(parent)
{
    QDBusServiceWatcher *watcher = new QDBusServiceWatcher(s_serviceName,
                                                           QDBusConnection::sessionBus(),
                                                           QDBusServiceWatcher::WatchForOwnerChange,
                                                           this);
    connect(watcher, &QDBusServiceWatcher::serviceRegistered, this, &MonitorPrivate::handleServiceRegistered);
    connect(watcher, &QDBusServiceWatcher::serviceUnregistered, this, &MonitorPrivate::handleServiceUnregistered);

    handleServiceRegistered();
}

MonitorPrivate::~MonitorPrivate()
{
}

void MonitorPrivate::handleServiceRegistered()
{
    QDBusConnection bus = QDBusConnection::sessionBus();

    const bool connected = bus.connect(s_serviceName, s_nightColorPath, s_propertiesInterface,
                                       QStringLiteral("PropertiesChanged"),
                                       this, SLOT(handlePropertiesChanged(QString,QVariantMap,QStringList)));
    if (!connected) {
        return;
    }

    QDBusMessage message = QDBusMessage::createMethodCall(s_serviceName,
                                                          s_nightColorPath,
                                                          s_propertiesInterface,
                                                          QStringLiteral("GetAll"));
    message.setArguments({ s_nightColorInterface });

    QDBusPendingReply<QVariantMap> properties = bus.asyncCall(message);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(properties, this);

    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *self) {
        self->deleteLater();

        const QDBusPendingReply<QVariantMap> properties = *self;
        if (properties.isError()) {
            return;
        }

        updateProperties(properties.value());
    });
}

void MonitorPrivate::handleServiceUnregistered()
{
    QDBusConnection bus = QDBusConnection::sessionBus();

    bus.disconnect(s_serviceName, s_nightColorPath, s_propertiesInterface,
                   QStringLiteral("PropertiesChanged"),
                   this, SLOT(handlePropertiesChanged(QString,QVariantMap,QStringList)));

    setAvailable(false);
}

void MonitorPrivate::handlePropertiesChanged(const QString &interfaceName,
                                             const QVariantMap &changedProperties,
                                             const QStringList &invalidatedProperties)
{
    Q_UNUSED(interfaceName)
    Q_UNUSED(invalidatedProperties)

    updateProperties(changedProperties);
}

int MonitorPrivate::currentTemperature() const
{
    return m_currentTemperature;
}

int MonitorPrivate::targetTemperature() const
{
    return m_targetTemperature;
}

void MonitorPrivate::updateProperties(const QVariantMap &properties)
{
    const QVariant available = properties.value(QStringLiteral("available"));
    if (available.isValid()) {
        setAvailable(available.toBool());
    }

    const QVariant enabled = properties.value(QStringLiteral("enabled"));
    if (enabled.isValid()) {
        setEnabled(enabled.toBool());
    }

    const QVariant running = properties.value(QStringLiteral("running"));
    if (running.isValid()) {
        setRunning(running.toBool());
    }

    const QVariant currentTemperature = properties.value(QStringLiteral("currentTemperature"));
    if (currentTemperature.isValid()) {
        setCurrentTemperature(currentTemperature.toInt());
    }

    const QVariant targetTemperature = properties.value(QStringLiteral("targetTemperature"));
    if (targetTemperature.isValid()) {
        setTargetTemperature(targetTemperature.toInt());
    }
}

void MonitorPrivate::setCurrentTemperature(int temperature)
{
    if (m_currentTemperature == temperature) {
        return;
    }
    m_currentTemperature = temperature;
    emit currentTemperatureChanged();
}

void MonitorPrivate::setTargetTemperature(int temperature)
{
    if (m_targetTemperature == temperature) {
        return;
    }
    m_targetTemperature = temperature;
    emit targetTemperatureChanged();
}

bool MonitorPrivate::isAvailable() const
{
    return m_isAvailable;
}

void MonitorPrivate::setAvailable(bool available)
{
    if (m_isAvailable == available) {
        return;
    }
    m_isAvailable = available;
    emit availableChanged();
}

bool MonitorPrivate::isEnabled() const
{
    return m_isEnabled;
}

void MonitorPrivate::setEnabled(bool enabled)
{
    if (m_isEnabled == enabled) {
        return;
    }
    m_isEnabled = enabled;
    emit enabledChanged();
}

bool MonitorPrivate::isRunning() const
{
    return m_isRunning;
}

void MonitorPrivate::setRunning(bool running)
{
    if (m_isRunning == running) {
        return;
    }
    m_isRunning = running;
    emit runningChanged();
}

Monitor::Monitor(QObject *parent)
    : QObject(parent)
    , d(new MonitorPrivate(this))
{
    connect(d, &MonitorPrivate::availableChanged, this, &Monitor::availableChanged);
    connect(d, &MonitorPrivate::enabledChanged, this, &Monitor::enabledChanged);
    connect(d, &MonitorPrivate::runningChanged, this, &Monitor::runningChanged);
    connect(d, &MonitorPrivate::currentTemperatureChanged, this, &Monitor::currentTemperatureChanged);
    connect(d, &MonitorPrivate::targetTemperatureChanged, this, &Monitor::targetTemperatureChanged);
}

Monitor::~Monitor()
{
}

bool Monitor::isAvailable() const
{
    return d->isAvailable();
}

bool Monitor::isEnabled() const
{
    return d->isEnabled();
}

bool Monitor::isRunning() const
{
    return d->isRunning();
}

int Monitor::currentTemperature() const
{
    return d->currentTemperature();
}

int Monitor::targetTemperature() const
{
    return d->targetTemperature();
}
