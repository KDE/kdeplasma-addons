/***************************************************************************
 *   Copyright (C) 2007-2009 by Shawn Starr <shawn.starr@rogers.com>       *
 *   Copyright (C) 2008 by Marco Martin <notmart@gmail.com>                *
 *   Copyright (C) 2012 by Luís Gabriel Lima <lampih@gmail.com>            *
 *   Copyright (C) 2017-2018 Friedrich W. H. Kossebau <kossebau@kde.org>   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "weatherapplet.h"

// KF
#include <KLocalizedString>
#include <KIconLoader>
#include <KConfigGroup>
#include <KUnitConversion/Value>
#include <KNotification>

#include <Plasma/Package>
// Qt
#include <QTimer>
// Std
#include <cmath>

using namespace KUnitConversion;

template <typename T>
T clampValue(T value, int decimals)
{
    const T mul = std::pow(static_cast<T>(10), decimals);
    return int(value * mul) / mul;
}

namespace {
namespace AppletConfigKeys {
inline QString services()                     { return QStringLiteral("services"); }
inline QString showTemperatureInCompactMode() { return QStringLiteral("showTemperatureInCompactMode"); }
inline QString temperatureUnitId()            { return QStringLiteral("temperatureUnitId"); }
inline QString windSpeedUnitId()              { return QStringLiteral("windSpeedUnitId"); }
inline QString pressureUnitId()               { return QStringLiteral("pressureUnitId"); }
inline QString visibilityUnitId()             { return QStringLiteral("visibilityUnitId"); }
inline QString updateInterval()               { return QStringLiteral("updateInterval"); }
inline QString source()                       { return QStringLiteral("source"); }
}
namespace StorageConfigKeys {
const char weatherServiceProviders[] =      "weatherServiceProviders";
const char showTemperatureInCompactMode[] = "showTemperatureInCompactMode";
const char temperatureUnit[] =              "temperatureUnit";
const char speedUnit[] =                    "speedUnit";
const char pressureUnit[] =                 "pressureUnit";
const char visibilityUnit[] =               "visibilityUnit";
const char updateInterval[] =               "updateInterval";
const char source[] =                       "source";
}
namespace PanelModelKeys {
inline QString location()                  { return QStringLiteral("location"); }
inline QString currentDayLowTemperature()  { return QStringLiteral("currentDayLowTemperature"); }
inline QString currentDayHighTemperature() { return QStringLiteral("currentDayHighTemperature"); }
inline QString currentConditions()         { return QStringLiteral("currentConditions"); }
inline QString currentTemperature()        { return QStringLiteral("currentTemperature"); }
inline QString currentConditionIcon()      { return QStringLiteral("currentConditionIcon"); }
inline QString totalDays()                 { return QStringLiteral("totalDays"); }
inline QString courtesy()                  { return QStringLiteral("courtesy"); }
inline QString creditUrl()                 { return QStringLiteral("creditUrl"); }
}
namespace NoticesKeys {
inline QString description() { return QStringLiteral("description"); }
inline QString info()        { return QStringLiteral("info"); }
}
namespace DataEngineIds {
inline QString weather() { return QStringLiteral("weather"); }
}
}

/**
 * Returns the @p iconName if the current icon theme contains an icon with that name,
 * otherwise returns "weather-not-available" (expecting the icon theme to have that in any case).
 */
QString existingWeatherIconName(const QString &iconName)
{
    const bool isValid = !iconName.isEmpty() &&
           !KIconLoader::global()->loadIcon(iconName, KIconLoader::Desktop, 0,
                                            KIconLoader::DefaultState, QStringList(), nullptr, true).isNull();
    return isValid ? iconName : QStringLiteral("weather-not-available");
}


WeatherApplet::WeatherApplet(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args)
{
    m_busyTimer = new QTimer(this);
    m_busyTimer->setInterval(2*60*1000); // 2 min
    m_busyTimer->setSingleShot(true);
    connect(m_busyTimer, &QTimer::timeout,
            this, &WeatherApplet::giveUpBeingBusy);
}

void WeatherApplet::init()
{
    resetPanelModel();

    configChanged();
}

