/*
 * Copyright 2018  Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef UTIL_H
#define UTIL_H

// KF
#include <KUnitConversion/Converter>
// Qt
#include <QObject>


class Util : public QObject
{
    Q_OBJECT

public:
    explicit Util(QObject *parent = nullptr);

public:
    /**
     * Returns the @p iconName if the current icon theme contains an icon with that name,
     * otherwise returns "weather-not-available" (expecting the icon theme to have that in any case).
     */
    Q_INVOKABLE QString existingWeatherIconName(const QString &iconName) const;

    Q_INVOKABLE QString temperatureToDisplayString(int displayUnitType, double value,
                                                   int valueUnitType, bool rounded = false, bool degreesOnly = false) const;
    Q_INVOKABLE QString valueToDisplayString(int displayUnitType, double value,
                                             int valueUnitType, int precision = 0) const;
    Q_INVOKABLE QString percentToDisplayString(double value) const;

    static QString nameFromUnitId(KUnitConversion::UnitId unitId);

private:
    static KUnitConversion::Converter m_converter;
};

#endif
