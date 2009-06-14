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

#ifndef WEATHERCONFIG_HEADER
#define WEATHERCONFIG_HEADER

#include <QWidget>

#include "plasmaweather_export.h"

namespace Plasma { class DataEngine; }

/**
 * @class WeatherConfig <plasmaweather/weatherconfig.h>
 *
 * @short Weather config widget class
 */
class PLASMAWEATHER_EXPORT WeatherConfig : public QWidget
{
    Q_OBJECT
public:
    enum ConfigurableUnit {
        None = 0,
        Temperature = 1,
        Pressure = 2,
        Speed = 4,
        Visibility = 8
    };
    Q_DECLARE_FLAGS(ConfigurableUnits, ConfigurableUnit)
    
    WeatherConfig(QWidget *parent = 0);
    virtual ~WeatherConfig();

    /**
     * Set units that are configurable. Default to all.
     **/
    void setConfigurableUnits(const ConfigurableUnits units);
    
    /**
     * Sets dataengine to use
     **/
    void setDataEngine(Plasma::DataEngine* dataengine);
    
    /**
     * Sets ion source
     **/
    void setSource(const QString& source);

    /**
     * Sets temperature unit
     **/
    void setTemperatureUnit(const QString& unit);

    /**
     * Sets pressure unit
     **/
    void setPressureUnit(const QString& unit);

    /**
     * Sets speed unit
     **/
    void setSpeedUnit(const QString& unit);

    /**
     * Sets visibility unit
     **/
    void setVisibilityUnit(const QString& unit);

    /**
     * @return ion source to use
     **/
    QString source();
    
    /**
     * @return update interval
     **/
    int updateInterval();
    
    /**
     * @return temperature unit
     **/
    QString temperatureUnit();

    /**
     * @return pressure unit
     **/
    QString pressureUnit();

    /**
     * @return speed unit
     **/
    QString speedUnit();

    /**
     * @return visibility unit
     **/
    QString visibilityUnit();

    /**
     * Sets header labels visible/hidden
     **/
    void setHeadersVisible(bool visible);

  signals:
    void settingsChanged();
    
public Q_SLOTS:
    /**
     * Sets update interval
     **/
    void setUpdateInterval(int interval);

private:
    class Private;
    Private * const d;
    
    Q_PRIVATE_SLOT(d, void changePressed())
};

Q_DECLARE_OPERATORS_FOR_FLAGS(WeatherConfig::ConfigurableUnits)

#endif
