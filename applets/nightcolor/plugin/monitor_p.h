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

#pragma once

#include <QObject>

class MonitorPrivate : public QObject
{
    Q_OBJECT

public:
    explicit MonitorPrivate(QObject *parent = nullptr);
    ~MonitorPrivate() override;

    int currentTemperature() const;
    int targetTemperature() const;
    bool isAvailable() const;
    bool isEnabled() const;
    bool isRunning() const;

Q_SIGNALS:
    void currentTemperatureChanged();
    void targetTemperatureChanged();
    void availableChanged();
    void enabledChanged();
    void runningChanged();

private Q_SLOTS:
    void handleServiceRegistered();
    void handleServiceUnregistered();
    void handlePropertiesChanged(const QString &interfaceName,
                                 const QVariantMap &changedProperties,
                                 const QStringList &invalidatedProperties);

private:
    void updateProperties(const QVariantMap &properties);
    void setCurrentTemperature(int temperature);
    void setTargetTemperature(int temperature);
    void setAvailable(bool available);
    void setEnabled(bool enabled);
    void setRunning(bool running);

    int m_currentTemperature = 0;
    int m_targetTemperature = 0;
    bool m_isAvailable = false;
    bool m_isEnabled = false;
    bool m_isRunning = false;
};
