/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

// KF
#include <KUnitConversion/Converter>
// Qt
#include <QObject>

#include <qqmlintegration.h>

class Util : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QString unknownWeatherIcon READ unknownWeatherIconName CONSTANT)

public:
    explicit Util(QObject *parent = nullptr);

public:
    Q_INVOKABLE QString temperatureToDisplayString(int displayUnitType, double value, int valueUnitType, bool rounded = false, bool degreesOnly = false) const;
    Q_INVOKABLE QString valueToDisplayString(int displayUnitType, double value, int valueUnitType, int precision = 0) const;
    Q_INVOKABLE QString percentToDisplayString(double value) const;

    static QString nameFromUnitId(KUnitConversion::UnitId unitId);

private:
    static KUnitConversion::Converter m_converter;
    static QString unknownWeatherIconName();
};
