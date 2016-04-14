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

#include <KConfigGroup>
#include <KLocalizedString>
#include <KNotification>

#include <KUnitConversion/Converter>

#include "weatherlocation.h"

using namespace KUnitConversion;

class Q_DECL_HIDDEN WeatherPopupApplet::Private
{
public:
    Private(WeatherPopupApplet *weatherapplet)
        : q(weatherapplet)
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
            q->setBusy(false);
            q->setConfigurationRequired(true);
        }

        location->deleteLater();
        location = nullptr;
    }

    void giveUpBeingBusy()
    {
        q->setBusy(false);

        QStringList list = source.split(QLatin1Char( '|' ), QString::SkipEmptyParts);
        if (list.count() < 3) {
            q->setConfigurationRequired(true);
        } else {
            timeoutNotification =
                KNotification::event( KNotification::Error, QString(), // TODO: some title?
                                      i18n("Weather information retrieval for %1 timed out.", list.value(2)),
                                      QStringLiteral("dialog-error"));
            QObject::connect(timeoutNotification, SIGNAL(closed()), q, SLOT(onTimeoutNotificationClosed()));
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
            return QStringLiteral("weather-none-available");
        }
        qreal temp = temperature.convertTo(Celsius).number();
        qreal p = pressure.convertTo(Kilopascal).number();
        qreal t = tendency(pressure, tend);

        // This is completely unscientific so if anyone have a better formula for this :-)
        p += t * 10;

        // PORT!
//         Plasma::DataEngine *timeEngine = dataEngine(QStringLiteral("time"));
//         Plasma::DataEngine::Data data = timeEngine->query(
//                 QString(QLatin1String( "Local|Solar|Latitude=%1|Longitude=%2" )).arg(latitude).arg(longitude));
        bool day = true;//(data[QLatin1String( "Corrected Elevation" )].toDouble() > 0.0);

        if (p > 103.0) {
            if (day) {
                result = QStringLiteral("weather-clear");
            } else {
                result = QStringLiteral("weather-clear-night");
            }
        } else if (p > 100.0) {
            if (day) {
                result = QStringLiteral("weather-clouds");
            } else {
                result = QStringLiteral("weather-clouds-night");
            }
        } else if (p > 99.0) {
            if (day) {
                if (temp > 1.0) {
                    result = QStringLiteral("weather-showers-scattered-day");
                } else if (temp < -1.0)  {
                    result = QStringLiteral("weather-snow-scattered-day");
                } else {
                    result = QStringLiteral("weather-snow-rain");
                }
            } else {
                if (temp > 1.0) {
                    result = QStringLiteral("weather-showers-scattered-night");
                } else if (temp < -1.0)  {
                    result = QStringLiteral("weather-snow-scattered-night");
                } else {
                    result = QStringLiteral("weather-snow-rain");
                }
            }
        } else {
            if (temp > 1.0) {
                result = QStringLiteral("weather-showers");
            } else if (temp < -1.0)  {
                result = QStringLiteral("weather-snow");
            } else {
                result = QStringLiteral("weather-snow-rain");
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
    configChanged();
}

void WeatherPopupApplet::connectToEngine()
{
    if (d->timeoutNotification) {
        QObject::disconnect(d->timeoutNotification, SIGNAL(closed()), this, SLOT(onTimeoutNotificationClosed()));
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

        Plasma::DataEngine *dataEngine = dataEngine(QStringLiteral("geolocation"));
        d->location->setDataEngines(dataEngine, d->weatherEngine);
        d->location->getDefault();
#endif
        setBusy(false);
        // TODO: remove once WeatherLocation is fixed above
        // fake d->location returning no result
        d->locationReady(QString());
    } else {
        delete d->location;
        d->location = nullptr;

        setBusy(true);
        d->busyTimer->start();

        Plasma::DataEngine* weatherDataEngine = dataEngine(QStringLiteral("weather"));
        weatherDataEngine->connectSource(d->source, this, d->updateInterval * 60 * 1000);
    }
}

void WeatherPopupApplet::saveConfig(const QVariantMap& configChanges)
{
    KConfigGroup cfg = config();

    // units
    if (configChanges.contains(QStringLiteral("temperatureUnitId"))) {
        cfg.writeEntry("temperatureUnit", configChanges.value(QStringLiteral("temperatureUnitId")).toInt());
    }
    if (configChanges.contains(QStringLiteral("windSpeedUnitId"))) {
        cfg.writeEntry("speedUnit", configChanges.value(QStringLiteral("windSpeedUnitId")).toInt());
    }
    if (configChanges.contains(QStringLiteral("pressureUnitId"))) {
        cfg.writeEntry("pressureUnit", configChanges.value(QStringLiteral("pressureUnitId")).toInt());
    }
    if (configChanges.contains(QStringLiteral("visibilityUnitId"))) {
        cfg.writeEntry("visibilityUnit", configChanges.value(QStringLiteral("visibilityUnitId")).toInt());
    }

    // data source
    if (configChanges.contains(QStringLiteral("updateInterval"))) {
        cfg.writeEntry("updateInterval", configChanges.value(QStringLiteral("updateInterval")).toInt());
    }
    if (configChanges.contains(QStringLiteral("source"))) {
        cfg.writeEntry("source", configChanges.value(QStringLiteral("source")).toString());
    }

    emit configNeedsSaving();
}

void WeatherPopupApplet::configChanged()
{
    if (!d->source.isEmpty()) {
        Plasma::DataEngine* weatherDataEngine = dataEngine(QStringLiteral("weather"));
        weatherDataEngine->disconnectSource(d->source, this);
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
    Q_UNUSED(source);

    if (data.isEmpty()) {
        return;
    }

    d->conditionIcon = data[QStringLiteral("Condition Icon")].toString();
    if (data[QLatin1String( "Pressure" )].toString() != QLatin1String( "N/A" )) {
        d->pressure = Value(data[QStringLiteral("Pressure")].toDouble(),
                            static_cast<UnitId>(data[QStringLiteral("Pressure Unit")].toInt()));
    } else {
        d->pressure = Value();
    }
    d->tend = data[QStringLiteral("Pressure Tendency")].toString();
    d->temperature = Value(data[QStringLiteral("Temperature")].toDouble(),
                           static_cast<UnitId>(data[QStringLiteral("Temperature Unit")].toInt()));
    d->latitude = data[QStringLiteral("Latitude")].toDouble();
    d->longitude = data[QStringLiteral("Longitude")].toDouble();
    const QString creditUrl = data[QStringLiteral("Credit Url")].toString();
    QList<QUrl> associatedApplicationUrls;
    if (!creditUrl.isEmpty()) {
        associatedApplicationUrls << creditUrl;
    }
    setAssociatedApplicationUrls(associatedApplicationUrls);

    d->busyTimer->stop();
    if (d->timeoutNotification) {
        d->timeoutNotification->close();
    }
    setBusy(false);
}

Unit WeatherPopupApplet::pressureUnit() const
{
    return d->pressureUnit;
}

Unit WeatherPopupApplet::temperatureUnit() const
{
    return d->temperatureUnit;
}

Unit WeatherPopupApplet::speedUnit() const
{
    return d->speedUnit;
}

Unit WeatherPopupApplet::visibilityUnit() const
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
    config.insert(QStringLiteral("temperatureUnitId"), d->temperatureUnit.id());
    config.insert(QStringLiteral("windSpeedUnitId"), d->speedUnit.id());
    config.insert(QStringLiteral("pressureUnitId"), d->pressureUnit.id());
    config.insert(QStringLiteral("visibilityUnitId"), d->visibilityUnit.id());

    // data source
    config.insert(QStringLiteral("updateInterval"), d->updateInterval);
    config.insert(QStringLiteral("source"), d->source);

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
