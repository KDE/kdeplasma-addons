/*
 * Copyright 2008-2009  Petri Damstén <damu@iki.fi>
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

#include <Plasma/PopupApplet>
#include <Plasma/DataEngine>
#include <conversion/value.h>
#include <plasmaweather/weatherpopupapplet.h>
#include "ui_appearanceconfig.h"

class LCD;
class QGraphicsLinearLayout;
namespace Conversion { class Value; }

class WeatherStation : public WeatherPopupApplet
{
    Q_OBJECT
    public:
        WeatherStation(QObject *parent, const QVariantList &args);
        ~WeatherStation();

        virtual void init();
        virtual void createConfigurationInterface(KConfigDialog *parent);
        virtual QGraphicsWidget *graphicsWidget();

    public slots:
        virtual void configAccepted();
        virtual void dataUpdated(const QString &name, const Plasma::DataEngine::Data &data);
        void clicked(const QString &name);

    protected:
        void setLCDIcon();
        void setBackground();

        void setWind(const Conversion::Value& speed, const QString& direction);
        void setPressure(const QString& condition, const Conversion::Value& pressure,
                         const QString& tendency);
        void setTemperature(const Conversion::Value& temperature);
        void setHumidity(QString humidity);

        QString fitValue(const Conversion::Value& value, int digits);
        QStringList fromCondition(const QString& condition);
        Conversion::Value value(const QString& value, const QString& unit);

    private:
        LCD *m_lcd;
        LCD *m_lcdPanel;
        Ui::AppearanceConfig m_appearanceConfig;
        bool m_useBackground;
        bool m_showToolTip;
        QString m_url;
};

K_EXPORT_PLASMA_APPLET(weatherstation, WeatherStation)

#endif
