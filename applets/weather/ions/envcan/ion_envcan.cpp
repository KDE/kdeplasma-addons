/*
    SPDX-FileCopyrightText: 2007-2011, 2019 Shawn Starr <shawn.starr@rogers.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

/* Ion for Environment Canada XML data */

#include "ion_envcan.h"

#include "envcan_debug.h"

#include <KIO/TransferJob>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KUnitConversion/Converter>

#include <QRegularExpression>
#include <QTimeZone>
#include <QUrl>

K_PLUGIN_CLASS_WITH_JSON(EnvCanadaIon, "metadata.json")

using namespace Qt::StringLiterals;

WeatherData::WeatherData()
    : stationLatitude(qQNaN())
    , stationLongitude(qQNaN())
    , temperature(qQNaN())
    , dewpoint(qQNaN())
    , windchill(qQNaN())
    , pressure(qQNaN())
    , visibility(qQNaN())
    , humidity(qQNaN())
    , windSpeed(qQNaN())
    , windGust(qQNaN())
    , normalHigh(qQNaN())
    , normalLow(qQNaN())
    , prevHigh(qQNaN())
    , prevLow(qQNaN())
    , recordHigh(qQNaN())
    , recordLow(qQNaN())
    , recordRain(qQNaN())
    , recordSnow(qQNaN())
{
}

WeatherData::ForecastInfo::ForecastInfo()
    : tempHigh(qQNaN())
    , tempLow(qQNaN())
    , popPrecent(qQNaN())
{
}

// ctor, dtor
EnvCanadaIon::EnvCanadaIon(QObject *parent, const QVariantList &args)
    : Ion(parent)
{
    Q_UNUSED(args);
}

EnvCanadaIon::~EnvCanadaIon()
{
}

QMap<QString, Ion::ConditionIcons> EnvCanadaIon::setupConditionIconMappings() const
{
    return QMap<QString, ConditionIcons>{

        // Explicit periods
        {QStringLiteral("mainly sunny"), FewCloudsDay},
        {QStringLiteral("mainly clear"), FewCloudsNight},
        {QStringLiteral("sunny"), ClearDay},
        {QStringLiteral("clear"), ClearNight},

        // Available conditions
        {QStringLiteral("blowing snow"), Snow},
        {QStringLiteral("cloudy"), Overcast},
        {QStringLiteral("distant precipitation"), LightRain},
        {QStringLiteral("drifting snow"), Flurries},
        {QStringLiteral("drizzle"), LightRain},
        {QStringLiteral("dust"), NotAvailable},
        {QStringLiteral("dust devils"), NotAvailable},
        {QStringLiteral("fog"), Mist},
        {QStringLiteral("fog bank near station"), Mist},
        {QStringLiteral("fog depositing ice"), Mist},
        {QStringLiteral("fog patches"), Mist},
        {QStringLiteral("freezing drizzle"), FreezingDrizzle},
        {QStringLiteral("freezing rain"), FreezingRain},
        {QStringLiteral("funnel cloud"), NotAvailable},
        {QStringLiteral("hail"), Hail},
        {QStringLiteral("haze"), Haze},
        {QStringLiteral("heavy blowing snow"), Snow},
        {QStringLiteral("heavy drifting snow"), Snow},
        {QStringLiteral("heavy drizzle"), LightRain},
        {QStringLiteral("heavy hail"), Hail},
        {QStringLiteral("heavy mixed rain and drizzle"), LightRain},
        {QStringLiteral("heavy mixed rain and snow shower"), RainSnow},
        {QStringLiteral("heavy rain"), Rain},
        {QStringLiteral("heavy rain and snow"), RainSnow},
        {QStringLiteral("heavy rainshower"), Rain},
        {QStringLiteral("heavy snow"), Snow},
        {QStringLiteral("heavy snow pellets"), Snow},
        {QStringLiteral("heavy snowshower"), Snow},
        {QStringLiteral("heavy thunderstorm with hail"), Thunderstorm},
        {QStringLiteral("heavy thunderstorm with rain"), Thunderstorm},
        {QStringLiteral("ice crystals"), Flurries},
        {QStringLiteral("ice pellets"), Hail},
        {QStringLiteral("increasing cloud"), Overcast},
        {QStringLiteral("light drizzle"), LightRain},
        {QStringLiteral("light freezing drizzle"), FreezingRain},
        {QStringLiteral("light freezing rain"), FreezingRain},
        {QStringLiteral("light rain"), LightRain},
        {QStringLiteral("light rainshower"), LightRain},
        {QStringLiteral("light snow"), LightSnow},
        {QStringLiteral("light snow pellets"), LightSnow},
        {QStringLiteral("light snowshower"), Flurries},
        {QStringLiteral("lightning visible"), Thunderstorm},
        {QStringLiteral("mist"), Mist},
        {QStringLiteral("mixed rain and drizzle"), LightRain},
        {QStringLiteral("mixed rain and snow shower"), RainSnow},
        {QStringLiteral("not reported"), NotAvailable},
        {QStringLiteral("rain"), Rain},
        {QStringLiteral("rain and snow"), RainSnow},
        {QStringLiteral("rainshower"), LightRain},
        {QStringLiteral("recent drizzle"), LightRain},
        {QStringLiteral("recent dust or sand storm"), NotAvailable},
        {QStringLiteral("recent fog"), Mist},
        {QStringLiteral("recent freezing precipitation"), FreezingDrizzle},
        {QStringLiteral("recent hail"), Hail},
        {QStringLiteral("recent rain"), Rain},
        {QStringLiteral("recent rain and snow"), RainSnow},
        {QStringLiteral("recent rainshower"), Rain},
        {QStringLiteral("recent snow"), Snow},
        {QStringLiteral("recent snowshower"), Flurries},
        {QStringLiteral("recent thunderstorm"), Thunderstorm},
        {QStringLiteral("recent thunderstorm with hail"), Thunderstorm},
        {QStringLiteral("recent thunderstorm with heavy hail"), Thunderstorm},
        {QStringLiteral("recent thunderstorm with heavy rain"), Thunderstorm},
        {QStringLiteral("recent thunderstorm with rain"), Thunderstorm},
        {QStringLiteral("sand or dust storm"), NotAvailable},
        {QStringLiteral("severe sand or dust storm"), NotAvailable},
        {QStringLiteral("shallow fog"), Mist},
        {QStringLiteral("smoke"), NotAvailable},
        {QStringLiteral("snow"), Snow},
        {QStringLiteral("snow crystals"), Flurries},
        {QStringLiteral("snow grains"), Flurries},
        {QStringLiteral("squalls"), Snow},
        {QStringLiteral("thunderstorm"), Thunderstorm},
        {QStringLiteral("thunderstorm with hail"), Thunderstorm},
        {QStringLiteral("thunderstorm with rain"), Thunderstorm},
        {QStringLiteral("thunderstorm with light rainshowers"), Thunderstorm},
        {QStringLiteral("thunderstorm with heavy rainshowers"), Thunderstorm},
        {QStringLiteral("thunderstorm with sand or dust storm"), Thunderstorm},
        {QStringLiteral("thunderstorm without precipitation"), Thunderstorm},
        {QStringLiteral("tornado"), NotAvailable},
    };
}

