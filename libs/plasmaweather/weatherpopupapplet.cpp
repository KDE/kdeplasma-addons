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

#include <QTimer>
#include <QIcon>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KNotification>

#include <KUnitConversion/Converter>

#include <Plasma/PluginLoader>

#include "weatherlocation.h"

using namespace KUnitConversion;

class WeatherPopupApplet::Private
{
public:
    Private(WeatherPopupApplet *weatherapplet)
        : q(weatherapplet)
        , weatherEngine(nullptr)
        , timeEngine(nullptr)
        , updateInterval(0)
        , location(nullptr)
        , timeoutNotification(nullptr)
    {
        busyTimer = new QTimer(q);
        busyTimer->setInterval(2*60*1000);
        busyTimer->setSingleShot(true);
        QObject::connect(busyTimer, SIGNAL(timeout()), q, SLOT(giveUpBeingBusy()));
    }

    WeatherPopupApplet *q;
    Plasma::DataEngine *weatherEngine;
    Plasma::DataEngine *timeEngine;
    Converter converter;
    Unit temperatureUnit;
    Unit speedUnit;
    Unit pressureUnit;
    Unit visibilityUnit;
    int updateInterval;
    QString source;
    WeatherLocation *location;

    QString conditionIcon;
    QString tend;
    Value pressure;
    Value temperature;
    double latitude;
    double longitude;
    QTimer *busyTimer;
    KNotification *timeoutNotification;

    void locationReady(const QString &src)
    {
        if (!src.isEmpty()) {
            source = src;
            KConfigGroup cfg = q->config();
            cfg.writeEntry("source", source);
            emit q->configNeedsSaving();
            q->connectToEngine();
            q->setConfigurationRequired(false);
        } else {
            busyTimer->stop();
            if (timeoutNotification) {
                timeoutNotification->close();
            }
            QObject *graphicObject = q->property("_plasma_graphicObject").value<QObject *>();
            if (graphicObject) {
                graphicObject->setProperty("busy", false);
            }
            q->setConfigurationRequired(true);
        }

        location->deleteLater();
        location = nullptr;
    }

    void giveUpBeingBusy()
    {
        QObject *graphicObject = q->property("_plasma_graphicObject").value<QObject *>();
        if (graphicObject) {
            graphicObject->setProperty("busy", false);
        }

        QStringList list = source.split(QLatin1Char( '|' ), QString::SkipEmptyParts);
        if (list.count() < 3) {
            q->setConfigurationRequired(true);
        } else {
            timeoutNotification =
                KNotification::event( KNotification::Error, QString(), // TODO: some title?
                                      i18n("Weather information retrieval for %1 timed out.", list.value(2)),
                                      QLatin1String("dialog-error"));
            QObject::connect(timeoutNotification, SIGNAL(close()), q, SLOT(onTimeoutNotificationClosed()));
        }
    }

    void onTimeoutNotificationClosed()
    {
        timeoutNotification = nullptr;
    }

    qreal tendency(const Value& pressure, const QString& tendency)
    {
        qreal t;

        if (tendency.toLower() == QLatin1String( "rising" )) {
            t = 0.75;
        } else if (tendency.toLower() == QLatin1String( "falling" )) {
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
            return QLatin1String( "weather-none-available" );
        }
        qreal temp = temperature.convertTo(Celsius).number();
        qreal p = pressure.convertTo(Kilopascal).number();
        qreal t = tendency(pressure, tend);

        // This is completely unscientific so if anyone have a better formula for this :-)
        p += t * 10;

        // PORT!
//         if (!timeEngine) {
//             timeEngine = Plasma::PluginLoader::self()->loadDataEngine( QStringLiteral("time") );
//         }
//         Plasma::DataEngine::Data data = timeEngine->query(
//                 QString(QLatin1String( "Local|Solar|Latitude=%1|Longitude=%2" )).arg(latitude).arg(longitude));
        bool day = true;//(data[QLatin1String( "Corrected Elevation" )].toDouble() > 0.0);

        if (p > 103.0) {
            if (day) {
                result = QLatin1String( "weather-clear" );
            } else {
                result = QLatin1String( "weather-clear-night" );
            }
        } else if (p > 100.0) {
            if (day) {
                result = QLatin1String( "weather-clouds" );
            } else {
                result = QLatin1String( "weather-clouds-night" );
            }
        } else if (p > 99.0) {
            if (day) {
                if (temp > 1.0) {
                    result = QLatin1String( "weather-showers-scattered-day" );
                } else if (temp < -1.0)  {
                    result = QLatin1String( "weather-snow-scattered-day" );
                } else {
                    result = QLatin1String( "weather-snow-rain" );
                }
            } else {
                if (temp > 1.0) {
                    result = QLatin1String( "weather-showers-scattered-night" );
                } else if (temp < -1.0)  {
                    result = QLatin1String( "weather-snow-scattered-night" );
                } else {
                    result = QLatin1String( "weather-snow-rain" );
                }
            }
        } else {
            if (temp > 1.0) {
                result = QLatin1String( "weather-showers" );
            } else if (temp < -1.0)  {
                result = QLatin1String( "weather-snow" );
            } else {
                result = QLatin1String( "weather-snow-rain" );
            }
        }
        //kDebug() << result;
        return result;
    }