void WeatherApplet::configChanged()
{
    if (!m_source.isEmpty()) {
        Plasma::DataEngine* weatherDataEngine = dataEngine(DataEngineIds::weather());
        weatherDataEngine->disconnectSource(m_source, this);
    }

    KConfigGroup cfg = config();

    const bool useMetric = (QLocale().measurementSystem() == QLocale::MetricSystem);

    m_displayTemperatureUnit = unit(cfg.readEntry(StorageConfigKeys::temperatureUnit, (useMetric ? "C" :   "F")));
    m_displaySpeedUnit =       unit(cfg.readEntry(StorageConfigKeys::speedUnit,       (useMetric ? "m/s" : "mph")));
    m_displayPressureUnit =    unit(cfg.readEntry(StorageConfigKeys::pressureUnit,    (useMetric ? "hPa" : "inHg")));
    m_displayVisibilityUnit =  unit(cfg.readEntry(StorageConfigKeys::visibilityUnit,  (useMetric ? "km" :  "ml")));

    m_updateInterval = cfg.readEntry(StorageConfigKeys::updateInterval, 30);
    m_source =         cfg.readEntry(StorageConfigKeys::source,         QString());

    setConfigurationRequired(m_source.isEmpty());

    connectToEngine();

    m_configuration.insert(AppletConfigKeys::showTemperatureInCompactMode(),
                           cfg.readEntry(StorageConfigKeys::showTemperatureInCompactMode, false));
    emit configurationChanged();
}

WeatherApplet::~WeatherApplet()
{
}

QString WeatherApplet::convertTemperature(const KUnitConversion::Unit& format, const QVariant& value,
                                          int type, bool rounded, bool degreesOnly)
{
    return convertTemperature(format, value.toFloat(), type, rounded, degreesOnly);
}

QString WeatherApplet::convertTemperature(const KUnitConversion::Unit& format, float value,
                                          int type, bool rounded, bool degreesOnly)
{
    KUnitConversion::Value v(value, static_cast<KUnitConversion::UnitId>(type));
    v = v.convertTo(format);

    const QString unit = degreesOnly ? i18nc("Degree, unit symbol", "°") : v.unit().symbol();

    if (rounded) {
        int tempNumber = qRound(v.number());
        return i18nc("temperature, unit", "%1%2", tempNumber, unit);
    }

    const QString formattedTemp = QLocale().toString(clampValue(v.number(), 1), 'f', 1);
    return i18nc("temperature, unit", "%1%2", formattedTemp, unit);
}

bool WeatherApplet::isValidData(const QString &data) const
{
    return (!data.isEmpty() && data != i18n("N/A"));
}

bool WeatherApplet::isValidData(const QVariant &data) const
{
    return isValidData(data.toString());
}

void WeatherApplet::resetPanelModel()
{
    m_panelModel[PanelModelKeys::location()] = QString();
    m_panelModel[PanelModelKeys::currentDayLowTemperature()] = QString();
    m_panelModel[PanelModelKeys::currentDayHighTemperature()] = QString();
    m_panelModel[PanelModelKeys::currentConditions()] = QString();
    m_panelModel[PanelModelKeys::currentTemperature()] = QString();
    m_panelModel[PanelModelKeys::currentConditionIcon()] = QString();
    m_panelModel[PanelModelKeys::totalDays()] = QString();
    m_panelModel[PanelModelKeys::courtesy()] = QString();
    m_panelModel[PanelModelKeys::creditUrl()] = QString();
}