QMap<QString, Ion::ConditionIcons> EnvCanadaIon::setupForecastIconMappings() const
{
    return QMap<QString, ConditionIcons>{

        // Abbreviated forecast descriptions
        {QStringLiteral("a few flurries"), Flurries},
        {QStringLiteral("a few flurries mixed with ice pellets"), RainSnow},
        {QStringLiteral("a few flurries or rain showers"), RainSnow},
        {QStringLiteral("a few flurries or thundershowers"), RainSnow},
        {QStringLiteral("a few rain showers or flurries"), RainSnow},
        {QStringLiteral("a few rain showers or wet flurries"), RainSnow},
        {QStringLiteral("a few showers"), LightRain},
        {QStringLiteral("a few showers or drizzle"), LightRain},
        {QStringLiteral("a few showers or thundershowers"), Thunderstorm},
        {QStringLiteral("a few showers or thunderstorms"), Thunderstorm},
        {QStringLiteral("a few thundershowers"), Thunderstorm},
        {QStringLiteral("a few thunderstorms"), Thunderstorm},
        {QStringLiteral("a few wet flurries"), RainSnow},
        {QStringLiteral("a few wet flurries or rain showers"), RainSnow},
        {QStringLiteral("a mix of sun and cloud"), PartlyCloudyDay},
        {QStringLiteral("cloudy with sunny periods"), PartlyCloudyDay},
        {QStringLiteral("partly cloudy"), PartlyCloudyDay},
        {QStringLiteral("mainly cloudy"), PartlyCloudyDay},
        {QStringLiteral("mainly sunny"), FewCloudsDay},
        {QStringLiteral("sunny"), ClearDay},
        {QStringLiteral("blizzard"), Snow},
        {QStringLiteral("clear"), ClearNight},
        {QStringLiteral("cloudy"), Overcast},
        {QStringLiteral("drizzle"), LightRain},
        {QStringLiteral("drizzle mixed with freezing drizzle"), FreezingDrizzle},
        {QStringLiteral("drizzle mixed with rain"), LightRain},
        {QStringLiteral("drizzle or freezing drizzle"), LightRain},
        {QStringLiteral("drizzle or rain"), LightRain},
        {QStringLiteral("flurries"), Flurries},
        {QStringLiteral("flurries at times heavy"), Flurries},
        {QStringLiteral("flurries at times heavy or rain snowers"), RainSnow},
        {QStringLiteral("flurries mixed with ice pellets"), FreezingRain},
        {QStringLiteral("flurries or ice pellets"), FreezingRain},
        {QStringLiteral("flurries or rain showers"), RainSnow},
        {QStringLiteral("flurries or thundershowers"), Flurries},
        {QStringLiteral("fog"), Mist},
        {QStringLiteral("fog developing"), Mist},
        {QStringLiteral("fog dissipating"), Mist},
        {QStringLiteral("fog patches"), Mist},
        {QStringLiteral("freezing drizzle"), FreezingDrizzle},
        {QStringLiteral("freezing rain"), FreezingRain},
        {QStringLiteral("freezing rain mixed with rain"), FreezingRain},
        {QStringLiteral("freezing rain mixed with snow"), FreezingRain},
        {QStringLiteral("freezing rain or ice pellets"), FreezingRain},
        {QStringLiteral("freezing rain or rain"), FreezingRain},
        {QStringLiteral("freezing rain or snow"), FreezingRain},
        {QStringLiteral("ice fog"), Mist},
        {QStringLiteral("ice fog developing"), Mist},
        {QStringLiteral("ice fog dissipating"), Mist},
        {QStringLiteral("ice pellets"), Hail},
        {QStringLiteral("ice pellets mixed with freezing rain"), Hail},
        {QStringLiteral("ice pellets mixed with snow"), Hail},
        {QStringLiteral("ice pellets or snow"), RainSnow},
        {QStringLiteral("light snow"), LightSnow},
        {QStringLiteral("light snow and blizzard"), LightSnow},
        {QStringLiteral("light snow and blizzard and blowing snow"), Snow},
        {QStringLiteral("light snow and blowing snow"), LightSnow},
        {QStringLiteral("light snow mixed with freezing drizzle"), FreezingDrizzle},
        {QStringLiteral("light snow mixed with freezing rain"), FreezingRain},
        {QStringLiteral("light snow or ice pellets"), LightSnow},
        {QStringLiteral("light snow or rain"), RainSnow},
        {QStringLiteral("light wet snow"), RainSnow},
        {QStringLiteral("light wet snow or rain"), RainSnow},
        {QStringLiteral("local snow squalls"), Snow},
        {QStringLiteral("near blizzard"), Snow},
        {QStringLiteral("overcast"), Overcast},
        {QStringLiteral("increasing cloudiness"), Overcast},
        {QStringLiteral("increasing clouds"), Overcast},
        {QStringLiteral("periods of drizzle"), LightRain},
        {QStringLiteral("periods of drizzle mixed with freezing drizzle"), FreezingDrizzle},
        {QStringLiteral("periods of drizzle mixed with rain"), LightRain},
        {QStringLiteral("periods of drizzle or freezing drizzle"), FreezingDrizzle},
        {QStringLiteral("periods of drizzle or rain"), LightRain},
        {QStringLiteral("periods of freezing drizzle"), FreezingDrizzle},
        {QStringLiteral("periods of freezing drizzle or drizzle"), FreezingDrizzle},
        {QStringLiteral("periods of freezing drizzle or rain"), FreezingDrizzle},
        {QStringLiteral("periods of freezing rain"), FreezingRain},
        {QStringLiteral("periods of freezing rain mixed with ice pellets"), FreezingRain},
        {QStringLiteral("periods of freezing rain mixed with rain"), FreezingRain},
        {QStringLiteral("periods of freezing rain mixed with snow"), FreezingRain},
        {QStringLiteral("periods of freezing rain mixed with freezing drizzle"), FreezingRain},
        {QStringLiteral("periods of freezing rain or ice pellets"), FreezingRain},
        {QStringLiteral("periods of freezing rain or rain"), FreezingRain},
        {QStringLiteral("periods of freezing rain or snow"), FreezingRain},
        {QStringLiteral("periods of ice pellets"), Hail},
        {QStringLiteral("periods of ice pellets mixed with freezing rain"), Hail},
        {QStringLiteral("periods of ice pellets mixed with snow"), Hail},
        {QStringLiteral("periods of ice pellets or freezing rain"), Hail},
        {QStringLiteral("periods of ice pellets or snow"), Hail},
        {QStringLiteral("periods of light snow"), LightSnow},
        {QStringLiteral("periods of light snow and blizzard"), Snow},
        {QStringLiteral("periods of light snow and blizzard and blowing snow"), Snow},
        {QStringLiteral("periods of light snow and blowing snow"), LightSnow},
        {QStringLiteral("periods of light snow mixed with freezing drizzle"), RainSnow},
        {QStringLiteral("periods of light snow mixed with freezing rain"), RainSnow},
        {QStringLiteral("periods of light snow mixed with ice pellets"), LightSnow},
        {QStringLiteral("periods of light snow mixed with rain"), RainSnow},
        {QStringLiteral("periods of light snow or freezing drizzle"), RainSnow},
        {QStringLiteral("periods of light snow or freezing rain"), RainSnow},
        {QStringLiteral("periods of light snow or ice pellets"), LightSnow},
        {QStringLiteral("periods of light snow or rain"), RainSnow},
        {QStringLiteral("periods of light wet snow"), LightSnow},
        {QStringLiteral("periods of light wet snow mixed with rain"), RainSnow},
        {QStringLiteral("periods of light wet snow or rain"), RainSnow},
        {QStringLiteral("periods of rain"), Rain},
        {QStringLiteral("periods of rain mixed with freezing rain"), Rain},
        {QStringLiteral("periods of rain mixed with snow"), RainSnow},
        {QStringLiteral("periods of rain or drizzle"), Rain},
        {QStringLiteral("periods of rain or freezing rain"), Rain},
        {QStringLiteral("periods of rain or thundershowers"), Showers},
        {QStringLiteral("periods of rain or thunderstorms"), Thunderstorm},
        {QStringLiteral("periods of rain or snow"), RainSnow},
        {QStringLiteral("periods of snow"), Snow},
        {QStringLiteral("periods of snow and blizzard"), Snow},
        {QStringLiteral("periods of snow and blizzard and blowing snow"), Snow},
        {QStringLiteral("periods of snow and blowing snow"), Snow},
        {QStringLiteral("periods of snow mixed with freezing drizzle"), RainSnow},
        {QStringLiteral("periods of snow mixed with freezing rain"), RainSnow},
        {QStringLiteral("periods of snow mixed with ice pellets"), Snow},
        {QStringLiteral("periods of snow mixed with rain"), RainSnow},
        {QStringLiteral("periods of snow or freezing drizzle"), RainSnow},
        {QStringLiteral("periods of snow or freezing rain"), RainSnow},
        {QStringLiteral("periods of snow or ice pellets"), Snow},
        {QStringLiteral("periods of snow or rain"), RainSnow},
        {QStringLiteral("periods of rain or snow"), RainSnow},
        {QStringLiteral("periods of wet snow"), Snow},
        {QStringLiteral("periods of wet snow mixed with rain"), RainSnow},
        {QStringLiteral("periods of wet snow or rain"), RainSnow},
        {QStringLiteral("rain"), Rain},
        {QStringLiteral("rain at times heavy"), Rain},
        {QStringLiteral("rain at times heavy mixed with freezing rain"), FreezingRain},
        {QStringLiteral("rain at times heavy mixed with snow"), RainSnow},
        {QStringLiteral("rain at times heavy or drizzle"), Rain},
        {QStringLiteral("rain at times heavy or freezing rain"), Rain},
        {QStringLiteral("rain at times heavy or snow"), RainSnow},
        {QStringLiteral("rain at times heavy or thundershowers"), Showers},
        {QStringLiteral("rain at times heavy or thunderstorms"), Thunderstorm},
        {QStringLiteral("rain mixed with freezing rain"), FreezingRain},
        {QStringLiteral("rain mixed with snow"), RainSnow},
        {QStringLiteral("rain or drizzle"), Rain},
        {QStringLiteral("rain or freezing rain"), Rain},
        {QStringLiteral("rain or snow"), RainSnow},
        {QStringLiteral("rain or thundershowers"), Showers},
        {QStringLiteral("rain or thunderstorms"), Thunderstorm},
        {QStringLiteral("rain showers or flurries"), RainSnow},
        {QStringLiteral("rain showers or wet flurries"), RainSnow},
        {QStringLiteral("showers"), Showers},
        {QStringLiteral("showers at times heavy"), Showers},
        {QStringLiteral("showers at times heavy or thundershowers"), Showers},
        {QStringLiteral("showers at times heavy or thunderstorms"), Thunderstorm},
        {QStringLiteral("showers or drizzle"), Showers},
        {QStringLiteral("showers or thundershowers"), Thunderstorm},
        {QStringLiteral("showers or thunderstorms"), Thunderstorm},
        {QStringLiteral("smoke"), NotAvailable},
        {QStringLiteral("snow"), Snow},
        {QStringLiteral("snow and blizzard"), Snow},
        {QStringLiteral("snow and blizzard and blowing snow"), Snow},
        {QStringLiteral("snow and blowing snow"), Snow},
        {QStringLiteral("snow at times heavy"), Snow},
        {QStringLiteral("snow at times heavy and blizzard"), Snow},
        {QStringLiteral("snow at times heavy and blowing snow"), Snow},
        {QStringLiteral("snow at times heavy mixed with freezing drizzle"), RainSnow},
        {QStringLiteral("snow at times heavy mixed with freezing rain"), RainSnow},
        {QStringLiteral("snow at times heavy mixed with ice pellets"), Snow},
        {QStringLiteral("snow at times heavy mixed with rain"), RainSnow},
        {QStringLiteral("snow at times heavy or freezing rain"), RainSnow},
        {QStringLiteral("snow at times heavy or ice pellets"), Snow},
        {QStringLiteral("snow at times heavy or rain"), RainSnow},
        {QStringLiteral("snow mixed with freezing drizzle"), RainSnow},
        {QStringLiteral("snow mixed with freezing rain"), RainSnow},
        {QStringLiteral("snow mixed with ice pellets"), Snow},
        {QStringLiteral("snow mixed with rain"), RainSnow},
        {QStringLiteral("snow or freezing drizzle"), RainSnow},
        {QStringLiteral("snow or freezing rain"), RainSnow},
        {QStringLiteral("snow or ice pellets"), Snow},
        {QStringLiteral("snow or rain"), RainSnow},
        {QStringLiteral("snow squalls"), Snow},
        {QStringLiteral("sunny"), ClearDay},
        {QStringLiteral("sunny with cloudy periods"), PartlyCloudyDay},
        {QStringLiteral("thunderstorms"), Thunderstorm},
        {QStringLiteral("thunderstorms and possible hail"), Thunderstorm},
        {QStringLiteral("wet flurries"), Flurries},
        {QStringLiteral("wet flurries at times heavy"), Flurries},
        {QStringLiteral("wet flurries at times heavy or rain snowers"), RainSnow},
        {QStringLiteral("wet flurries or rain showers"), RainSnow},
        {QStringLiteral("wet snow"), Snow},
        {QStringLiteral("wet snow at times heavy"), Snow},
        {QStringLiteral("wet snow at times heavy mixed with rain"), RainSnow},
        {QStringLiteral("wet snow mixed with rain"), RainSnow},
        {QStringLiteral("wet snow or rain"), RainSnow},
        {QStringLiteral("windy"), NotAvailable},

        {QStringLiteral("chance of drizzle mixed with freezing drizzle"), LightRain},
        {QStringLiteral("chance of flurries mixed with ice pellets"), Flurries},
        {QStringLiteral("chance of flurries or ice pellets"), Flurries},
        {QStringLiteral("chance of flurries or rain showers"), RainSnow},
        {QStringLiteral("chance of flurries or thundershowers"), RainSnow},
        {QStringLiteral("chance of freezing drizzle"), FreezingDrizzle},
        {QStringLiteral("chance of freezing rain"), FreezingRain},
        {QStringLiteral("chance of freezing rain mixed with snow"), RainSnow},
        {QStringLiteral("chance of freezing rain or rain"), FreezingRain},
        {QStringLiteral("chance of freezing rain or snow"), RainSnow},
        {QStringLiteral("chance of light snow and blowing snow"), LightSnow},
        {QStringLiteral("chance of light snow mixed with freezing drizzle"), LightSnow},
        {QStringLiteral("chance of light snow mixed with ice pellets"), LightSnow},
        {QStringLiteral("chance of light snow mixed with rain"), RainSnow},
        {QStringLiteral("chance of light snow or freezing rain"), RainSnow},
        {QStringLiteral("chance of light snow or ice pellets"), LightSnow},
        {QStringLiteral("chance of light snow or rain"), RainSnow},
        {QStringLiteral("chance of light wet snow"), Snow},
        {QStringLiteral("chance of rain"), Rain},
        {QStringLiteral("chance of rain at times heavy"), Rain},
        {QStringLiteral("chance of rain mixed with snow"), RainSnow},
        {QStringLiteral("chance of rain or drizzle"), Rain},
        {QStringLiteral("chance of rain or freezing rain"), Rain},
        {QStringLiteral("chance of rain or snow"), RainSnow},
        {QStringLiteral("chance of rain showers or flurries"), RainSnow},
        {QStringLiteral("chance of rain showers or wet flurries"), RainSnow},
        {QStringLiteral("chance of severe thunderstorms"), Thunderstorm},
        {QStringLiteral("chance of showers at times heavy"), Rain},
        {QStringLiteral("chance of showers at times heavy or thundershowers"), Thunderstorm},
        {QStringLiteral("chance of showers at times heavy or thunderstorms"), Thunderstorm},
        {QStringLiteral("chance of showers or thundershowers"), Thunderstorm},
        {QStringLiteral("chance of showers or thunderstorms"), Thunderstorm},
        {QStringLiteral("chance of snow"), Snow},
        {QStringLiteral("chance of snow and blizzard"), Snow},
        {QStringLiteral("chance of snow mixed with freezing drizzle"), Snow},
        {QStringLiteral("chance of snow mixed with freezing rain"), RainSnow},
        {QStringLiteral("chance of snow mixed with rain"), RainSnow},
        {QStringLiteral("chance of snow or rain"), RainSnow},
        {QStringLiteral("chance of snow squalls"), Snow},
        {QStringLiteral("chance of thundershowers"), Showers},
        {QStringLiteral("chance of thunderstorms"), Thunderstorm},
        {QStringLiteral("chance of thunderstorms and possible hail"), Thunderstorm},
        {QStringLiteral("chance of wet flurries"), Flurries},
        {QStringLiteral("chance of wet flurries at times heavy"), Flurries},
        {QStringLiteral("chance of wet flurries or rain showers"), RainSnow},
        {QStringLiteral("chance of wet snow"), Snow},
        {QStringLiteral("chance of wet snow mixed with rain"), RainSnow},
        {QStringLiteral("chance of wet snow or rain"), RainSnow},
    };
}

