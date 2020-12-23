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
    Q_UNUSED(scriptEngine)

    QVector<UnitItem> items {
        UnitItem(KUnitConversion::Celsius),
        UnitItem(KUnitConversion::Fahrenheit),
        UnitItem(KUnitConversion::Kelvin)
    };

    return new AbstractUnitListModel(items, engine);
}

static QObject* pressureUnitListModelSingletonTypeProvider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(scriptEngine)

    QVector<UnitItem> items {
        UnitItem(KUnitConversion::Hectopascal),
        UnitItem(KUnitConversion::Kilopascal),
        UnitItem(KUnitConversion::Millibar),
        UnitItem(KUnitConversion::InchesOfMercury),
        UnitItem(KUnitConversion::MillimetersOfMercury)
    };

    return new AbstractUnitListModel(items, engine);
}

static QObject* windSpeedUnitListModelSingletonTypeProvider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(scriptEngine)

    QVector<UnitItem> items {
        UnitItem(KUnitConversion::MeterPerSecond),
        UnitItem(KUnitConversion::KilometerPerHour),
        UnitItem(KUnitConversion::MilePerHour),
        UnitItem(KUnitConversion::Knot),
        UnitItem(KUnitConversion::Beaufort)
    };

    return new AbstractUnitListModel(items, engine);
}

static QObject* visibilityUnitListModelSingletonTypeProvider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(scriptEngine)

    QVector<UnitItem> items {
        UnitItem(KUnitConversion::Kilometer),
        UnitItem(KUnitConversion::Mile)
    };

    return new AbstractUnitListModel(items, engine);
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
