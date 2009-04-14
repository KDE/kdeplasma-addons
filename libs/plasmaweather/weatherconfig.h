/*
 * Copyright 2009  Petri Damstén <damu@iki.fi>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef WEATHERCONFIG_HEADER
#define WEATHERCONFIG_HEADER

#include <QWidget>

#include "plasmaweather_export.h"

namespace Plasma { class DataEngine; }

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

    void setConfigurableUnits(const ConfigurableUnits units);
    
    void setDataEngine(Plasma::DataEngine* dataengine);
    void setSource(const QString& source);
    void setTemperatureUnit(const QString& unit);
    void setPressureUnit(const QString& unit);
    void setSpeedUnit(const QString& unit);
    void setVisibilityUnit(const QString& unit);

    QString source();
    int updateInterval();
    QString temperatureUnit();
    QString pressureUnit();
    QString speedUnit();
    QString visibilityUnit();

public Q_SLOTS:
    void setUpdateInterval(int interval);

private:
    class Private;
    Private * const d;
    
    Q_PRIVATE_SLOT(d, void changePressed())
};

Q_DECLARE_OPERATORS_FOR_FLAGS(WeatherConfig::ConfigurableUnits)

#endif
