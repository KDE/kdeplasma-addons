/*
 *   SPDX-FileCopyrightText: 2007-2009 Shawn Starr <shawn.starr@rogers.com>
 *   SPDX-FileCopyrightText: 2008 Marco Martin <notmart@gmail.com>
 *   SPDX-FileCopyrightText: 2012 Luís Gabriel Lima <lampih@gmail.com>
 *   SPDX-FileCopyrightText: 2017-2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "weatherapplet.h"

#include <Plasma/DataContainer>
#include <Plasma/DataEngine>

// KF
#include <KConfigGroup>
#include <KLocalizedString>
#include <KUnitConversion/Value>

using namespace KUnitConversion;

namespace
{
namespace AppletConfigKeys
{
inline QString services()
{
    return QStringLiteral("services");
}

inline QString showTemperatureInTooltip()
{
    return QStringLiteral("showTemperatureInTooltip");
}

inline QString showWindInTooltip()
{
    return QStringLiteral("showWindInTooltip");
}

inline QString showPressureInTooltip()
{
    return QStringLiteral("showPressureInTooltip");
}

inline QString showHumidityInTooltip()
{
    return QStringLiteral("showHumidityInTooltip");
}

inline QString showTemperatureInCompactMode()
{
    return QStringLiteral("showTemperatureInCompactMode");
}

inline QString temperatureUnitId()
{
    return QStringLiteral("temperatureUnitId");
}

inline QString windSpeedUnitId()
{
    return QStringLiteral("windSpeedUnitId");
}

inline QString pressureUnitId()
{
    return QStringLiteral("pressureUnitId");
}

inline QString visibilityUnitId()
{
    return QStringLiteral("visibilityUnitId");
}

inline QString updateInterval()
{
    return QStringLiteral("updateInterval");
}

inline QString source()
{
    return QStringLiteral("source");
}
}
namespace StorageConfigKeys
{
const char weatherServiceProviders[] = "weatherServiceProviders";
const char showTemperatureInTooltip[] = "showTemperatureInTooltip";
const char showWindInTooltip[] = "showWindInTooltip";
const char showPressureInTooltip[] = "showPressureInTooltip";
const char showHumidityInTooltip[] = "showHumidityInTooltip";
const char showTemperatureInCompactMode[] = "showTemperatureInCompactMode";
const char temperatureUnit[] = "temperatureUnit";
const char speedUnit[] = "speedUnit";
const char pressureUnit[] = "pressureUnit";
const char visibilityUnit[] = "visibilityUnit";
const char updateInterval[] = "updateInterval";
const char source[] = "source";
}
}

WeatherApplet::WeatherApplet(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args)
{
    Plasma::DataEngine *dataengine = dataEngine(QStringLiteral("weather"));
    const QVariantList plugins = dataengine->containerForSource(QLatin1String("ions"))->data().values();
    for (const QVariant &plugin : plugins) {
        const QStringList pluginInfo = plugin.toString().split(QLatin1Char('|'));
        if (pluginInfo.count() > 1) {
            m_defaultProviders << pluginInfo[1];
        }
    }
}

void WeatherApplet::init()
{
    configChanged();
}

void WeatherApplet::configChanged()
{
    KConfigGroup cfg = config();

    // snapshot current values
    const QString oldSource = m_source;
    const int oldUpdateInterval = m_updateInterval;
    const bool oldTemperatureShownInTooltip = m_temperatureShownInTooltip;
    const bool oldWindShownInTooltip = m_windShownInTooltip;
    const bool oldPressureShownInTooltip = m_pressureShownInTooltip;
    const bool oldHumidityShownInTooltip = m_humidityShownInTooltip;
    const bool oldTemperatureShownInCompactMode = m_temperatureShownInCompactMode;
    const Unit oldDisplayTemperatureUnit = m_displayTemperatureUnit;
    const Unit oldDisplaySpeedUnit = m_displaySpeedUnit;
    const Unit oldDisplayPressureUnit = m_displayPressureUnit;
    const Unit oldDisplayVisibilityUnit = m_displayVisibilityUnit;

    // read values from config storage
    const bool useMetric = (QLocale().measurementSystem() == QLocale::MetricSystem);

    m_displayTemperatureUnit = unit(cfg.readEntry(StorageConfigKeys::temperatureUnit, (useMetric ? "C" : "F")));
    m_displaySpeedUnit = unit(cfg.readEntry(StorageConfigKeys::speedUnit, (useMetric ? "m/s" : "mph")));
    m_displayPressureUnit = unit(cfg.readEntry(StorageConfigKeys::pressureUnit, (useMetric ? "hPa" : "inHg")));
    m_displayVisibilityUnit = unit(cfg.readEntry(StorageConfigKeys::visibilityUnit, (useMetric ? "km" : "mi")));

    m_updateInterval = cfg.readEntry(StorageConfigKeys::updateInterval, 30);
    m_source = cfg.readEntry(StorageConfigKeys::source, QString());

    m_temperatureShownInTooltip = cfg.readEntry(StorageConfigKeys::showTemperatureInTooltip, true);
    m_windShownInTooltip = cfg.readEntry(StorageConfigKeys::showWindInTooltip, false);
    m_pressureShownInTooltip = cfg.readEntry(StorageConfigKeys::showPressureInTooltip, false);
    m_humidityShownInTooltip = cfg.readEntry(StorageConfigKeys::showHumidityInTooltip, false);
    m_temperatureShownInCompactMode = cfg.readEntry(StorageConfigKeys::showTemperatureInCompactMode, false);

    setConfigurationRequired(m_source.isEmpty());

    if (oldTemperatureShownInTooltip != m_temperatureShownInTooltip) {
        Q_EMIT temperatureShownInTooltipChanged();
    }
    if (oldWindShownInTooltip != m_windShownInTooltip) {
        Q_EMIT windShownInTooltipChanged();
    }
    if (oldPressureShownInTooltip != m_pressureShownInTooltip) {
        Q_EMIT pressureShownInTooltipChanged();
    }
    if (oldHumidityShownInTooltip != m_humidityShownInTooltip) {
        Q_EMIT humidityShownInTooltipChanged();
    }
    if (oldTemperatureShownInCompactMode != m_temperatureShownInCompactMode) {
        Q_EMIT temperatureShownInCompactModeChanged();
    }
    if (oldSource != m_source) {
        Q_EMIT sourceChanged();
    }
    if (oldUpdateInterval != m_updateInterval) {
        Q_EMIT updateIntervalChanged();
    }
    if (oldDisplayTemperatureUnit != m_displayTemperatureUnit //
        || oldDisplaySpeedUnit != m_displaySpeedUnit //
        || oldDisplayPressureUnit != m_displayPressureUnit //
        || oldDisplayVisibilityUnit != m_displayVisibilityUnit) {
        Q_EMIT displayUnitsChanged();
    }
}

WeatherApplet::~WeatherApplet()
{
}

QVariantMap WeatherApplet::configValues() const
{
    KConfigGroup cfg = this->config();

    return QVariantMap{
        // UI settings
        {AppletConfigKeys::services(), cfg.readEntry(StorageConfigKeys::weatherServiceProviders, m_defaultProviders)},
        {AppletConfigKeys::showTemperatureInTooltip(), cfg.readEntry(StorageConfigKeys::showTemperatureInTooltip, true)},
        {AppletConfigKeys::showWindInTooltip(), cfg.readEntry(StorageConfigKeys::showWindInTooltip, false)},
        {AppletConfigKeys::showPressureInTooltip(), cfg.readEntry(StorageConfigKeys::showPressureInTooltip, false)},
        {AppletConfigKeys::showHumidityInTooltip(), cfg.readEntry(StorageConfigKeys::showHumidityInTooltip, false)},
        {AppletConfigKeys::showTemperatureInCompactMode(), cfg.readEntry(StorageConfigKeys::showTemperatureInCompactMode, false)},

        // units
        {AppletConfigKeys::temperatureUnitId(), m_displayTemperatureUnit.id()},
        {AppletConfigKeys::windSpeedUnitId(), m_displaySpeedUnit.id()},
        {AppletConfigKeys::pressureUnitId(), m_displayPressureUnit.id()},
        {AppletConfigKeys::visibilityUnitId(), m_displayVisibilityUnit.id()},

        // data source
        {AppletConfigKeys::updateInterval(), m_updateInterval},
        {AppletConfigKeys::source(), m_source},
    };
}

void WeatherApplet::saveConfig(const QVariantMap &configChanges)
{
    KConfigGroup cfg = config();

    // UI settings
    auto it = configChanges.find(AppletConfigKeys::services());
    if (it != configChanges.end()) {
        cfg.writeEntry(StorageConfigKeys::weatherServiceProviders, it.value().toStringList());
    }
    it = configChanges.find(AppletConfigKeys::showTemperatureInTooltip());
    if (it != configChanges.end()) {
        cfg.writeEntry(StorageConfigKeys::showTemperatureInTooltip, it.value().toBool());
    }
    it = configChanges.find(AppletConfigKeys::showWindInTooltip());
    if (it != configChanges.end()) {
        cfg.writeEntry(StorageConfigKeys::showWindInTooltip, it.value().toBool());
    }
    it = configChanges.find(AppletConfigKeys::showPressureInTooltip());
    if (it != configChanges.end()) {
        cfg.writeEntry(StorageConfigKeys::showPressureInTooltip, it.value().toBool());
    }
    it = configChanges.find(AppletConfigKeys::showHumidityInTooltip());
    if (it != configChanges.end()) {
        cfg.writeEntry(StorageConfigKeys::showHumidityInTooltip, it.value().toBool());
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

    Q_EMIT configNeedsSaving();
}

Unit WeatherApplet::unit(const QString &unit)
{
    if (!unit.isEmpty() && unit[0].isDigit()) {
        return m_converter.unit(static_cast<UnitId>(unit.toInt()));
    }
    // Support < 4.4 config values
    return m_converter.unit(unit);
}

K_PLUGIN_CLASS_WITH_JSON(WeatherApplet, "package/metadata.json")

#include "weatherapplet.moc"
