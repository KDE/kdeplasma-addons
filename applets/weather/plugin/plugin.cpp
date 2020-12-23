/*
 * SPDX-FileCopyrightText: 2016 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