void WeatherApplet::updatePanelModel(const Plasma::DataEngine::Data &data)
{
    resetPanelModel();

    m_panelModel[PanelModelKeys::location()] = data[QStringLiteral("Place")].toString();

    const int reportTemperatureUnit = data[QStringLiteral("Temperature Unit")].toInt();

    // Get current time period of day
    const QStringList fiveDayTokens = data[QStringLiteral("Short Forecast Day 0")].toString().split(QLatin1Char('|'));

    if (fiveDayTokens.count() == 6) {

        const QString& reportLowString = fiveDayTokens[4];
        if (reportLowString != QLatin1String("N/A") && !reportLowString.isEmpty()) {
            m_panelModel[PanelModelKeys::currentDayLowTemperature()] =
                convertTemperature(m_displayTemperatureUnit, reportLowString, reportTemperatureUnit, true);
        }

        const QString& reportHighString = fiveDayTokens[3];
        if (reportHighString != QLatin1String("N/A") && !reportHighString.isEmpty()) {
            m_panelModel[PanelModelKeys::currentDayHighTemperature()] =
                convertTemperature(m_displayTemperatureUnit, reportHighString, reportTemperatureUnit, true);
        }
    }

    m_panelModel[PanelModelKeys::currentConditions()] = data[QStringLiteral("Current Conditions")].toString().trimmed();

    const QVariant temperature = data[QStringLiteral("Temperature")];
    if (isValidData(temperature)) {
        m_panelModel[PanelModelKeys::currentTemperature()] = convertTemperature(m_displayTemperatureUnit, temperature, reportTemperatureUnit);
    }

    const QString conditionIconName = data[QStringLiteral("Condition Icon")].toString();
    QString weatherIconName;
    // specific icon?
    if (!conditionIconName.isEmpty() &&
        conditionIconName != QLatin1String("weather-none-available") &&
        conditionIconName != QLatin1String("N/U") && // TODO: N/U and N/A should not be used here, fix dataengines
        conditionIconName != QLatin1String("N/A")) {

        weatherIconName = existingWeatherIconName(conditionIconName);
    } else {
        // icon to use from current weather forecast?
        if (fiveDayTokens.count() == 6 && fiveDayTokens[1] != QLatin1String("N/U")) {
            // show the current weather
            weatherIconName = existingWeatherIconName(fiveDayTokens[1]);
        } else {
            weatherIconName = QStringLiteral("weather-none-available");
        }
    }
    m_panelModel[PanelModelKeys::currentConditionIcon()] = weatherIconName;

    m_panelModel[PanelModelKeys::courtesy()] =  data[QStringLiteral("Credit")].toString();
    m_panelModel[PanelModelKeys::creditUrl()] = data[QStringLiteral("Credit Url")].toString();
}

void WeatherApplet::updateForecastModel(const Plasma::DataEngine::Data &data)
{
    const int foreCastDayCount = data[QStringLiteral("Total Weather Days")].toInt();
    if (foreCastDayCount <= 0) {
        return;
    }

    m_forecastModel.clear();

    const int reportTemperatureUnit = data[QStringLiteral("Temperature Unit")].toInt();

    QStringList dayItems;
    QStringList conditionItems; // Icon
    QStringList hiItems;
    QStringList lowItems;

    for (int i = 0; i < foreCastDayCount; ++i) {
        const QString foreCastDayKey = QStringLiteral("Short Forecast Day %1").arg(i);
        const QStringList fiveDayTokens = data[foreCastDayKey].toString().split(QLatin1Char('|'));

        if (fiveDayTokens.count() != 6) {
            // We don't have the right number of tokens, abort trying
            break;
        }

        dayItems << fiveDayTokens[0];

        // If we see N/U (Not Used) we skip the item
        const QString& weatherIconName = fiveDayTokens[1];
        if (weatherIconName != QLatin1String("N/U") && !weatherIconName.isEmpty()) {
            QString iconAndToolTip = existingWeatherIconName(weatherIconName);

            iconAndToolTip += QLatin1Char('|');

            const QString& condition = fiveDayTokens[2];
            const QString& probability = fiveDayTokens[5];
            if (probability != QLatin1String("N/U") &&
                probability != QLatin1String("N/A") &&
                !probability.isEmpty()) {
                iconAndToolTip += i18nc("certain weather condition (probability percentage)",
                                        "%1 (%2%)", condition, probability);
            } else {
                iconAndToolTip += condition;
            }
            conditionItems << iconAndToolTip;
        }

        const QString& tempHigh = fiveDayTokens[3];
        if (tempHigh != QLatin1String("N/U")) {
            if (tempHigh == QLatin1String("N/A") || tempHigh.isEmpty()) {
                hiItems << i18nc("Short for no data available", "-");
            } else {
                hiItems << convertTemperature(m_displayTemperatureUnit,
                                              tempHigh,
                                              reportTemperatureUnit,
                                              true);
            }
        }

        const QString& tempLow = fiveDayTokens[4];
        if (tempLow != QLatin1String("N/U")) {
            if (tempLow == QLatin1String("N/A") || tempLow.isEmpty()) {
                lowItems << i18nc("Short for no data available", "-");
            } else {
                lowItems << convertTemperature(m_displayTemperatureUnit,
                                               tempLow,
                                               reportTemperatureUnit,
                                               true);
            }
        }
    }

    if (!dayItems.isEmpty()) {
        m_forecastModel << dayItems;
    }
    if (!conditionItems.isEmpty()) {
        m_forecastModel << conditionItems;
    }
    if (!hiItems.isEmpty())  {
        m_forecastModel << hiItems;
    }
    if (!lowItems.isEmpty()) {
        m_forecastModel << lowItems;
    }

    m_panelModel[PanelModelKeys::totalDays()] = i18ncp("Forecast period timeframe", "1 Day",
                                                       "%1 Days", foreCastDayCount);
}