QMap<QString, Ion::ConditionIcons> const &EnvCanadaIon::conditionIcons() const
{
    static QMap<QString, ConditionIcons> const condval = setupConditionIconMappings();
    return condval;
}

QMap<QString, Ion::ConditionIcons> const &EnvCanadaIon::forecastIcons() const
{
    static QMap<QString, ConditionIcons> const foreval = setupForecastIconMappings();
    return foreval;
}

void EnvCanadaIon::clearForecastData()
{
    qCDebug(WEATHER::ION::ENVCAN) << " clear forecast data";
    m_territoryName.clear();
    m_cityCode.clear();
    m_forecastPromise.reset();
    m_xmlForecast.reset();
    m_weatherData.reset();
}

void EnvCanadaIon::clearLocationData()
{
    qCDebug(WEATHER::ION::ENVCAN) << " clear location data";
    m_xmlSetup.reset();
    m_searchString.clear();
    m_locationPromise.reset();
}

void EnvCanadaIon::validate(const QString &source)
{
    if (m_locationPromise->isCanceled()) {
        clearLocationData();
        return;
    }

    auto locations = std::make_shared<Locations>();

    qCDebug(WEATHER::ION::ENVCAN) << "start adding locations";

    QString sourceNormalized = source.toUpper();
    for (auto it = m_places.constBegin(); it != m_places.constEnd(); ++it) {
        if (it.key().toUpper().contains(sourceNormalized)) {
            Location location;
            location.setDisplayName(it.key());
            location.setStation(it->territoryName);
            location.setCode(it->cityCode);
            location.setPlaceInfo(it->territoryName + u"|"_s + it->cityCode);
            locations->addLocation(location);
        }
    }

    qCDebug(WEATHER::ION::ENVCAN) << " validated locations. Total locations:" << locations->rowCount() << " .Return";

    m_locationPromise->addResult(locations);
}

QUrl EnvCanadaIon::providerURL() const
{
    return QUrl(QStringLiteral("https://dd.weather.gc.ca"));
}

void EnvCanadaIon::findPlaces(std::shared_ptr<QPromise<std::shared_ptr<Locations>>> promise, const QString &searchString)
{
    m_locationPromise = promise;

    if (m_locationPromise->isCanceled()) {
        m_forecastPromise->finish();
        clearLocationData();
        return;
    }

    if (m_places.isEmpty()) {
        m_searchString = searchString;
        // If network is down, we need to spin and wait
        const QUrl url(QStringLiteral("https://dd.weather.gc.ca/today/citypage_weather/siteList.xml"));
        qCDebug(WEATHER::ION::ENVCAN) << "Fetching station list:" << url;

        KIO::TransferJob *getJob = KIO::get(url, KIO::NoReload, KIO::HideProgressInfo);

        m_xmlSetup = std::make_shared<QXmlStreamReader>();
        connect(getJob, &KIO::TransferJob::data, this, &EnvCanadaIon::places_slotDataArrived);
        connect(getJob, &KJob::result, this, &EnvCanadaIon::places_slotJobFinished);
    } else {
        validate(searchString);
        m_locationPromise->finish();
        clearLocationData();
    }
}

