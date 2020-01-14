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
