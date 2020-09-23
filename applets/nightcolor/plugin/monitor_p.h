/*
 * SPDX-FileCopyrightText: 2019 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
