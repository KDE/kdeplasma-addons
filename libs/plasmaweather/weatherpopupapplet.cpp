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

namespace {
namespace AppletConfigKeys {
inline QString temperatureUnitId() { return QStringLiteral("temperatureUnitId"); }
inline QString windSpeedUnitId()   { return QStringLiteral("windSpeedUnitId"); }
inline QString pressureUnitId()    { return QStringLiteral("pressureUnitId"); }
inline QString visibilityUnitId()  { return QStringLiteral("visibilityUnitId"); }
inline QString updateInterval()    { return QStringLiteral("updateInterval"); }
inline QString source()            { return QStringLiteral("source"); }
}
namespace StorageConfigKeys {
const char temperatureUnit[] = "temperatureUnit";
const char speedUnit[] =       "speedUnit";
const char pressureUnit[] =    "pressureUnit";
const char visibilityUnit[] =  "visibilityUnit";
const char updateInterval[] =  "updateInterval";
const char source[] =          "source";
}
namespace DataEngineIds {
inline QString weather() { return QStringLiteral("weather"); }
}
}

namespace Plasma {

class WeatherPopupAppletPrivate
{
public:
    WeatherPopupAppletPrivate(WeatherPopupApplet *weatherapplet)
        : q(weatherapplet)
        , updateInterval(0)
        , location(nullptr)
        , latitude(qQNaN())
        , longitude(qQNaN())
        , timeoutNotification(nullptr)
    {
        busyTimer = new QTimer(q);
        busyTimer->setInterval(2*60*1000);
        busyTimer->setSingleShot(true);
        QObject::connect(busyTimer, &QTimer::timeout,
                         q, [&]() { giveUpBeingBusy(); });
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
    QMetaObject::Connection timeoutNotificationConnection;

    bool isValidLatitude() {
        return -90 <= latitude && latitude <= 90;
    }
    bool isValidLongitude() {
        return -180 <= longitude && longitude <= 180;
    }

    void locationReady(const QString &src)
    {
        if (!src.isEmpty()) {
            source = src;
            KConfigGroup cfg = q->config();
            cfg.writeEntry(StorageConfigKeys::source, source);
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
            // seems global disconnect with wildcard does not cover lambdas, so remembering manually for disconnect
            timeoutNotificationConnection =
                QObject::connect(timeoutNotification, &KNotification::closed,
                                 q, [&]() { onTimeoutNotificationClosed(); });
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
    , d(new WeatherPopupAppletPrivate(this))
{
}

WeatherPopupApplet::~WeatherPopupApplet() = default;

void WeatherPopupApplet::init()
{
    configChanged();
}

void WeatherPopupApplet::connectToEngine()
{
    if (d->timeoutNotification) {
        QObject::disconnect(d->timeoutNotificationConnection);
        d->timeoutNotification = nullptr;
    }

    const bool missingLocation = d->source.isEmpty();

    if (missingLocation) {
// TODO: fix WeatherLocation from using no longer existing bbcukmet ion for getting weather station for current position
#if 0
        if (!d->location) {
            d->location = new WeatherLocation(this);
            connect(d->location, &WeatherLocation::finished,
                    this, [&](const QString& source) { d->locationReady(source); });
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

        Plasma::DataEngine* weatherDataEngine = dataEngine(DataEngineIds::weather());
        weatherDataEngine->connectSource(d->source, this, d->updateInterval * 60 * 1000);
    }
}

void WeatherPopupApplet::saveConfig(const QVariantMap& configChanges)
{
    KConfigGroup cfg = config();

    // units
    auto it = configChanges.find(AppletConfigKeys::temperatureUnitId());
    if (it != configChanges.end()) {
        cfg.writeEntry(StorageConfigKeys::temperatureUnit, it.value().toInt());
    }
    it = configChanges.find(AppletConfigKeys::windSpeedUnitId());
    if (it != configChanges.end()) {
        cfg.writeEntry(StorageConfigKeys::speedUnit, it.value().toInt());
    }
    it = configChanges.find(AppletConfigKeys::pressureUnitId());
    if (it != configChanges.end()) {
        cfg.writeEntry(StorageConfigKeys::pressureUnit, it.value().toInt());
    }
    it = configChanges.find(AppletConfigKeys::visibilityUnitId());
    if (it != configChanges.end()) {
        cfg.writeEntry(StorageConfigKeys::visibilityUnit, it.value().toInt());
    }

    // data source
    it = configChanges.find(AppletConfigKeys::updateInterval());
    if (it != configChanges.end()) {
        cfg.writeEntry(StorageConfigKeys::updateInterval, it.value().toInt());
    }
    it = configChanges.find(AppletConfigKeys::source());
    if (it != configChanges.end()) {
        cfg.writeEntry(StorageConfigKeys::source, it.value().toString());
    }

    emit configNeedsSaving();
}

void WeatherPopupApplet::configChanged()
{
    if (!d->source.isEmpty()) {
        Plasma::DataEngine* weatherDataEngine = dataEngine(DataEngineIds::weather());
        weatherDataEngine->disconnectSource(d->source, this);
    }

    KConfigGroup cfg = config();

    const bool useMetric = (QLocale().measurementSystem() == QLocale::MetricSystem);

    d->temperatureUnit = d->unit(cfg.readEntry(StorageConfigKeys::temperatureUnit, (useMetric ? "C" :   "F")));
    d->speedUnit =       d->unit(cfg.readEntry(StorageConfigKeys::speedUnit,       (useMetric ? "m/s" : "mph")));
    d->pressureUnit =    d->unit(cfg.readEntry(StorageConfigKeys::pressureUnit,    (useMetric ? "hPa" : "inHg")));
    d->visibilityUnit =  d->unit(cfg.readEntry(StorageConfigKeys::visibilityUnit,  (useMetric ? "km" :  "ml")));

    d->updateInterval = cfg.readEntry(StorageConfigKeys::updateInterval, 30);
    d->source =         cfg.readEntry(StorageConfigKeys::source,         QString());

    setConfigurationRequired(d->source.isEmpty());

    connectToEngine();
}

inline double readGeoDouble(const Plasma::DataEngine::Data &data, const QString &key)
{
    const Plasma::DataEngine::Data::ConstIterator it = data.find(key);

    if (it == data.end()) {
        return qQNaN();
    }

    bool ok = false;
    double result = it.value().toDouble(&ok);
    if (!ok) {
        result = qQNaN();
    }
    return result;
}

void WeatherPopupApplet::dataUpdated(const QString& source,
                                     const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(source);

    if (data.isEmpty()) {
        return;
    }

    d->conditionIcon = data[QStringLiteral("Condition Icon")].toString();
    const auto pressure = data[QStringLiteral("Pressure")];
    if (pressure.toString() != QLatin1String( "N/A" )) {
        d->pressure = Value(pressure.toDouble(),
                            static_cast<UnitId>(data[QStringLiteral("Pressure Unit")].toInt()));
    } else {
        d->pressure = Value();
    }
    d->tend = data[QStringLiteral("Pressure Tendency")].toString();
    d->temperature = Value(data[QStringLiteral("Temperature")].toDouble(),
                           static_cast<UnitId>(data[QStringLiteral("Temperature Unit")].toInt()));
    d->latitude = readGeoDouble(data, QStringLiteral("Latitude"));
    d->longitude = readGeoDouble(data, QStringLiteral("Longitude"));
    const QString creditUrl = data[QStringLiteral("Credit Url")].toString();
    QList<QUrl> associatedApplicationUrls;
    if (!creditUrl.isEmpty()) {
        associatedApplicationUrls << QUrl(creditUrl);
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
    return QVariantMap {
        // units
        { AppletConfigKeys::temperatureUnitId(), d->temperatureUnit.id() },
        { AppletConfigKeys::windSpeedUnitId(),   d->speedUnit.id() },
        { AppletConfigKeys::pressureUnitId(),    d->pressureUnit.id() },
        { AppletConfigKeys::visibilityUnitId(),  d->visibilityUnit.id() },

        // data source
        { AppletConfigKeys::updateInterval(), d->updateInterval },
        { AppletConfigKeys::source(),         d->source },
    };
}


QString WeatherPopupApplet::conditionIcon()
{
    if (d->conditionIcon.isEmpty() || d->conditionIcon == QLatin1String( "weather-none-available" )) {
        d->conditionIcon = d->conditionFromPressure();
    }
    return d->conditionIcon;
}

}