    Unit unit(const QString& unit)
    {
        if (!unit.isEmpty() && unit[0].isDigit()) {
            // PORT?
            return converter.unit(static_cast<UnitId>(unit.toInt()));
        }
        // Support < 4.4 config values
        return converter.unit(unit);
    }
};

WeatherPopupApplet::WeatherPopupApplet(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args)
    , d(new Private(this))
{
}

WeatherPopupApplet::~WeatherPopupApplet()
{
    delete d;
}

void WeatherPopupApplet::init()
{
    // workaround for weather dataengine being destructed behind our back when there are no
    // current connections for a while
    // get the weather dataengine and do a dummy connect for ions source, never to be disconnected
    // this will keep the refcount of connections for the dataengine always above 0
    d->weatherEngine = Plasma::PluginLoader::self()->loadDataEngine( QStringLiteral("weather") );
    d->weatherEngine->connectSource(QStringLiteral("ions"), this);

    configChanged();
}

void WeatherPopupApplet::connectToEngine()
{
    if (d->timeoutNotification) {
        QObject::disconnect(d->timeoutNotification, SIGNAL(close()), this, SLOT(onTimeoutNotificationClosed()));
        d->timeoutNotification = nullptr;
    }

    const bool missingLocation = d->source.isEmpty();

    if (missingLocation) {
// TODO: fix WeatherLocation from using no longer existing bbcukmet ion for getting weather station for current position
#if 0
        if (!d->location) {
            d->location = new WeatherLocation(this);
            connect(d->location, SIGNAL(finished(QString)), this, SLOT(locationReady(QString)));
        }

        Plasma::DataEngine *dataEngine =
            Plasma::PluginLoader::self()->loadDataEngine( QStringLiteral("geolocation") );
        d->location->setDataEngines(dataEngine, d->weatherEngine);
        d->location->getDefault();
#endif
        QObject *graphicObject = property("_plasma_graphicObject").value<QObject *>();
        if (graphicObject) {
            graphicObject->setProperty("busy", false);
        }
        // TODO: remove once WeatherLocation is fixed above
        // fake d->location returning no result
        d->locationReady(QString());
    } else {
        delete d->location;
        d->location = nullptr;

        d->weatherEngine->connectSource(d->source, this, d->updateInterval * 60 * 1000);
        QObject *graphicObject = property("_plasma_graphicObject").value<QObject *>();
        if (graphicObject) {
            graphicObject->setProperty("busy", true);
        }
        d->busyTimer->start();
    }
}

void WeatherPopupApplet::saveConfig(const QVariantMap& configChanges)
{
    KConfigGroup cfg = config();

    // units
    if (configChanges.contains("temperatureUnitId")) {
        cfg.writeEntry("temperatureUnit", configChanges.value("temperatureUnitId").toInt());
    }
    if (configChanges.contains("windSpeedUnitId")) {
        cfg.writeEntry("speedUnit", configChanges.value("windSpeedUnitId").toInt());
    }
    if (configChanges.contains("pressureUnitId")) {
        cfg.writeEntry("pressureUnit", configChanges.value("pressureUnitId").toInt());
    }
    if (configChanges.contains("visibilityUnitId")) {
        cfg.writeEntry("visibilityUnit", configChanges.value("visibilityUnitId").toInt());
    }

    // data source
    if (configChanges.contains("updateInterval")) {
        cfg.writeEntry("updateInterval", configChanges.value("updateInterval").toInt());
    }
    if (configChanges.contains("source")) {
        cfg.writeEntry("source", configChanges.value("source").toString());
    }

    emit configNeedsSaving();
}