void WeatherApplet::updateDetailsModel(const Plasma::DataEngine::Data &data)
{
    m_detailsModel.clear();

    QLocale locale;
    const QString textId = QStringLiteral("text");
    const QString iconId = QStringLiteral("icon");

    // reused map for each row
    QVariantMap row;
    row.insert(iconId, QString());
    row.insert(textId, QString());

    const int reportTemperatureUnit = data[QStringLiteral("Temperature Unit")].toInt();

    const QVariant windChill = data[QStringLiteral("Windchill")];
    if (isValidData(windChill)) {
        // Use temperature unit to convert windchill temperature
        // we only show degrees symbol not actual temperature unit
        const QString temp = convertTemperature(m_displayTemperatureUnit, windChill, reportTemperatureUnit, false, true);
        row[textId] = i18nc("windchill, unit", "Windchill: %1", temp);

        m_detailsModel << row;
    }

    const QString humidex = data[QStringLiteral("Humidex")].toString();
    if (isValidData(humidex)) {
        // TODO: this seems wrong, does the humidex have temperature as units?
        // Use temperature unit to convert humidex temperature
        // we only show degrees symbol not actual temperature unit
        QString temp = convertTemperature(m_displayTemperatureUnit, humidex, reportTemperatureUnit, false, true);
        row[textId] = i18nc("humidex, unit","Humidex: %1", temp);

        m_detailsModel << row;
    }

    const QVariant dewpoint = data[QStringLiteral("Dewpoint")];
    if (isValidData(dewpoint)) {
        QString temp = convertTemperature(m_displayTemperatureUnit, dewpoint, reportTemperatureUnit);
        row[textId] = i18nc("ground temperature, unit", "Dewpoint: %1", temp);

        m_detailsModel << row;
    }

    const QVariant pressure = data[QStringLiteral("Pressure")];
    if (isValidData(pressure)) {
        KUnitConversion::Value v(pressure.toDouble(),
                                 static_cast<KUnitConversion::UnitId>(data[QStringLiteral("Pressure Unit")].toInt()));
        v = v.convertTo(m_displayPressureUnit);
        row[textId] = i18nc("pressure, unit","Pressure: %1 %2",
                            locale.toString(clampValue(v.number(), 2), 'f', 2), v.unit().symbol());

        m_detailsModel << row;
    }

    const QString pressureTendency = data[QStringLiteral("Pressure Tendency")].toString();
    if (isValidData(pressureTendency)) {
        const QString i18nPressureTendency = i18nc("pressure tendency", pressureTendency.toUtf8().data());
        row[textId] = i18nc("pressure tendency, rising/falling/steady",
                            "Pressure Tendency: %1", i18nPressureTendency);

        m_detailsModel << row;
    }

    const QVariant visibility = data[QStringLiteral("Visibility")];
    if (isValidData(visibility)) {
        const KUnitConversion::UnitId unitId = static_cast<KUnitConversion::UnitId>(data[QStringLiteral("Visibility Unit")].toInt());
        if (unitId != KUnitConversion::NoUnit) {
            KUnitConversion::Value v(visibility.toDouble(), unitId);
            v = v.convertTo(m_displayVisibilityUnit);
            row[textId] = i18nc("distance, unit","Visibility: %1 %2",
                                locale.toString(clampValue(v.number(), 1), 'f', 1), v.unit().symbol());
        } else {
            row[textId] = i18nc("visibility from distance", "Visibility: %1", visibility.toString());
        }

        m_detailsModel << row;
    }

    const QVariant humidity = data[QStringLiteral("Humidity")];
    if (isValidData(humidity)) {
        row[textId] = i18nc("content of water in air", "Humidity: %1%2",
                            locale.toString(clampValue(humidity.toFloat(), 0), 'f', 0), i18nc("Percent, measure unit", "%"));

        m_detailsModel << row;
    }

    const QVariant windSpeed = data[QStringLiteral("Wind Speed")];
    if (isValidData(windSpeed)) {
        // TODO: missing check for windDirection validness
        const QString windDirection = data[QStringLiteral("Wind Direction")].toString();
        row[iconId] = windDirection;

        bool isNumeric;
        const double windSpeedNumeric = windSpeed.toDouble(&isNumeric);
        if (isNumeric) {
            if (windSpeedNumeric != 0) {
                KUnitConversion::Value v(windSpeedNumeric,
                                        static_cast<KUnitConversion::UnitId>(data[QStringLiteral("Wind Speed Unit")].toInt()));
                v = v.convertTo(m_displaySpeedUnit);
                const QString i18nWindDirection = i18nc("wind direction", windDirection.toUtf8().data());
                row[textId] = i18nc("wind direction, speed","%1 %2 %3", i18nWindDirection,
                                    locale.toString(clampValue(v.number(), 1), 'f', 1), v.unit().symbol());
            } else {
                row[textId] = i18nc("Wind condition", "Calm");
            }
        } else {
            row[textId] = windSpeed.toString();
        }

        m_detailsModel << row;
        row[iconId] = QString(); // reset
    }

    const QVariant windGust = data[QStringLiteral("Wind Gust")];
    if (isValidData(windGust)) {
        // Convert the wind format for nonstandard types
        KUnitConversion::Value v(windGust.toDouble(),
                                 static_cast<KUnitConversion::UnitId>(data[QStringLiteral("Wind Speed Unit")].toInt()));
        v = v.convertTo(m_displaySpeedUnit);
        row[textId] = i18nc("winds exceeding wind speed briefly", "Wind Gust: %1 %2",
                            locale.toString(clampValue(v.number(), 1), 'f', 1), v.unit().symbol());

        m_detailsModel << row;
    }
}