void EnvCanadaIon::fetchForecast(std::shared_ptr<QPromise<std::shared_ptr<Forecast>>> promise, const QString &placeInfo)
{
    m_forecastPromise = promise;

    if (m_forecastPromise->isCanceled()) {
        m_forecastPromise->finish();
        m_forecastPromise.reset();
        return;
    }

    // place_name|id - Triggers receiving weather of place
    const QList<QString> info = placeInfo.split('|'_L1);

    qCDebug(WEATHER::ION::ENVCAN) << "Start receiving forecast for info " << placeInfo;

    // Support the old format for receiving a forecast
    if (info.size() == 3 && info[1] == "weather"_L1 && !m_isLegacy) {
        m_isLegacy = true;
        if (m_places.isEmpty()) {
            // If network is down, we need to spin and wait
            const QUrl url(QStringLiteral("https://dd.weather.gc.ca/today/citypage_weather/siteList.xml"));
            qCDebug(WEATHER::ION::ENVCAN) << "Fetching station list:" << url;

            KIO::TransferJob *getJob = KIO::get(url, KIO::NoReload, KIO::HideProgressInfo);

            m_xmlSetup = std::make_shared<QXmlStreamReader>();
            connect(getJob, &KIO::TransferJob::data, this, &EnvCanadaIon::places_slotDataArrived);
            connect(getJob, &KJob::result, this, &EnvCanadaIon::places_slotJobFinished);
            m_legacyPlaceInfo = info[2];
            return;
        }
        if (auto it = m_places.constFind(placeInfo); it != m_places.constEnd()) {
            fetchForecast(m_forecastPromise, it->territoryName + u"|"_s + it->cityCode);
            return;
        }
        m_isLegacy = false;
        m_forecastPromise->finish();
        m_forecastPromise.reset();
        return;
    }

    if (info.count() > 2) {
        m_forecastPromise->finish();
        m_forecastPromise.reset();
        return;
    }

    m_territoryName = info.first();
    m_cityCode = info.last();

    if (m_territoryName.isEmpty() && m_cityCode.isEmpty()) {
        m_forecastPromise->finish();
        clearForecastData();
        return;
    }

    m_xmlForecast = std::make_shared<QXmlStreamReader>();
    m_weatherData = std::make_shared<WeatherData>();

    m_weatherData->shortTerritoryName = m_territoryName;

    getWeatherData();
}

void EnvCanadaIon::places_slotDataArrived(KIO::Job *job, const QByteArray &data)
{
    Q_UNUSED(job)

    if (data.isEmpty()) {
        return;
    }

    if (!m_isLegacy && m_locationPromise->isCanceled()) {
        return;
    }

    // Send to xml.
    m_xmlSetup->addData(data);
}

void EnvCanadaIon::places_slotJobFinished(KJob *job)
{
    Q_UNUSED(job)

    qCDebug(WEATHER::ION::ENVCAN) << "Location job finished";

    if (m_isLegacy) {
        readXMLSetup();
        if (auto it = m_places.constFind(m_legacyPlaceInfo); it != m_places.constEnd()) {
            qCDebug(WEATHER::ION::ENVCAN) << "Start fetching default forecast";
            fetchForecast(m_forecastPromise, it->territoryName + u"|"_s + it->cityCode);
            return;
        }
        m_isLegacy = false;
        m_forecastPromise->finish();
        clearForecastData();
        return;
    }

    if (m_locationPromise->isCanceled()) {
        m_locationPromise->finish();
        clearLocationData();
        return;
    }

    readXMLSetup();

    m_locationPromise->finish();
    clearLocationData();
}

// The weather URL has a dynamic name and path depending on its timestamp:
// https://dd.weather.gc.ca/today/citypage_weather/{PROV}/{HH}/{YYYYMMDD}T{HHmmss.sss}Z_MSC_CitypageWeather_{SiteCode}_en.xml
// This method is called iteratively 3 times to get the URL and then the weather report
void EnvCanadaIon::getWeatherData()
{
    WeatherData::UrlInfo &info = m_weatherData->urlInfo;

    info.requests++;
    if (info.requests > 3) {
        qCWarning(WEATHER::ION::ENVCAN) << "Too many requests to find the weather URL";
        return;
    }

    // We get the place info from the stations list
    if (info.cityCode.isEmpty()) {
        info.province = m_territoryName;
        info.cityCode = m_cityCode;
    }

    // 1. Base URL, on the territory dir, to get the list of hours
    QString url = u"https://dd.weather.gc.ca/today/citypage_weather/%1/"_s.arg(info.province);
    // 2. When we know the hour folder, we check for the weather report files
    if (!info.hours.isEmpty()) {
        url += info.hours.at(info.hourIndex) + u"/"_s;
    }
    // 3. Now we have the full information to compose the URL
    if (!info.fileName.isEmpty()) {
        url += info.fileName;
    }

    qCDebug(WEATHER::ION::ENVCAN) << "Fetching weather URL:" << url;

    KIO::TransferJob *getJob = KIO::get(QUrl(url), KIO::Reload, KIO::HideProgressInfo);

    m_xmlForecast = std::make_shared<QXmlStreamReader>();

    connect(getJob, &KIO::TransferJob::data, this, &EnvCanadaIon::forecast_slotDataArrived);
    connect(getJob, &KJob::result, this, &EnvCanadaIon::forecast_slotJobFinished);
}

void EnvCanadaIon::forecast_slotDataArrived(KIO::Job *job, const QByteArray &data)
{
    Q_UNUSED(job);

    if (m_forecastPromise->isCanceled()) {
        return;
    }

    if (data.isEmpty()) {
        return;
    }

    // Remove the HTML doctype line from XML parsing
    if (data.startsWith("<!DOCTYPE"_ba)) {
        int newLinePos = data.indexOf('\n');
        m_xmlForecast->addData(QByteArrayView(data).slice(newLinePos + 1));
        return;
    }

    m_xmlForecast->addData(data);
}

void EnvCanadaIon::forecast_slotJobFinished(KJob *job)
{
    if (m_forecastPromise->isCanceled()) {
        m_forecastPromise->finish();
        clearForecastData();
        return;
    }

    if (job->error()) {
        m_forecastPromise->finish();
        clearForecastData();
        return;
    }

    readXMLData(*m_xmlForecast);
}

// Parse the city list and store into a QMap
void EnvCanadaIon::readXMLSetup()
{
    QString territory;
    QString code;
    QString cityName;

    while (!m_xmlSetup->atEnd()) {
        m_xmlSetup->readNext();

        const auto elementName = m_xmlSetup->name();

        if (m_xmlSetup->isStartElement()) {
            // XML ID code to match filename
            if (elementName == QLatin1String("site")) {
                code = m_xmlSetup->attributes().value(u"code"_s).toString();
            }

            if (elementName == QLatin1String("nameEn")) {
                cityName = m_xmlSetup->readElementText(); // Name of cities
            }

            if (elementName == QLatin1String("provinceCode")) {
                territory = m_xmlSetup->readElementText(); // Provinces/Territory list
            }
        }

        if (m_xmlSetup->isEndElement() && elementName == QLatin1String("site")) {
            XMLMapInfo info;
            QString tmp = cityName + u", "_s + territory; // Build the key name.

            // Set the mappings
            info.cityCode = code;
            info.territoryName = territory;
            info.cityName = cityName;

            // Set the string list, we will use for the applet to display the available cities.
            m_places[tmp] = info;
        }
    }

    // Update and return locations if no error
    if (m_xmlSetup->hasError() && !m_isLegacy) {
        validate(m_searchString);
    }
}

void EnvCanadaIon::parseWeatherSite(WeatherData &data, QXmlStreamReader &xml)
{
    while (!xml.atEnd()) {
        xml.readNext();

        const auto elementName = xml.name();

        if (xml.isStartElement()) {
            if (elementName == QLatin1String("license")) {
                data.creditUrl = xml.readElementText();
            } else if (elementName == QLatin1String("location")) {
                parseLocations(data, xml);
            } else if (elementName == QLatin1String("warnings")) {
                // Cleanup warning list on update
                data.warnings.clear();
                parseWarnings(data, xml);
            } else if (elementName == QLatin1String("currentConditions")) {
                parseConditions(data, xml);
            } else if (elementName == QLatin1String("forecastGroup")) {
                // Clean up forecast list on update
                data.forecasts.clear();
                parseWeatherForecast(data, xml);
            } else if (elementName == QLatin1String("yesterdayConditions")) {
                parseYesterdayWeather(data, xml);
            } else if (elementName == QLatin1String("riseSet")) {
                parseAstronomicals(data, xml);
            } else if (elementName == QLatin1String("almanac")) {
                parseWeatherRecords(data, xml);
            } else {
                parseUnknownElement(xml);
            }
        }
    }
}

// Parse Weather data main loop, from here we have to descend into each tag pair
void EnvCanadaIon::readXMLData(QXmlStreamReader &xml)
{
    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement()) {
            break;
        }

        if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("siteData")) {
                parseWeatherSite(*m_weatherData, xml);
            } else if (xml.name() == QLatin1String("html")) {
                if (!xml.hasError()) {
                    parseDirListing(m_weatherData->urlInfo, xml);
                    getWeatherData();
                    return;
                }
            } else {
                parseUnknownElement(xml);
            }
        }
    }

    // update and return forecast if no error
    if (!xml.hasError()) {
        updateWeather();
    }

    m_forecastPromise->finish();
    clearForecastData();
}