void WeatherPopupApplet::configChanged()
{
    if (!d->source.isEmpty()) {
        d->weatherEngine->disconnectSource(d->source, this);
    }

    KConfigGroup cfg = config();

    if (QLocale().measurementSystem() == QLocale::MetricSystem) {
        d->temperatureUnit = d->unit(cfg.readEntry("temperatureUnit", "C"));
        d->speedUnit = d->unit(cfg.readEntry("speedUnit", "m/s"));
        d->pressureUnit = d->unit(cfg.readEntry("pressureUnit", "hPa"));
        d->visibilityUnit = d->unit(cfg.readEntry("visibilityUnit", "km"));
    } else {
        d->temperatureUnit = d->unit(cfg.readEntry("temperatureUnit", "F"));
        d->speedUnit = d->unit(cfg.readEntry("speedUnit", "mph"));
        d->pressureUnit = d->unit(cfg.readEntry("pressureUnit", "inHg"));
        d->visibilityUnit = d->unit(cfg.readEntry("visibilityUnit", "ml"));
    }
    d->updateInterval = cfg.readEntry("updateInterval", 30);
    d->source = cfg.readEntry("source", QString());
    setConfigurationRequired(d->source.isEmpty());

    connectToEngine();
}

void WeatherPopupApplet::dataUpdated(const QString& source,
                                     const Plasma::DataEngine::Data &data)
{
    if (source == QLatin1String("ions")) {
        return;
    }

    if (data.isEmpty()) {
        return;
    }

    d->conditionIcon = data[QLatin1String( "Condition Icon" )].toString();
    if (data[QLatin1String( "Pressure" )].toString() != QLatin1String( "N/A" )) {
        d->pressure = Value(data[QLatin1String( "Pressure" )].toDouble(),
                            static_cast<UnitId>(data[QLatin1String( "Pressure Unit" )].toInt()));
    } else {
        d->pressure = Value();
    }
    d->tend = data[QLatin1String( "Pressure Tendency" )].toString();
    d->temperature = Value(data[QLatin1String( "Temperature" )].toDouble(),
                           static_cast<UnitId>(data[QLatin1String( "Temperature Unit" )].toInt()));
    d->latitude = data[QLatin1String( "Latitude" )].toDouble();
    d->longitude = data[QLatin1String( "Longitude" )].toDouble();
    setAssociatedApplicationUrls(QList<QUrl>() << QUrl(data.value(QLatin1String( "Credit Url" )).toString()));

    d->busyTimer->stop();
    if (d->timeoutNotification) {
        d->timeoutNotification->close();
    }
    QObject *graphicObject = property("_plasma_graphicObject").value<QObject *>();
    if (graphicObject) {
        graphicObject->setProperty("busy", false);
    }
}

Unit WeatherPopupApplet::pressureUnit()
{
    return d->pressureUnit;
}

Unit WeatherPopupApplet::temperatureUnit()
{
    return d->temperatureUnit;
}

Unit WeatherPopupApplet::speedUnit()
{
    return d->speedUnit;
}

Unit WeatherPopupApplet::visibilityUnit()
{
    return d->visibilityUnit;
}


QString WeatherPopupApplet::source() const
{
    return d->source;
}

QVariantMap WeatherPopupApplet::configValues() const
{
    QVariantMap config;

    // units
    config.insert("temperatureUnitId", d->temperatureUnit.id());
    config.insert("windSpeedUnitId", d->speedUnit.id());
    config.insert("pressureUnitId", d->pressureUnit.id());
    config.insert("visibilityUnitId", d->visibilityUnit.id());

    // data source
    config.insert("updateInterval", d->updateInterval);
    config.insert("source", d->source);

    return config;
}


QString WeatherPopupApplet::conditionIcon()
{
    if (d->conditionIcon.isEmpty() || d->conditionIcon == QLatin1String( "weather-none-available" )) {
        d->conditionIcon = d->conditionFromPressure();
    }
    return d->conditionIcon;
}


// needed due to private slots
#include "moc_weatherpopupapplet.cpp"