void WeatherApplet::updateNoticesModel(const Plasma::DataEngine::Data &data)
{
    m_noticesModel.clear();

    QVariantList warnings;
    const int warningsCount = data[QStringLiteral("Total Warnings Issued")].toInt();
    warnings.reserve(warningsCount);
    for (int i = 0; i < warningsCount; ++i) {
        warnings << QVariantMap {
            { NoticesKeys::description(), data[QStringLiteral("Warning Description %1").arg(i)] },
            { NoticesKeys::info(),        data[QStringLiteral("Warning Info %1").arg(i)] },
        };
    }
    m_noticesModel << QVariant(warnings);

    QVariantList watches;
    const int watchesCount = data[QStringLiteral("Total Watches Issued")].toInt();
    watches.reserve(watchesCount);
    for (int i = 0; i < watchesCount; ++i) {
        watches << QVariantMap {
            { NoticesKeys::description(), data[QStringLiteral("Watch Description %1").arg(i)] },
            { NoticesKeys::info(),        data[QStringLiteral("Watch Info %1").arg(i)] },
        };
    }
    m_noticesModel << QVariant(watches);
}

void WeatherApplet::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(source);

    if (data.isEmpty()) {
        return;
    }

    updatePanelModel(data);
    updateForecastModel(data);
    updateDetailsModel(data);
    updateNoticesModel(data);

    const QString creditUrl = data[QStringLiteral("Credit Url")].toString();
    QList<QUrl> associatedApplicationUrls;
    if (!creditUrl.isEmpty()) {
        associatedApplicationUrls << QUrl(creditUrl);
    }
    setAssociatedApplicationUrls(associatedApplicationUrls);

    m_busyTimer->stop();
    if (m_timeoutNotification) {
        m_timeoutNotification->close();
    }
    setBusy(false);

    emit modelUpdated();
}

