/***************************************************************************
 *   Copyright (C) 2007-2009 by Shawn Starr <shawn.starr@rogers.com>       *
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

#ifndef WEATHERAPPLET_H
#define WEATHERAPPLET_H

#include <KUnitConversion/Unit>
#include <KUnitConversion/Converter>

#include <Plasma/Applet>

// TODO: remove C++ applet and use main.xml-based config
// Blocked by:
// configuration has been stored via config() and without further config sub group
// E.g.
// [Containments][18][Applets][101][Configuration]
// main.xml based config storage though uses additional subgroups using the name attribute of
// the <group> containers, so e.g. <group name="General"> would use
// [Containments][18][Applets][101][Configuration][General]
// Challenge: migrating configuration e.g. using kconf_update? how?
class WeatherApplet : public Plasma::Applet
{
    Q_OBJECT
    // used for making this information available to the config pages
    Q_PROPERTY(bool needsToBeSquare MEMBER m_needsToBeSquare NOTIFY needsToBeSquareChanged FINAL)

    // config properties
    Q_PROPERTY(QString source READ source NOTIFY sourceChanged FINAL)
    Q_PROPERTY(int updateInterval READ updateInterval NOTIFY updateIntervalChanged FINAL)

    Q_PROPERTY(int displayTemperatureUnit READ displayTemperatureUnit NOTIFY displayUnitsChanged FINAL)
    Q_PROPERTY(int displaySpeedUnit READ displaySpeedUnit NOTIFY displayUnitsChanged FINAL)
    Q_PROPERTY(int displayPressureUnit READ displayPressureUnit NOTIFY displayUnitsChanged FINAL)
    Q_PROPERTY(int displayVisibilityUnit READ displayVisibilityUnit NOTIFY displayUnitsChanged FINAL)

    Q_PROPERTY(bool temperatureShownInTooltip READ temperatureShownInTooltip NOTIFY temperatureShownInTooltipChanged FINAL)
    Q_PROPERTY(bool windShownInTooltip READ windShownInTooltip NOTIFY windShownInTooltipChanged FINAL)
    Q_PROPERTY(bool pressureShownInTooltip READ pressureShownInTooltip NOTIFY pressureShownInTooltipChanged FINAL)
    Q_PROPERTY(bool humidityShownInTooltip READ humidityShownInTooltip NOTIFY humidityShownInTooltipChanged FINAL)
    Q_PROPERTY(bool temperatureShownInCompactMode READ temperatureShownInCompactMode NOTIFY temperatureShownInCompactModeChanged FINAL)

public:
    WeatherApplet(QObject *parent, const QVariantList &args);
    ~WeatherApplet() override;

public: // Plasma::Applet API
    void init() override;
    void configChanged() override;

public: // QML config control API
    /**
     * @return currently used config values
     */
    Q_INVOKABLE QVariantMap configValues() const;

    /**
     * @param configChanges config key-value entries which have changed
     */
    Q_INVOKABLE void saveConfig(const QVariantMap& configChanges);

public:
    QString source() const     { return m_source; }
    int updateInterval() const { return m_updateInterval; }

    int displayTemperatureUnit() const { return m_displayTemperatureUnit.id(); }
    int displaySpeedUnit() const       { return m_displaySpeedUnit.id(); }
    int displayPressureUnit() const    { return m_displayPressureUnit.id(); }
    int displayVisibilityUnit() const  { return m_displayVisibilityUnit.id(); }

    bool temperatureShownInTooltip() const  { return m_temperatureShownInTooltip; }
    bool windShownInTooltip() const         { return m_windShownInTooltip; }
    bool pressureShownInTooltip() const     { return m_pressureShownInTooltip; }
    bool humidityShownInTooltip() const     { return m_humidityShownInTooltip; }

    bool temperatureShownInCompactMode() const  { return m_temperatureShownInCompactMode; }

Q_SIGNALS:
    void configurationChanged();
    void needsToBeSquareChanged();
    void sourceChanged();
    void updateIntervalChanged();
    void displayUnitsChanged();

    void temperatureShownInTooltipChanged();
    void windShownInTooltipChanged();
    void pressureShownInTooltipChanged();
    void humidityShownInTooltipChanged();

    void temperatureShownInCompactModeChanged();

private:
    KUnitConversion::Unit unit(const QString& unit);

private:
    KUnitConversion::Converter m_converter;
    KUnitConversion::Unit m_displayTemperatureUnit;
    KUnitConversion::Unit m_displaySpeedUnit;
    KUnitConversion::Unit m_displayPressureUnit;
    KUnitConversion::Unit m_displayVisibilityUnit;

    int m_updateInterval = 30; // in minutes
    QString m_source;

    bool m_temperatureShownInTooltip = true;
    bool m_windShownInTooltip = false;
    bool m_pressureShownInTooltip = false;
    bool m_humidityShownInTooltip = false;

    bool m_temperatureShownInCompactMode = false;

    bool m_needsToBeSquare = false;
};

#endif