void EnvCanadaIon::parseDirListing(WeatherData::UrlInfo &info, QXmlStreamReader &xml)
{
    const bool expectingFileNames = !info.hours.isEmpty();

    while (!xml.atEnd()) {
        xml.readNext();

        // We are parsing a directory listing with files or folders as hyperlinks
        if (xml.isStartElement() && xml.name() == "a"_L1) {
            QString item = xml.attributes().value(u"href"_s).toString().trimmed();

            // Check for hour folders
            if (!expectingFileNames && item.endsWith('/'_L1)) {
                item.slice(0, item.length() - 1);

                bool isHour = false;
                item.toInt(&isHour);
                if (isHour) {
                    info.hours.prepend(item);
                }
                continue;
            }

            // Check just for files that match our city code en English language
            if (item.endsWith(u"%1_en.xml"_s.arg(info.cityCode))) {
                info.fileName = item;
            }
        }
    }

    // Sort hours in reverse order (more recent first)
    if (!expectingFileNames && !info.hours.isEmpty()) {
        std::sort(info.hours.begin(), info.hours.end(), [](const auto &a, const auto &b) {
            return a.toInt() > b.toInt();
        });
    }

    // If we didn't find the filename in the current hour folder
    // set up a new requests to search it on the previous one
    if (expectingFileNames && info.fileName.isEmpty()) {
        if (info.hourIndex < info.hours.count()) {
            info.hourIndex++;
            info.requests--;
        }
    }
}

void EnvCanadaIon::parseFloat(float &value, QXmlStreamReader &xml)
{
    bool ok = false;
    const float result = xml.readElementText().toFloat(&ok);
    if (ok) {
        value = result;
    }
}

void EnvCanadaIon::parseDateTime(WeatherData &data, QXmlStreamReader &xml, std::shared_ptr<WeatherData::WeatherEvent> event)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("dateTime"));

    // What kind of date info is this?
    const QString dateType = xml.attributes().value(u"name"_s).toString();
    const QString dateZone = xml.attributes().value(u"zone"_s).toString();
    const QString dateUtcOffset = xml.attributes().value(u"UTCOffset"_s).toString();

    QString selectTimeStamp;

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement()) {
            break;
        }

        const auto elementName = xml.name();

        if (xml.isStartElement()) {
            if (dateType == QLatin1String("xmlCreation")) {
                return;
            }
            if (dateZone == QLatin1String("UTC")) {
                return;
            }
            if (elementName == QLatin1String("year")) {
                xml.readElementText();
            } else if (elementName == QLatin1String("month")) {
                xml.readElementText();
            } else if (elementName == QLatin1String("day")) {
                xml.readElementText();
            } else if (elementName == QLatin1String("hour"))
                xml.readElementText();
            else if (elementName == QLatin1String("minute"))
                xml.readElementText();
            else if (elementName == QLatin1String("timeStamp"))
                selectTimeStamp = xml.readElementText();
            else if (elementName == QLatin1String("textSummary")) {
                if (dateType == QLatin1String("eventIssue")) {
                    if (event) {
                        event->timestamp = xml.readElementText();
                    }
                } else if (dateType == QLatin1String("observation")) {
                    xml.readElementText();
                    QDateTime observationDateTime = QDateTime::fromString(selectTimeStamp, QStringLiteral("yyyyMMddHHmmss"));
                    QTimeZone timeZone = QTimeZone(dateZone.toUtf8());
                    // if timezone id not recognized, fallback to utcoffset
                    if (!timeZone.isValid()) {
                        timeZone = QTimeZone(dateUtcOffset.toInt() * 3600);
                    }
                    if (observationDateTime.isValid() && timeZone.isValid()) {
                        data.observationDateTime = observationDateTime;
                        data.observationDateTime.setTimeZone(timeZone);
                    }
                    data.obsTimestamp = observationDateTime.toString(QStringLiteral("dd.MM.yyyy @ hh:mm"));
                } else if (dateType == QLatin1String("forecastIssue")) {
                    data.forecastTimestamp = xml.readElementText();
                } else if (dateType == QLatin1String("sunrise")) {
                    data.sunriseTimestamp = xml.readElementText();
                } else if (dateType == QLatin1String("sunset")) {
                    data.sunsetTimestamp = xml.readElementText();
                } else if (dateType == QLatin1String("moonrise")) {
                    data.moonriseTimestamp = xml.readElementText();
                } else if (dateType == QLatin1String("moonset")) {
                    data.moonsetTimestamp = xml.readElementText();
                }
            }
        }
    }
}

void EnvCanadaIon::parseLocations(WeatherData &data, QXmlStreamReader &xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("location"));

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement()) {
            break;
        }

        const auto elementName = xml.name();

        if (xml.isStartElement()) {
            if (elementName == QLatin1String("country")) {
                data.countryName = xml.readElementText();
            } else if (elementName == QLatin1String("province") || elementName == QLatin1String("territory")) {
                data.longTerritoryName = xml.readElementText();
            } else if (elementName == QLatin1String("name")) {
                data.cityName = xml.readElementText();
            } else if (elementName == QLatin1String("region")) {
                data.regionName = xml.readElementText();
            } else {
                parseUnknownElement(xml);
            }
        }
    }
}

void EnvCanadaIon::parseWindInfo(WeatherData &data, QXmlStreamReader &xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("wind"));

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement()) {
            break;
        }

        const auto elementName = xml.name();

        if (xml.isStartElement()) {
            if (elementName == QLatin1String("speed")) {
                parseFloat(data.windSpeed, xml);
            } else if (elementName == QLatin1String("gust")) {
                parseFloat(data.windGust, xml);
            } else if (elementName == QLatin1String("direction")) {
                data.windDirection = xml.readElementText();
            } else if (elementName == QLatin1String("bearing")) {
                data.windDegrees = xml.attributes().value(u"degrees"_s).toString();
            } else {
                parseUnknownElement(xml);
            }
        }
    }
}

float EnvCanadaIon::parseCoordinate(QStringView coord) const
{
    // Coordinates are in form of "64.52N" or "105.23W"
    const QRegularExpression coord_re(QStringLiteral("([0-9\\.]+)([NSEW])"));
    const QRegularExpressionMatch match = coord_re.matchView(coord);
    if (!match.hasMatch()) {
        return qQNaN();
    }

    bool ok = false;
    const float value = match.captured(1).toFloat(&ok);
    if (!ok) {
        return qQNaN();
    }

    const bool isNegative = (match.captured(2) == u"S"_s || match.captured(2) == u"W"_s);
    return isNegative ? -value : value;
};

void EnvCanadaIon::parseConditions(WeatherData &data, QXmlStreamReader &xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("currentConditions"));

    // Reset all the condition properties
    data.temperature = qQNaN();
    data.dewpoint = qQNaN();
    data.condition = i18n("N/A");
    data.humidex.clear();
    data.stationID = i18n("N/A");
    data.stationLatitude = qQNaN();
    data.stationLongitude = qQNaN();
    data.pressure = qQNaN();
    data.visibility = qQNaN();
    data.humidity = qQNaN();
    data.windSpeed = qQNaN();
    data.windGust = qQNaN();
    data.windDirection = QString();
    data.windDegrees = QString();

    while (!xml.atEnd()) {
        xml.readNext();

        const auto elementName = xml.name();

        if (xml.isEndElement() && elementName == QLatin1String("currentConditions"))
            break;

        if (xml.isStartElement()) {
            if (elementName == QLatin1String("station")) {
                data.stationID = xml.attributes().value(u"code"_s).toString();
                data.stationLatitude = parseCoordinate(xml.attributes().value(u"lat"_s).toString());
                data.stationLongitude = parseCoordinate(xml.attributes().value(u"lon"_s).toString());
            } else if (elementName == QLatin1String("dateTime")) {
                parseDateTime(data, xml);
            } else if (elementName == QLatin1String("condition")) {
                data.condition = xml.readElementText().trimmed();
            } else if (elementName == QLatin1String("temperature")) {
                // prevent N/A text to result in 0.0 value
                parseFloat(data.temperature, xml);
            } else if (elementName == QLatin1String("dewpoint")) {
                // prevent N/A text to result in 0.0 value
                parseFloat(data.dewpoint, xml);
            } else if (elementName == QLatin1String("humidex")) {
                data.humidex = xml.readElementText();
            } else if (elementName == QLatin1String("windChill")) {
                // prevent N/A text to result in 0.0 value
                parseFloat(data.windchill, xml);
            } else if (elementName == QLatin1String("pressure")) {
                data.pressureTendency = xml.attributes().value(u"tendency"_s).toString();
                if (data.pressureTendency.isEmpty()) {
                    data.pressureTendency = u"steady"_s;
                }
                parseFloat(data.pressure, xml);
            } else if (elementName == QLatin1String("visibility")) {
                parseFloat(data.visibility, xml);
            } else if (elementName == QLatin1String("relativeHumidity")) {
                parseFloat(data.humidity, xml);
            } else if (elementName == QLatin1String("wind")) {
                parseWindInfo(data, xml);
            }
        }
    }
}