QVariantMap WeatherApplet::configValues() const
{
    KConfigGroup cfg = this->config();

    return QVariantMap {
        // UI settings
        { AppletConfigKeys::services(),       cfg.readEntry(StorageConfigKeys::weatherServiceProviders, QStringList()) },
        { AppletConfigKeys::showTemperatureInCompactMode(), cfg.readEntry(StorageConfigKeys::showTemperatureInCompactMode, false) },

        // units
        { AppletConfigKeys::temperatureUnitId(), m_displayTemperatureUnit.id() },
        { AppletConfigKeys::windSpeedUnitId(),   m_displaySpeedUnit.id() },
        { AppletConfigKeys::pressureUnitId(),    m_displayPressureUnit.id() },
        { AppletConfigKeys::visibilityUnitId(),  m_displayVisibilityUnit.id() },

        // data source
        { AppletConfigKeys::updateInterval(), m_updateInterval },
        { AppletConfigKeys::source(),         m_source },
    };
}

void WeatherApplet::saveConfig(const QVariantMap& configChanges)
{
    // TODO: if just units where changed there is no need to reset the complete model or reconnect to engine
    resetPanelModel();
    m_forecastModel.clear();
    m_detailsModel.clear();

    emit modelUpdated();

    KConfigGroup cfg = config();

    // UI settings
    auto it = configChanges.find(AppletConfigKeys::services());
    if (it != configChanges.end()) {
        cfg.writeEntry(StorageConfigKeys::weatherServiceProviders, it.value().toStringList());
    }
    it = configChanges.find(AppletConfigKeys::showTemperatureInCompactMode());
    if (it != configChanges.end()) {
        cfg.writeEntry(StorageConfigKeys::showTemperatureInCompactMode, it.value().toBool());
    }

    // units
    it = configChanges.find(AppletConfigKeys::temperatureUnitId());
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

void WeatherApplet::connectToEngine()
{
    if (m_timeoutNotification) {
        QObject::disconnect(m_timeoutNotificationConnection);
        m_timeoutNotification = nullptr;
    }

    const bool missingLocation = m_source.isEmpty();

    if (missingLocation) {
        setBusy(false);
        m_busyTimer->stop();
        setConfigurationRequired(true);
    } else {
        setBusy(true);
        m_busyTimer->start();

        Plasma::DataEngine* weatherDataEngine = dataEngine(DataEngineIds::weather());
        weatherDataEngine->connectSource(m_source, this, m_updateInterval * 60 * 1000);
    }
}

void WeatherApplet::giveUpBeingBusy()
{
    setBusy(false);

    const QStringList sourceDetails = m_source.split(QLatin1Char( '|' ), QString::SkipEmptyParts);
    if (sourceDetails.size() < 3) {
        setConfigurationRequired(true);
    } else {
        m_timeoutNotification =
            KNotification::event(KNotification::Error, QString(), // TODO: some title?
                                 i18n("Weather information retrieval for %1 timed out.", sourceDetails.value(2)),
                                 QStringLiteral("dialog-error"));
        // seems global disconnect with wildcard does not cover lambdas, so remembering manually for disconnect
        m_timeoutNotificationConnection =
            connect(m_timeoutNotification, &KNotification::closed,
                    this, &WeatherApplet::onTimeoutNotificationClosed);
    }
}

void WeatherApplet::onTimeoutNotificationClosed()
{
    m_timeoutNotification = nullptr;
}

Unit WeatherApplet::unit(const QString& unit)
{
    if (!unit.isEmpty() && unit[0].isDigit()) {
        return m_converter.unit(static_cast<UnitId>(unit.toInt()));
    }
    // Support < 4.4 config values
    return m_converter.unit(unit);
}

K_EXPORT_PLASMA_APPLET_WITH_JSON(weather, WeatherApplet, "metadata.json")

#include "weatherapplet.moc"
