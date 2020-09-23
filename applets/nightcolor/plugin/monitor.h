/*
 * SPDX-FileCopyrightText: 2019 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QObject>

class MonitorPrivate;

/**
 * The Monitor provides a way for monitoring the state of Night Color.
 */
class Monitor : public QObject
{
    Q_OBJECT

    /**
     * This property holds a value to indicate if Night Color is available.
     */
    Q_PROPERTY(bool available READ isAvailable NOTIFY availableChanged)

    /**
     * This property holds a value to indicate if Night Color is enabled.
     */
    Q_PROPERTY(bool enabled READ isEnabled NOTIFY enabledChanged)

    /**
     * This property holds a value to indicate if Night Color is running.
     */
    Q_PROPERTY(bool running READ isRunning NOTIFY runningChanged)

    /**
     * This property holds a value to indicate currently applied color temperature.
     */
    Q_PROPERTY(int currentTemperature READ currentTemperature NOTIFY currentTemperatureChanged)

    /**
     * This property holds a value to indicate currently applied color temperature.
     */
    Q_PROPERTY(int targetTemperature READ targetTemperature NOTIFY targetTemperatureChanged)

public:
    explicit Monitor(QObject *parent = nullptr);
    ~Monitor() override;

    /**
     * Returns @c true if Night Color is available; otherwise @c false.
     */
    bool isAvailable() const;

    /**
     * Returns @c true if Night Color is enabled; otherwise @c false.
     */
    bool isEnabled() const;

    /**
     * Returns @c true if Night Color is running; otherwise @c false.
     */
    bool isRunning() const;

    /**
     * Returns currently applied screen color temperature.
     */
    int currentTemperature() const;

    /**
     * Returns currently applied screen color temperature.
     */
    int targetTemperature() const;

Q_SIGNALS:
    /**
     * This signal is emitted when Night Color becomes (un)available.
     */
    void availableChanged();

    /**
     * Emitted whenever Night Color is enabled or disabled.
     */
    void enabledChanged();

    /**
     * Emitted whenever Night Color starts or stops running.
     */
    void runningChanged();

    /**
     * Emitted whenever the current screen color temperature has changed.
     */
    void currentTemperatureChanged();

    /**
     * Emitted whenever the current screen color temperature has changed.
     */
    void targetTemperatureChanged();

private:
    MonitorPrivate *d;
};
