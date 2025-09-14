// SPDX-FileCopyrightText: 2025 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QLocale>
#include <QObject>
#include <QQmlEngine>

#include "abstractunitlistmodel.h"

static bool isMetric()
{
    return QLocale().measurementSystem() == QLocale::MetricSystem;
}

struct TemperatureUnitListModel {
    Q_GADGET
    QML_FOREIGN(AbstractUnitListModel)
    QML_ELEMENT
    QML_SINGLETON
public:
    static AbstractUnitListModel *create(QQmlEngine *engine, QJSEngine *)
    {
        QList<UnitItem> items{
            UnitItem(KUnitConversion::Celsius),
            UnitItem(KUnitConversion::Fahrenheit),
            UnitItem(KUnitConversion::Kelvin),
        };

        const int defaultUnitId = isMetric() ? KUnitConversion::Celsius : KUnitConversion::Fahrenheit;
        return new AbstractUnitListModel(items, defaultUnitId, engine);
    }
};

struct VisibilityUnitListModel {
    Q_GADGET
    QML_FOREIGN(AbstractUnitListModel)
    QML_ELEMENT
    QML_SINGLETON
public:
    static AbstractUnitListModel *create(QQmlEngine *engine, QJSEngine *)
    {
        QList<UnitItem> items{
            UnitItem(KUnitConversion::Kilometer),
            UnitItem(KUnitConversion::Mile),
        };

        const int defaultUnitId = isMetric() ? KUnitConversion::Kilometer : KUnitConversion::Mile;
        return new AbstractUnitListModel(items, defaultUnitId, engine);
    }
};

struct PressureUnitListModel {
    Q_GADGET
    QML_FOREIGN(AbstractUnitListModel)
    QML_NAMED_ELEMENT(PressureUnitListModel)
    QML_SINGLETON
public:
    static AbstractUnitListModel *create(QQmlEngine *engine, QJSEngine *)
    {
        const QList<UnitItem> items{
            UnitItem(KUnitConversion::Hectopascal),
            UnitItem(KUnitConversion::Kilopascal),
            UnitItem(KUnitConversion::Pascal),
            UnitItem(KUnitConversion::Millibar),
            UnitItem(KUnitConversion::InchesOfMercury),
            UnitItem(KUnitConversion::MillimetersOfMercury),
        };

        const int defaultUnitId = isMetric() ? KUnitConversion::Hectopascal : KUnitConversion::InchesOfMercury;
        return new AbstractUnitListModel(items, defaultUnitId, engine);
    }
};

struct WindSpeedUnitListModel {
    Q_GADGET
    QML_FOREIGN(AbstractUnitListModel)
    QML_ELEMENT
    QML_SINGLETON
public:
    static AbstractUnitListModel *create(QQmlEngine *engine, QJSEngine *)
    {
        const QList<UnitItem> items{
            UnitItem(KUnitConversion::MeterPerSecond),
            UnitItem(KUnitConversion::KilometerPerHour),
            UnitItem(KUnitConversion::MilePerHour),
            UnitItem(KUnitConversion::Knot),
            UnitItem(KUnitConversion::Beaufort),
        };

        const int defaultUnitId = isMetric() ? KUnitConversion::MeterPerSecond : KUnitConversion::MilePerHour;
        return new AbstractUnitListModel(items, defaultUnitId, engine);
    }
};
