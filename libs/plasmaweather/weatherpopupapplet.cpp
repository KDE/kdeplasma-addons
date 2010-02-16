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
#include <KUnitConversion/Converter>

#include "weatherconfig.h"
#include "weatherlocation.h"

using namespace KUnitConversion;

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
        busyTimer = new QTimer(q);
        busyTimer->setSingleShot(true);
        busyTimer->setInterval(2*60*1000);
        QObject::connect(busyTimer, SIGNAL(timeout()), q, SLOT(giveUpBeingBusy()));
    }

    WeatherPopupApplet *q;
    WeatherConfig *weatherConfig;
    Plasma::DataEngine *weatherEngine;
    Plasma::DataEngine *timeEngine;
    Converter converter;
    UnitPtr temperatureUnit;
    UnitPtr speedUnit;
    UnitPtr pressureUnit;
    UnitPtr visibilityUnit;
    int updateInterval;
    QString source;
    WeatherLocation location;

    QString conditionIcon;
    QString tend;
    Value pressure;
    Value temperature;
    double latitude;
    double longitude;
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

    qreal tendency(const Value& pressure, const QString& tendency)
    {
        qreal t;

        if (tendency.toLower() == "rising") {
            t = 0.75;
        } else if (tendency.toLower() == "falling") {
            t = -0.75;
        } else {
            t = Value(tendency.toDouble(), pressure.unit()).convertTo(Kilopascal).number();
        }
        return t;
    }

    QString conditionFromPressure()
    {
        QString result;
        if (!pressure.isValid()) {
            return "weather-none-available";
        }
        qreal temp = temperature.convertTo(Celsius).number();
        qreal p = pressure.convertTo(Kilopascal).number();
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

    UnitPtr unit(const QString& unit)
    {
        if (!unit.isEmpty() && unit[0].isDigit()) {
            return converter.unit(unit.toInt());
        } else {
            // Support < 4.4 config values
            return converter.unit(unit);
        }
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
    configChanged();
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
    d->weatherConfig->setTemperatureUnit(d->temperatureUnit->id());
    d->weatherConfig->setSpeedUnit(d->speedUnit->id());
    d->weatherConfig->setPressureUnit(d->pressureUnit->id());
    d->weatherConfig->setVisibilityUnit(d->visibilityUnit->id());
    parent->addPage(d->weatherConfig, i18n("Weather"), icon());
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
}

void WeatherPopupApplet::configAccepted()
{
    d->temperatureUnit = d->converter.unit(d->weatherConfig->temperatureUnit());
    d->speedUnit = d->converter.unit(d->weatherConfig->speedUnit());
    d->pressureUnit = d->converter.unit(d->weatherConfig->pressureUnit());
    d->visibilityUnit = d->converter.unit(d->weatherConfig->visibilityUnit());
    d->updateInterval = d->weatherConfig->updateInterval();
    d->source = d->weatherConfig->source();

    KConfigGroup cfg = config();
    cfg.writeEntry("temperatureUnit", d->temperatureUnit->id());
    cfg.writeEntry("speedUnit", d->speedUnit->id());
    cfg.writeEntry("pressureUnit", d->pressureUnit->id());
    cfg.writeEntry("visibilityUnit", d->visibilityUnit->id());
    cfg.writeEntry("updateInterval", d->updateInterval);
    cfg.writeEntry("source", d->source);

    emit configNeedsSaving();
}

void WeatherPopupApplet::configChanged()
{
    setConfigurationRequired(false);
    if (!d->source.isEmpty()) {
        d->weatherEngine->disconnectSource(d->source, this);
    }

    KConfigGroup cfg = config();

    if (KGlobal::locale()->measureSystem() == KLocale::Metric) {
        d->temperatureUnit = d->unit(cfg.readEntry("temperatureUnit", "C"));
        d->speedUnit = d->unit(cfg.readEntry("speedUnit", "m/s"));
        d->pressureUnit = d->unit(cfg.readEntry("pressureUnit", "hPa"));
        d->visibilityUnit = d->unit(cfg.readEntry("visibilityUnit", "km"));
    } else {
        d->temperatureUnit = d->unit(cfg.readEntry("temperatureUnit", "F"));
        d->speedUnit = d->unit(cfg.readEntry("speedUnit", "mph"));
        d->pressureUnit = d->unit(cfg.readEntry("pressureUnit", "mb"));
        d->visibilityUnit = d->unit(cfg.readEntry("visibilityUnit", "ml"));
    }
    d->updateInterval = cfg.readEntry("updateWeather", 30);
    d->source = cfg.readEntry("source", "");

    d->weatherEngine = dataEngine("weather");
    d->timeEngine = dataEngine("time");

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
        d->pressure = Value(data["Pressure"].toDouble(), data["Pressure Unit"].toInt());
    } else {
        d->pressure = Value();
    }
    d->tend = data["Pressure Tendency"].toString();
    d->temperature = Value(data["Temperature"].toDouble(), data["Temperature Unit"].toInt());
    d->latitude = data["Latitude"].toDouble();
    d->longitude = data["Longitude"].toDouble();
    setAssociatedApplicationUrls(KUrl(data.value("Credit Url").toString()));

    d->busyTimer->stop();
    showMessage(QIcon(), QString(), Plasma::ButtonNone);
    setBusy(false);
}

UnitPtr WeatherPopupApplet::pressureUnit()
{
    return d->pressureUnit;
}

UnitPtr WeatherPopupApplet::temperatureUnit()
{
    return d->temperatureUnit;
}

UnitPtr WeatherPopupApplet::speedUnit()
{
    return d->speedUnit;
}

UnitPtr WeatherPopupApplet::visibilityUnit()
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

#include "weatherpopupapplet.moc"
