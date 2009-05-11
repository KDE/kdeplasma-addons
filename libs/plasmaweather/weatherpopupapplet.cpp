/*
 *   Copyright (C) 2009 Petri Damstén <damu@iki.fi>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "weatherpopupapplet.h"
#include "weatheri18ncatalog.h"

#include <QTimer>

#include <KConfigGroup>
#include <KConfigDialog>

#include <plasma/weather/weatherutils.h>
#include <conversion/converter.h>

#include "weatherconfig.h"
#include "weatherlocation.h"

using namespace Conversion;

class WeatherPopupApplet::Private
{
public:
    Private(WeatherPopupApplet *weatherapplet)
        : q(weatherapplet)
        , weatherConfig(0)
        , weatherEngine(0)
        , timeEngine(0)
        , updateInterval(0)
    {
        QObject::connect(&location, SIGNAL(finished(const QString&)),
                         q, SLOT(locationReady(const QString&)));
        unitMap["C"] = WeatherUtils::Celsius;
        unitMap["F"] = WeatherUtils::Fahrenheit;
        unitMap["K"] = WeatherUtils::Kelvin;
        unitMap["hPa"] = WeatherUtils::Hectopascals;
        unitMap["kPa"] = WeatherUtils::Kilopascals;
        unitMap["mbar"] = WeatherUtils::Millibars;
        unitMap["inHg"] = WeatherUtils::InchesHG;
        unitMap["ms"] = WeatherUtils::MetersPerSecond;
        unitMap["km/h"] = WeatherUtils::KilometersPerHour;
        unitMap["mph"] = WeatherUtils::MilesPerHour;
        unitMap["kt"] = WeatherUtils::Knots;
        unitMap["bft"] = WeatherUtils::Beaufort;
        unitMap["km"] = WeatherUtils::Kilometers;
        unitMap["ml"] = WeatherUtils::Miles;

        busyTimer = new QTimer(q);
        busyTimer->setSingleShot(true);
        busyTimer->setInterval(2*60*1000);
        QObject::connect(busyTimer, SIGNAL(timeout()), q, SLOT(giveUpBeingBusy()));
    }

    WeatherPopupApplet *q;
    WeatherConfig *weatherConfig;
    Plasma::DataEngine *weatherEngine;
    Plasma::DataEngine *timeEngine;
    QString temperatureUnit;
    QString speedUnit;
    QString pressureUnit;
    QString visibilityUnit;
    int updateInterval;
    QString source;
    WeatherLocation location;

    QString conditionIcon;
    QString tend;
    Value pressure;
    Value temperature;
    double latitude;
    double longitude;
    QHash<QString, int> unitMap;
    QTimer *busyTimer;

    void locationReady(const QString &src)
    {
        if (!src.isEmpty()) {
            source = src;
            KConfigGroup cfg = q->config();
            cfg.writeEntry("source", source);
            emit q->configNeedsSaving();
            q->connectToEngine();
        } else {
            busyTimer->stop();
            q->showMessage(QIcon(), QString(), Plasma::ButtonNone);
            q->setBusy(false);
            q->setConfigurationRequired(true);
        }
    }

    void giveUpBeingBusy()
    {
        q->setBusy(false);

        QStringList list = source.split('|', QString::SkipEmptyParts);
        if (list.count() < 3) {
            q->setConfigurationRequired(true);
        } else {
            q->showMessage(KIcon("dialog-error"),
                           i18n("Weather information retrieval for %1 timed out.", list.value(2)),
                           Plasma::ButtonNone);
        }
    }

    qreal tendency(const Conversion::Value& pressure, const QString& tendency)
    {
        qreal t;

        if (tendency.toLower() == "rising") {
            t = 0.75;
        } else if (tendency.toLower() == "falling") {
            t = -0.75;
        } else {
            t = Conversion::Converter::self()->convert(
                    Conversion::Value(tendency.toDouble(), pressure.unit()), "kPa").number();
        }
        return t;
    }

    QString conditionFromPressure()
    {
        QString result;
        if (!pressure.isValid()) {
            return "weather-none-available";
        }
        qreal temp = Converter::self()->convert(temperature, "C").number();
        qreal p = Converter::self()->convert(pressure, "kPa").number();
        qreal t = tendency(pressure, tend);

        // This is completely unscientific so if anyone have a better formula for this :-)
        p += t * 10;

        Plasma::DataEngine::Data data = timeEngine->query(
                QString("Local|Solar|Latitude=%1|Longitude=%2").arg(latitude).arg(longitude));
        bool day = (data["Corrected Elevation"].toDouble() > 0.0);

        if (p > 103.0) {
            if (day) {
                result = "weather-clear";
            } else {
                result = "weather-clear-night";
            }
        } else if (p > 100.0) {
            if (day) {
                result = "weather-clouds";
            } else {
                result = "weather-clouds-night";
            }
        } else if (p > 99.0) {
            if (day) {
                if (temp > 1.0) {
                    result = "weather-showers-scattered-day";
                } else if (temp < -1.0)  {
                    result = "weather-snow-scattered-day";
                } else {
                    result = "weather-snow-rain";
                }
            } else {
                if (temp > 1.0) {
                    result = "weather-showers-scattered-night";
                } else if (temp < -1.0)  {
                    result = "weather-snow-scattered-night";
                } else {
                    result = "weather-snow-rain";
                }
            }
        } else {
            if (temp > 1.0) {
                result = "weather-showers";
            } else if (temp < -1.0)  {
                result = "weather-snow";
            } else {
                result = "weather-snow-rain";
            }
        }
        //kDebug() << result;
        return result;
    }
};

WeatherPopupApplet::WeatherPopupApplet(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args)
    , d(new Private(this))
{
    Weatheri18nCatalog::loadCatalog();
    setHasConfigurationInterface(true);
}

WeatherPopupApplet::~WeatherPopupApplet()
{
    delete d;
}

void WeatherPopupApplet::init()
{
    KConfigGroup cfg = config();

    if (KGlobal::locale()->measureSystem() == KLocale::Metric) {
        d->temperatureUnit = cfg.readEntry("temperatureUnit", "C");
        d->speedUnit = cfg.readEntry("speedUnit", "ms");
        d->pressureUnit = cfg.readEntry("pressureUnit", "hPa");
        d->visibilityUnit = cfg.readEntry("visibilityUnit", "km");
    } else {
        d->temperatureUnit = cfg.readEntry("temperatureUnit", "F");
        d->speedUnit = cfg.readEntry("speedUnit", "mph");
        d->pressureUnit = cfg.readEntry("pressureUnit", "mb");
        d->visibilityUnit = cfg.readEntry("visibilityUnit", "ml");
    }
    d->updateInterval = cfg.readEntry("updateWeather", 30);
    d->source = cfg.readEntry("source", "");

    d->weatherEngine = dataEngine("weather");
    d->timeEngine = dataEngine("time");
    connectToEngine();
}

void WeatherPopupApplet::connectToEngine()
{
    setBusy(true);
    d->busyTimer->start();
    if (d->source.isEmpty()) {
        d->location.setDataEngines(dataEngine("geolocation"), d->weatherEngine);
        d->location.getDefault();
    } else {
        d->weatherEngine->connectSource(d->source, this, d->updateInterval * 60 * 1000);
    }
}

void WeatherPopupApplet::createConfigurationInterface(KConfigDialog *parent)
{
    d->weatherConfig = new WeatherConfig(parent);
    d->weatherConfig->setDataEngine(d->weatherEngine);
    d->weatherConfig->setSource(d->source);
    d->weatherConfig->setUpdateInterval(d->updateInterval);
    d->weatherConfig->setTemperatureUnit(d->temperatureUnit);
    d->weatherConfig->setSpeedUnit(d->speedUnit);
    d->weatherConfig->setPressureUnit(d->pressureUnit);
    d->weatherConfig->setVisibilityUnit(d->visibilityUnit);
    parent->addPage(d->weatherConfig, i18n("Weather"), icon());
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
}

void WeatherPopupApplet::configAccepted()
{
    setConfigurationRequired(false);
    if (!d->source.isEmpty()) {
        d->weatherEngine->disconnectSource(d->source, this);
    }

    d->temperatureUnit = d->weatherConfig->temperatureUnit();
    d->speedUnit = d->weatherConfig->speedUnit();
    d->pressureUnit = d->weatherConfig->pressureUnit();
    d->visibilityUnit = d->weatherConfig->visibilityUnit();
    d->updateInterval = d->weatherConfig->updateInterval();
    d->source = d->weatherConfig->source();

    KConfigGroup cfg = config();
    cfg.writeEntry("temperatureUnit", d->temperatureUnit);
    cfg.writeEntry("speedUnit", d->speedUnit);
    cfg.writeEntry("pressureUnit", d->pressureUnit);
    cfg.writeEntry("visibilityUnit", d->visibilityUnit);
    cfg.writeEntry("updateInterval", d->updateInterval);
    cfg.writeEntry("source", d->source);

    emit configNeedsSaving();
    connectToEngine();
}

void WeatherPopupApplet::dataUpdated(const QString& source,
                                     const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(source)

    if (data.isEmpty()) {
        return;
    }

    d->conditionIcon = data["Condition Icon"].toString();
    if (data["Pressure"].toString() != "N/A") {
        d->pressure = Value(data["Pressure"],
                            WeatherUtils::getUnitString(data["Pressure Unit"].toInt()));
    } else {
        d->pressure = Value();
    }
    d->tend = data["Pressure Tendency"].toString();
    d->temperature = Value(data["Temperature"],
                           WeatherUtils::getUnitString(data["Temperature Unit"].toInt(), true));
    d->latitude = data["Latitude"].toDouble();
    d->longitude = data["Longitude"].toDouble();

    d->busyTimer->stop();
    showMessage(QIcon(), QString(), Plasma::ButtonNone);
    setBusy(false);
}

QString WeatherPopupApplet::pressureUnit()
{
    return d->pressureUnit;
}

QString WeatherPopupApplet::temperatureUnit()
{
    return d->temperatureUnit;
}

QString WeatherPopupApplet::speedUnit()
{
    return d->speedUnit;
}

QString WeatherPopupApplet::visibilityUnit()
{
    return d->visibilityUnit;
}

QString WeatherPopupApplet::conditionIcon()
{
    if (d->conditionIcon.isEmpty() || d->conditionIcon == "weather-none-available") {
        d->conditionIcon = d->conditionFromPressure();
    }
    return d->conditionIcon;
}

WeatherConfig* WeatherPopupApplet::weatherConfig()
{
    return d->weatherConfig;
}

int WeatherPopupApplet::pressureUnitInt()
{
    return d->unitMap[pressureUnit()];
}

int WeatherPopupApplet::temperatureUnitInt()
{
    return d->unitMap[temperatureUnit()];
}

int WeatherPopupApplet::speedUnitInt()
{
    return d->unitMap[speedUnit()];
}

int WeatherPopupApplet::visibilityUnitInt()
{
    return d->unitMap[visibilityUnit()];
}

#include "weatherpopupapplet.moc"
