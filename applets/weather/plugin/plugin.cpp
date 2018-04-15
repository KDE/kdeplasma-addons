/*
 * Copyright 2016  Friedrich W. H. Kossebau <kossebau@kde.org>
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

#include "plugin.h"

#include "abstractunitlistmodel.h"
#include "locationlistmodel.h"
#include "servicelistmodel.h"
#include "util.h"

// KF
#include <KLocalizedString>
// Qt
#include <QQmlEngine>


static QObject* temperatureUnitListModelSingletonTypeProvider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    QVector<UnitItem> items {
        UnitItem(i18n("Celsius \302\260C"), KUnitConversion::Celsius),
        UnitItem(i18n("Fahrenheit \302\260F"), KUnitConversion::Fahrenheit),
        UnitItem(i18n("Kelvin K"), KUnitConversion::Kelvin)
    };

    return new AbstractUnitListModel(items);
}

static QObject* pressureUnitListModelSingletonTypeProvider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    QVector<UnitItem> items {
        UnitItem(i18n("Hectopascals hPa"), KUnitConversion::Hectopascal),
        UnitItem(i18n("Kilopascals kPa"), KUnitConversion::Kilopascal),
        UnitItem(i18n("Millibars mbar"), KUnitConversion::Millibar),
        UnitItem(i18n("Inches of Mercury inHg"), KUnitConversion::InchesOfMercury)
    };

    return new AbstractUnitListModel(items);
}

static QObject* windSpeedUnitListModelSingletonTypeProvider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    QVector<UnitItem> items {
        UnitItem(i18n("Meters per Second m/s"), KUnitConversion::MeterPerSecond),
        UnitItem(i18n("Kilometers per Hour km/h"), KUnitConversion::KilometerPerHour),
        UnitItem(i18n("Miles per Hour mph"), KUnitConversion::MilePerHour),
        UnitItem(i18n("Knots kt"), KUnitConversion::Knot),
        UnitItem(i18n("Beaufort scale bft"), KUnitConversion::Beaufort)
    };

    return new AbstractUnitListModel(items);
}

static QObject* visibilityUnitListModelSingletonTypeProvider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    QVector<UnitItem> items {
        UnitItem(i18n("Kilometers"), KUnitConversion::Kilometer),
        UnitItem(i18n("Miles"), KUnitConversion::Mile)
    };

    return new AbstractUnitListModel(items);
}

static QObject* utilSingletonTypeProvider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    return new Util();
}


void WeatherPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.kde.plasma.private.weather"));

    qmlRegisterSingletonType<AbstractUnitListModel>(uri, 1, 0, "TemperatureUnitListModel",
                                                    temperatureUnitListModelSingletonTypeProvider);
    qmlRegisterSingletonType<AbstractUnitListModel>(uri, 1, 0, "PressureUnitListModel",
                                                    pressureUnitListModelSingletonTypeProvider);
    qmlRegisterSingletonType<AbstractUnitListModel>(uri, 1, 0, "WindSpeedUnitListModel",
                                                    windSpeedUnitListModelSingletonTypeProvider);
    qmlRegisterSingletonType<AbstractUnitListModel>(uri, 1, 0, "VisibilityUnitListModel",
                                                    visibilityUnitListModelSingletonTypeProvider);
    qmlRegisterSingletonType<Util>(uri, 1, 0, "Util",
                                   utilSingletonTypeProvider);
    qmlRegisterType<LocationListModel>(uri, 1, 0, "LocationListModel");
    qmlRegisterType<ServiceListModel>(uri, 1, 0, "ServiceListModel");
}