void EnvCanadaIon::parseWarnings(WeatherData &data, QXmlStreamReader &xml)
{
    auto warning = std::make_shared<WeatherData::WeatherEvent>();

    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("warnings"));
    QString eventURL = xml.attributes().value(u"url"_s).toString();

    // envcan provides three type of events: 'warning', 'watch' and 'advisory'
    const auto mapToPriority = [](const QString &type) {
        if (type == QLatin1String("warning")) {
            return Warnings::High;
        }
        if (type == QLatin1String("watch")) {
            return Warnings::Medium;
        }
        return Warnings::Low;
    };

    while (!xml.atEnd()) {
        xml.readNext();

        const auto elementName = xml.name();

        if (xml.isEndElement() && elementName == QLatin1String("warnings")) {
            break;
        }

        if (xml.isStartElement()) {
            if (elementName == QLatin1String("dateTime")) {
                parseDateTime(data, xml, warning);
                if (!warning->timestamp.isEmpty() && !warning->url.isEmpty()) {
                    data.warnings.append(warning);
                    warning = std::make_shared<WeatherData::WeatherEvent>();
                }
            } else if (elementName == QLatin1String("event")) {
                // Append new event to list.
                warning->url = eventURL;
                warning->description = xml.attributes().value(u"description"_s).toString();
                warning->priority = mapToPriority(xml.attributes().value(u"type"_s).toString());
            } else {
                if (xml.name() != QLatin1String("dateTime")) {
                    parseUnknownElement(xml);
                }
            }
        }
    }
}

void EnvCanadaIon::parseWeatherForecast(WeatherData &data, QXmlStreamReader &xml)
{
    auto forecast = std::make_shared<WeatherData::ForecastInfo>();
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("forecastGroup"));

    while (!xml.atEnd()) {
        xml.readNext();

        const auto elementName = xml.name();

        if (xml.isEndElement() && elementName == QLatin1String("forecastGroup")) {
            break;
        }

        if (xml.isStartElement()) {
            if (elementName == QLatin1String("dateTime")) {
                parseDateTime(data, xml);
            } else if (elementName == QLatin1String("regionalNormals")) {
                parseRegionalNormals(data, xml);
            } else if (elementName == QLatin1String("forecast")) {
                parseForecast(data, xml, forecast);
                forecast = std::make_shared<WeatherData::ForecastInfo>();
            } else {
                parseUnknownElement(xml);
            }
        }
    }
}

void EnvCanadaIon::parseRegionalNormals(WeatherData &data, QXmlStreamReader &xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("regionalNormals"));

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement()) {
            break;
        }

        const auto elementName = xml.name();

        if (xml.isStartElement()) {
            if (elementName == QLatin1String("textSummary")) {
                xml.readElementText();
            } else if (elementName == QLatin1String("temperature") && xml.attributes().value(u"class"_s) == QLatin1String("high")) {
                // prevent N/A text to result in 0.0 value
                parseFloat(data.normalHigh, xml);
            } else if (elementName == QLatin1String("temperature") && xml.attributes().value(u"class"_s) == QLatin1String("low")) {
                // prevent N/A text to result in 0.0 value
                parseFloat(data.normalLow, xml);
            }
        }
    }
}

void EnvCanadaIon::parseForecast(WeatherData &data, QXmlStreamReader &xml, std::shared_ptr<WeatherData::ForecastInfo> forecast)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("forecast"));

    while (!xml.atEnd()) {
        xml.readNext();

        const auto elementName = xml.name();

        if (xml.isEndElement() && elementName == QLatin1String("forecast")) {
            data.forecasts.append(forecast);
            break;
        }

        if (xml.isStartElement()) {
            if (elementName == QLatin1String("period")) {
                forecast->forecastPeriod = xml.attributes().value(u"textForecastName"_s).toString();
            } else if (elementName == QLatin1String("textSummary")) {
                forecast->forecastSummary = xml.readElementText();
            } else if (elementName == QLatin1String("abbreviatedForecast")) {
                parseShortForecast(forecast, xml);
            } else if (elementName == QLatin1String("temperatures")) {
                parseForecastTemperatures(forecast, xml);
            } else if (elementName == QLatin1String("winds")) {
                parseWindForecast(forecast, xml);
            } else if (elementName == QLatin1String("precipitation")) {
                parsePrecipitationForecast(forecast, xml);
            } else if (elementName == QLatin1String("uv")) {
                data.UVRating = xml.attributes().value(u"category"_s).toString();
                parseUVIndex(data, xml);
                // else if (elementName == QLatin1String("frost")) { FIXME: Wait until winter to see what this looks like.
                //  parseFrost(xml, forecast);
            } else {
                if (elementName != QLatin1String("forecast")) {
                    parseUnknownElement(xml);
                }
            }
        }
    }
}

void EnvCanadaIon::parseShortForecast(std::shared_ptr<WeatherData::ForecastInfo> forecast, QXmlStreamReader &xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("abbreviatedForecast"));

    QString shortText;

    while (!xml.atEnd()) {
        xml.readNext();

        const auto elementName = xml.name();

        if (xml.isEndElement() && elementName == QLatin1String("abbreviatedForecast")) {
            break;
        }

        if (xml.isStartElement()) {
            if (elementName == QLatin1String("pop")) {
                parseFloat(forecast->popPrecent, xml);
            }
            if (elementName == QLatin1String("textSummary")) {
                shortText = xml.readElementText();
                QMap<QString, ConditionIcons> forecastList = forecastIcons();
                if ((forecast->forecastPeriod == QLatin1String("tonight")) || (forecast->forecastPeriod.contains(QLatin1String("night")))) {
                    forecastList.insert(QStringLiteral("a few clouds"), FewCloudsNight);
                    forecastList.insert(QStringLiteral("cloudy periods"), PartlyCloudyNight);
                    forecastList.insert(QStringLiteral("chance of drizzle mixed with rain"), ChanceShowersNight);
                    forecastList.insert(QStringLiteral("chance of drizzle"), ChanceShowersNight);
                    forecastList.insert(QStringLiteral("chance of drizzle or rain"), ChanceShowersNight);
                    forecastList.insert(QStringLiteral("chance of flurries"), ChanceSnowNight);
                    forecastList.insert(QStringLiteral("chance of light snow"), ChanceSnowNight);
                    forecastList.insert(QStringLiteral("chance of flurries at times heavy"), ChanceSnowNight);
                    forecastList.insert(QStringLiteral("chance of showers or drizzle"), ChanceShowersNight);
                    forecastList.insert(QStringLiteral("chance of showers"), ChanceShowersNight);
                    forecastList.insert(QStringLiteral("clearing"), ClearNight);
                } else {
                    forecastList.insert(QStringLiteral("a few clouds"), FewCloudsDay);
                    forecastList.insert(QStringLiteral("cloudy periods"), PartlyCloudyDay);
                    forecastList.insert(QStringLiteral("chance of drizzle mixed with rain"), ChanceShowersDay);
                    forecastList.insert(QStringLiteral("chance of drizzle"), ChanceShowersDay);
                    forecastList.insert(QStringLiteral("chance of drizzle or rain"), ChanceShowersDay);
                    forecastList.insert(QStringLiteral("chance of flurries"), ChanceSnowDay);
                    forecastList.insert(QStringLiteral("chance of light snow"), ChanceSnowDay);
                    forecastList.insert(QStringLiteral("chance of flurries at times heavy"), ChanceSnowDay);
                    forecastList.insert(QStringLiteral("chance of showers or drizzle"), ChanceShowersDay);
                    forecastList.insert(QStringLiteral("chance of showers"), ChanceShowersDay);
                    forecastList.insert(QStringLiteral("clearing"), ClearDay);
                }
                forecast->shortForecast = shortText;
                forecast->iconName = getWeatherIcon(forecastList, shortText.toLower());
            }
        }
    }
}

void EnvCanadaIon::parseUVIndex(WeatherData &data, QXmlStreamReader &xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("uv"));

    while (!xml.atEnd()) {
        xml.readNext();

        const auto elementName = xml.name();

        if (xml.isEndElement() && elementName == QLatin1String("uv")) {
            break;
        }

        if (xml.isStartElement()) {
            if (elementName == QLatin1String("index")) {
                data.UVIndex = xml.readElementText();
            }
            if (elementName == QLatin1String("textSummary")) {
                xml.readElementText();
            }
        }
    }
}

void EnvCanadaIon::parseForecastTemperatures(std::shared_ptr<WeatherData::ForecastInfo> forecast, QXmlStreamReader &xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("temperatures"));

    while (!xml.atEnd()) {
        xml.readNext();

        const auto elementName = xml.name();

        if (xml.isEndElement() && elementName == QLatin1String("temperatures")) {
            break;
        }

        if (xml.isStartElement()) {
            if (elementName == QLatin1String("temperature") && xml.attributes().value(u"class"_s) == QLatin1String("low")) {
                parseFloat(forecast->tempLow, xml);
            } else if (elementName == QLatin1String("temperature") && xml.attributes().value(u"class"_s) == QLatin1String("high")) {
                parseFloat(forecast->tempHigh, xml);
            } else if (elementName == QLatin1String("textSummary")) {
                xml.readElementText();
            }
        }
    }
}

