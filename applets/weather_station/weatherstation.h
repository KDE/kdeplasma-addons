/*
 * Copyright 2008  Petri Damsten <damu@iki.fi>
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

#ifndef WEATHERSTATION_HEADER
#define WEATHERSTATION_HEADER

#include "weatherconfig.h"
#include <Plasma/PopupApplet>
#include <Plasma/DataEngine>
#include <plasma/weather/weatherutils.h>

class LCD;
class QGraphicsLinearLayout;
class WeatherConfig;

class WeatherStation : public Plasma::PopupApplet
{
    Q_OBJECT
    public:
        WeatherStation(QObject *parent, const QVariantList &args);
        ~WeatherStation();

        virtual void init();
        virtual void createConfigurationInterface(KConfigDialog *parent);
        virtual QGraphicsWidget *graphicsWidget();

    public slots:
        void configAccepted();
        void dataUpdated(const QString &name, const Plasma::DataEngine::Data &data);

    protected:
        void connectToEngine();
        void setLCDIcon();

        void setWind(const QString& speed, int unit, const QString& direction);
        void setPressure(const QString& pressure, int unit, const QString& tendency);
        void setTemperature(const QString& temperature, int unit);
        void setHumidity(QString humidity);

        QString getUnitString(int unit);

    private:
        LCD *m_lcd;
        LCD *m_lcdPanel;
        WeatherConfig *m_weatherConfig;
        ConfigData c;
};

K_EXPORT_PLASMA_APPLET(weatherstation, WeatherStation)

#endif