void EnvCanadaIon::parsePrecipitationForecast(std::shared_ptr<WeatherData::ForecastInfo> forecast, QXmlStreamReader &xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("precipitation"));

    while (!xml.atEnd()) {
        xml.readNext();

        const auto elementName = xml.name();

        if (xml.isEndElement() && elementName == QLatin1String("precipitation")) {
            break;
        }

        if (xml.isStartElement()) {
            if (elementName == QLatin1String("textSummary")) {
                forecast->precipForecast = xml.readElementText();
            } else if (elementName == QLatin1String("precipType")) {
                forecast->precipType = xml.readElementText();
            } else if (elementName == QLatin1String("accumulation")) {
                parsePrecipTotals(forecast, xml);
            }
        }
    }
}

void EnvCanadaIon::parsePrecipTotals(std::shared_ptr<WeatherData::ForecastInfo> forecast, QXmlStreamReader &xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("accumulation"));

    while (!xml.atEnd()) {
        xml.readNext();

        const auto elementName = xml.name();

        if (xml.isEndElement() && elementName == QLatin1String("accumulation")) {
            break;
        }

        if (elementName == QLatin1String("name")) {
            xml.readElementText();
        } else if (elementName == QLatin1String("amount")) {
            forecast->precipTotalExpected = xml.readElementText();
        }
    }
}

void EnvCanadaIon::parseWindForecast(std::shared_ptr<WeatherData::ForecastInfo> forecast, QXmlStreamReader &xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("winds"));

    while (!xml.atEnd()) {
        xml.readNext();

        const auto elementName = xml.name();

        if (xml.isEndElement() && elementName == QLatin1String("winds")) {
            break;
        }

        if (xml.isStartElement()) {
            if (elementName == QLatin1String("textSummary")) {
                forecast->windForecast = xml.readElementText();
            } else {
                if (xml.name() != QLatin1String("winds")) {
                    parseUnknownElement(xml);
                }
            }
        }
    }
}

void EnvCanadaIon::parseYesterdayWeather(WeatherData &data, QXmlStreamReader &xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("yesterdayConditions"));

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement()) {
            break;
        }

        const auto elementName = xml.name();

        if (xml.isStartElement()) {
            if (elementName == QLatin1String("temperature") && xml.attributes().value(u"class"_s) == QLatin1String("high")) {
                parseFloat(data.prevHigh, xml);
            } else if (elementName == QLatin1String("temperature") && xml.attributes().value(u"class"_s) == QLatin1String("low")) {
                parseFloat(data.prevLow, xml);
            } else if (elementName == QLatin1String("precip")) {
                data.prevPrecipType = xml.attributes().value(u"units"_s).toString();
                if (data.prevPrecipType.isEmpty()) {
                    data.prevPrecipType = QString::number(KUnitConversion::NoUnit);
                }
                parseFloat(data.prevPrecipTotal, xml);
            }
        }
    }
}

void EnvCanadaIon::parseWeatherRecords(WeatherData &data, QXmlStreamReader &xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("almanac"));

    while (!xml.atEnd()) {
        xml.readNext();

        const auto elementName = xml.name();

        if (xml.isEndElement() && elementName == QLatin1String("almanac")) {
            break;
        }

        if (xml.isStartElement()) {
            if (elementName == QLatin1String("temperature") && xml.attributes().value(u"class"_s) == QLatin1String("extremeMax")) {
                parseFloat(data.recordHigh, xml);
            } else if (elementName == QLatin1String("temperature") && xml.attributes().value(u"class"_s) == QLatin1String("extremeMin")) {
                parseFloat(data.recordLow, xml);
            } else if (elementName == QLatin1String("precipitation") && xml.attributes().value(u"class"_s) == QLatin1String("extremeRainfall")) {
                parseFloat(data.recordRain, xml);
            } else if (elementName == QLatin1String("precipitation") && xml.attributes().value(u"class"_s) == QLatin1String("extremeSnowfall")) {
                parseFloat(data.recordSnow, xml);
            }
        }
    }
}

void EnvCanadaIon::parseAstronomicals(WeatherData &data, QXmlStreamReader &xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("riseSet"));

    while (!xml.atEnd()) {
        xml.readNext();

        const auto elementName = xml.name();

        if (xml.isEndElement() && elementName == QLatin1String("riseSet")) {
            break;
        }

        if (xml.isStartElement()) {
            if (elementName == QLatin1String("disclaimer")) {
                xml.readElementText();
            } else if (elementName == QLatin1String("dateTime")) {
                parseDateTime(data, xml);
            }
        }
    }
}

// handle when no XML tag is found
void EnvCanadaIon::parseUnknownElement(QXmlStreamReader &xml) const
{
    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement()) {
            break;
        }

        if (xml.isStartElement()) {
            parseUnknownElement(xml);
        }
    }
}

QString EnvCanadaIon::updateForecastPeriod(const std::shared_ptr<WeatherData::ForecastInfo> &info)
{
    QString forecastPeriod = info->forecastPeriod;

    if (forecastPeriod.isEmpty()) {
        forecastPeriod = i18n("N/A");
    } else {
        // We need to shortform the week day strings and remove the "night" word because FutureDays already
        // set "day" and "night" labels.
        const QString today = i18n("Today");
        forecastPeriod.replace(QStringLiteral("Today"), today);
        forecastPeriod.replace(QStringLiteral("Tonight"), today);
        forecastPeriod.replace(QStringLiteral("night"), u""_s);
        forecastPeriod.replace(QStringLiteral("Saturday"), i18nc("Short for Saturday", "Sat"));
        forecastPeriod.replace(QStringLiteral("Sunday"), i18nc("Short for Sunday", "Sun"));
        forecastPeriod.replace(QStringLiteral("Monday"), i18nc("Short for Monday", "Mon"));
        forecastPeriod.replace(QStringLiteral("Tuesday"), i18nc("Short for Tuesday", "Tue"));
        forecastPeriod.replace(QStringLiteral("Wednesday"), i18nc("Short for Wednesday", "Wed"));
        forecastPeriod.replace(QStringLiteral("Thursday"), i18nc("Short for Thursday", "Thu"));
        forecastPeriod.replace(QStringLiteral("Friday"), i18nc("Short for Friday", "Fri"));
    }

    return forecastPeriod;
}

FutureForecast EnvCanadaIon::forecastInfoToFutureForecast(const std::shared_ptr<WeatherData::ForecastInfo> &info)
{
    const QString shortForecast = info->shortForecast.isEmpty() ? i18n("N/A") : i18nc("weather forecast", info->shortForecast.toUtf8().data());

    FutureForecast futureForecast;

    futureForecast.setConditionIcon(info->iconName);
    futureForecast.setCondition(shortForecast);
    if (!qIsNaN(info->tempHigh)) {
        futureForecast.setHighTemp(info->tempHigh);
    }
    if (!qIsNaN(info->tempLow)) {
        futureForecast.setLowTemp(info->tempLow);
    }
    if (!qIsNaN(info->popPrecent)) {
        futureForecast.setConditionProbability(info->popPrecent);
    }

    return futureForecast;
}

void EnvCanadaIon::updateWeather()
{
    m_weatherData->isNight = isNightTime(m_weatherData->observationDateTime, m_weatherData->stationLatitude, m_weatherData->stationLongitude);

    auto forecast = std::make_shared<Forecast>();

    MetaData metaData;

    metaData.setCredit(i18nc("credit line, keep string short", "Data from Environment and Climate Change\302\240Canada"));
    metaData.setCreditURL(m_weatherData->creditUrl);
    metaData.setTemperatureUnit(KUnitConversion::Celsius);
    metaData.setPressureUnit(KUnitConversion::Kilopascal);
    metaData.setVisibilityUnit(KUnitConversion::Kilometer);
    metaData.setHumidityUnit(KUnitConversion::Percent);
    metaData.setWindSpeedUnit(KUnitConversion::KilometerPerHour);
    metaData.setRainfallUnit(KUnitConversion::Millimeter);
    metaData.setSnowfallUnit(KUnitConversion::Centimeter);
    if (!m_weatherData->prevPrecipType.isEmpty()) {
        if (m_weatherData->prevPrecipType == QLatin1String("mm")) {
            metaData.setPrecipUnit(KUnitConversion::Millimeter);
        } else if (m_weatherData->prevPrecipType == QLatin1String("cm")) {
            metaData.setPrecipUnit(KUnitConversion::Centimeter);
        }
    }

    forecast->setMetadata(metaData);

    Station station;

    station.setCountry(m_weatherData->countryName);
    station.setPlace(m_weatherData->cityName + u", "_s + m_weatherData->shortTerritoryName);
    station.setRegion(m_weatherData->regionName);
    station.setStation(m_weatherData->stationID.isEmpty() ? i18n("N/A") : m_weatherData->stationID.toUpper());

    if (!qIsNaN(m_weatherData->stationLatitude) && !qIsNaN(m_weatherData->stationLongitude)) {
        station.setCoordinates(m_weatherData->stationLatitude, m_weatherData->stationLongitude);
    }

    if (m_isLegacy) {
        station.setNewPlaceInfo(m_territoryName + u"|"_s + m_cityCode);
    }

    forecast->setStation(station);

    LastObservation lastObservation;

    // Real weather - Current conditions
    if (m_weatherData->observationDateTime.isValid()) {
        lastObservation.setObservationTimestamp(m_weatherData->observationDateTime);
    }

    if (!m_weatherData->condition.isEmpty()) {
        lastObservation.setCurrentConditions(i18nc("weather condition", m_weatherData->condition.toUtf8().data()));
    }

    QMap<QString, ConditionIcons> conditionList = conditionIcons();

    if (m_weatherData->isNight) {
        conditionList.insert(QStringLiteral("decreasing cloud"), FewCloudsNight);
        conditionList.insert(QStringLiteral("mostly cloudy"), PartlyCloudyNight);
        conditionList.insert(QStringLiteral("partly cloudy"), PartlyCloudyNight);
        conditionList.insert(QStringLiteral("fair"), FewCloudsNight);
    } else {
        conditionList.insert(QStringLiteral("decreasing cloud"), FewCloudsDay);
        conditionList.insert(QStringLiteral("mostly cloudy"), PartlyCloudyDay);
        conditionList.insert(QStringLiteral("partly cloudy"), PartlyCloudyDay);
        conditionList.insert(QStringLiteral("fair"), FewCloudsDay);
    }

    lastObservation.setConditionIcon(getWeatherIcon(conditionList, m_weatherData->condition));

    if (!qIsNaN(m_weatherData->temperature)) {
        lastObservation.setTemperature(m_weatherData->temperature);
    }
    if (!qIsNaN(m_weatherData->windchill)) {
        lastObservation.setWindchill(m_weatherData->windchill);
    }
    if (!m_weatherData->humidex.isEmpty()) {
        lastObservation.setHumidex(m_weatherData->humidex);
    }

    if (!qIsNaN(m_weatherData->dewpoint)) {
        lastObservation.setDewpoint(m_weatherData->dewpoint);
    }

    if (!qIsNaN(m_weatherData->pressure)) {
        lastObservation.setPressure(m_weatherData->pressure);
        if (m_weatherData->pressureTendency == u"rising"_s) {
            lastObservation.setPressureTendency(i18n("Rising"));
        } else if (m_weatherData->pressureTendency == u"falling"_s) {
            lastObservation.setPressureTendency(i18n("Falling"));
        } else {
            lastObservation.setPressureTendency(i18n("Steady"));
        }
    }

    if (!qIsNaN(m_weatherData->visibility)) {
        lastObservation.setVisibility(m_weatherData->visibility);
    }

    if (!qIsNaN(m_weatherData->humidity)) {
        lastObservation.setHumidity(m_weatherData->humidity);
    }

    if (!qIsNaN(m_weatherData->windSpeed)) {
        lastObservation.setWindSpeed(m_weatherData->windSpeed);
    }
    if (!qIsNaN(m_weatherData->windGust)) {
        lastObservation.setWindGust(m_weatherData->windGust);
    }

    if (!qIsNaN(m_weatherData->windSpeed) || !qIsNaN(m_weatherData->windGust)) { }

    if (!qIsNaN(m_weatherData->windSpeed) && static_cast<int>(m_weatherData->windSpeed) == 0) {
        lastObservation.setWindDirection(QStringLiteral("VR")); // Variable/calm
    } else if (!m_weatherData->windDirection.isEmpty()) {
        lastObservation.setWindDirection(m_weatherData->windDirection);
    }

    // Check if UV index is available for the location
    if (!m_weatherData->UVIndex.isEmpty()) {
        lastObservation.setUVIndex(m_weatherData->UVIndex.toInt());
    }

    forecast->setLastObservation(lastObservation);

    CurrentDay currentDay;

    if (!qIsNaN(m_weatherData->normalHigh)) {
        currentDay.setNormalHighTemp(m_weatherData->normalHigh);
    }
    if (!qIsNaN(m_weatherData->normalLow)) {
        currentDay.setNormalLowTemp(m_weatherData->normalLow);
    }
    // records
    if (!qIsNaN(m_weatherData->recordHigh)) {
        currentDay.setRecordHighTemp(m_weatherData->recordHigh);
    }
    if (!qIsNaN(m_weatherData->recordLow)) {
        currentDay.setRecordLowTemp(m_weatherData->recordLow);
    }
    if (!qIsNaN(m_weatherData->recordRain)) {
        currentDay.setRecordRainfall(m_weatherData->recordRain);
    }
    if (!qIsNaN(m_weatherData->recordSnow)) {
        currentDay.setRecordSnowfall(m_weatherData->recordSnow);
    }

    forecast->setCurrentDay(currentDay);

    auto returnWarnings = std::make_shared<Warnings>();

    const auto &warnings = m_weatherData->warnings;

    for (int k = 0; k < warnings.size(); ++k) {
        const std::shared_ptr<WeatherData::WeatherEvent> &warning = warnings.at(k);

        Warning returnWarning(warning->priority, warning->description);
        returnWarning.setInfo(warning->url);
        returnWarning.setTimestamp(warning->timestamp);

        returnWarnings->addWarning(returnWarning);
    }

    forecast->setWarnings(returnWarnings);

    auto futureDays = std::make_shared<FutureDays>();

    const auto &forecasts = m_weatherData->forecasts;

    // m_weatherData->forecasts is a list with mixed day and night forecasts. Convert them to
    // FutureDays which have day and night spit. Also update and properly show forecast period.
    for (int i = 0; i < forecasts.size(); ++i) {
        const auto &current = forecasts[i];

        std::shared_ptr<WeatherData::ForecastInfo> next = nullptr;

        if (i + 1 < forecasts.size()) {
            next = forecasts[i + 1];
        }

        // The first two forecasts are named "Today" and "Tonight". It's possible that "Today" is omitted.
        // If the first day is a "Today" forecast add it to a full day forecast
        if (current->forecastPeriod == QStringLiteral("Today")) {
            // if the second day is a "Tonight" forecast add it too
            if (next && next->forecastPeriod == QStringLiteral("Tonight")) {
                FutureDayForecast futureDayForecast;
                futureDayForecast.setWeekDay(updateForecastPeriod(current));
                futureDayForecast.setDaytime(forecastInfoToFutureForecast(current));
                futureDayForecast.setNight(forecastInfoToFutureForecast(next));
                futureDays->addDay(futureDayForecast);
                ++i;
            } else {
                // else just create a full day forecast only with a "Today" forecast
                FutureDayForecast futureDayForecast;
                futureDayForecast.setWeekDay(updateForecastPeriod(current));
                futureDayForecast.setDaytime(forecastInfoToFutureForecast(current));
                futureDays->addDay(futureDayForecast);
            }
            // if the forecast starts with night then create a full day forecast only with "Tonight"
        } else if (current->forecastPeriod == QStringLiteral("Tonight")) {
            FutureDayForecast futureDayForecast;
            futureDayForecast.setWeekDay(updateForecastPeriod(current));
            futureDayForecast.setNight(forecastInfoToFutureForecast(current));
            futureDays->addDay(futureDayForecast);
        } else {
            // Then check for next days. Next days are named according to the week days, but with exception
            // that in the night forecasts added "night" at the end of the name.
            // check if the forecast is a daytime forecast
            if (!current->forecastPeriod.contains(QStringLiteral("night"))) {
                // if yes then check the next forecast for night
                if (next && next->forecastPeriod.contains(QStringLiteral("night")) && next->forecastPeriod.contains(current->forecastPeriod)) {
                    // if the next forecast is a night forecast then create full day forecast with day and night forecasts accordingly
                    FutureDayForecast futureDayForecast;
                    futureDayForecast.setWeekDay(updateForecastPeriod(current));
                    futureDayForecast.setDaytime(forecastInfoToFutureForecast(current));
                    futureDayForecast.setNight(forecastInfoToFutureForecast(next));
                    futureDays->addDay(futureDayForecast);
                    ++i;
                } else {
                    // if the next forecast is not a night forecast then add only the daytime forecast
                    FutureDayForecast futureDayForecast;
                    futureDayForecast.setWeekDay(updateForecastPeriod(current));
                    futureDayForecast.setDaytime(forecastInfoToFutureForecast(current));
                    futureDays->addDay(futureDayForecast);
                }
            } else {
                // if no then create a full day forecast just with the night forecast
                FutureDayForecast futureDayForecast;
                futureDayForecast.setWeekDay(updateForecastPeriod(current));
                futureDayForecast.setNight(forecastInfoToFutureForecast(current));
                futureDays->addDay(futureDayForecast);
            }
        }
    }

    qCDebug(WEATHER::ION::ENVCAN) << "Forecasts added. Total: " << futureDays->columnCount();

    forecast->setFutureDays(futureDays);

    LastDay lastDay;

    // yesterday
    if (!qIsNaN(m_weatherData->prevHigh)) {
        lastDay.setNormalHighTemp(m_weatherData->prevHigh);
    }
    if (!qIsNaN(m_weatherData->prevLow)) {
        lastDay.setNormalLowTemp(m_weatherData->prevLow);
    }
    if (!m_weatherData->prevPrecipType.isEmpty()) {
        lastDay.setPrecipTotal(m_weatherData->prevPrecipTotal);
    }

    forecast->setLastDay(lastDay);

    m_forecastPromise->addResult(forecast);

    qCDebug(WEATHER::ION::ENVCAN) << "Updated weather for: " << m_territoryName << ", " << m_cityCode;
}

#include "ion_envcan.moc"
#include "moc_ion_envcan.cpp"
