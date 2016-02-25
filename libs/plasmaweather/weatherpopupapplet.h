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

#ifndef WEATHERPOPUPAPPLET_HEADER
#define WEATHERPOPUPAPPLET_HEADER

#include <KUnitConversion/Unit>
#include <Plasma/DataEngine>
#include <Plasma/Applet>

#include "plasmaweather_export.h"

class WeatherConfig;

/**
 * @class WeatherPopupApplet <plasmaweather/weatherpopupapplet.h>
 *
 * @short Base class for Weather Applets
 */
class PLASMAWEATHER_EXPORT WeatherPopupApplet : public Plasma::Applet
{
    Q_OBJECT
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged);
    Q_PROPERTY(int updateInterval READ updateInterval WRITE setUpdateInterval NOTIFY updateIntervalChanged);
    Q_PROPERTY(int temperatureUnitId READ temperatureUnitId WRITE setTemperatureUnitId NOTIFY temperatureUnitIdChanged);
    Q_PROPERTY(int pressureUnitId READ pressureUnitId WRITE setPressureUnitId NOTIFY pressureUnitIdChanged);
    Q_PROPERTY(int windSpeedUnitId READ windSpeedUnitId WRITE setWindSpeedUnitId NOTIFY windSpeedUnitIdChanged);
    Q_PROPERTY(int visibilityUnitId READ visibilityUnitId WRITE setVisibilityUnitId NOTIFY visibilityUnitIdChanged);
    Q_PROPERTY(Plasma::DataEngine* weatherDataEngine READ weatherDataEngine NOTIFY weatherDataEngineChanged)

    public:
        WeatherPopupApplet(QObject *parent, const QVariantList &args);
        ~WeatherPopupApplet();

        /**
         * Reimplemented from Plasma::Applet
         */
        virtual void init();

        /**
         * Reimplemented from Plasma::Applet
         */
//         virtual void createConfigurationInterface(KConfigDialog *parent);

    public:
        /**
         * @return pressure unit
         **/
        KUnitConversion::Unit pressureUnit();

        /**
         * @return temperature unit
         **/
        KUnitConversion::Unit temperatureUnit();

        /**
         * @return speed unit
         **/
        KUnitConversion::Unit speedUnit();

        /**
         * @return visibility unit
         **/
        KUnitConversion::Unit visibilityUnit();

        /**
         * @return update interval
         **/
        int updateInterval() const;

        /**
         * @return condition icon with guessed value if it was empty
         **/
        QString conditionIcon();

        /**
         * @return weather config dialog widget
         **/
        WeatherConfig* weatherConfig();

        QString source() const;
        void setSource(const QString &source);

        /**
         * Sets update interval
         **/
        void setUpdateInterval(int updateInterval);

        int temperatureUnitId() const;
        void setTemperatureUnitId(int temperatureUnitId);

        int pressureUnitId() const;
        void setPressureUnitId(int pressureUnitId);

        int windSpeedUnitId() const;
        void setWindSpeedUnitId(int windSpeedUnitId);

        int visibilityUnitId() const;
        void setVisibilityUnitId(int visibilityUnitId);

        Plasma::DataEngine* weatherDataEngine() const;

    public Q_SLOTS:
        /**
         * Called when config is accepted
         */
        virtual void configAccepted();

        /**
         * Called when data is updated
         */
        virtual void dataUpdated(const QString &name,
                                 const Plasma::DataEngine::Data &data);

        /**
         * Called when config is chnaged
         */
        virtual void configChanged();

    Q_SIGNALS:
        /**
         * Emitted when the applet begins a fetch for a new weather source
         */
        void newWeatherSource();

        void sourceChanged(const QString &source);
        void updateIntervalChanged(int updateInterval);
        void temperatureUnitIdChanged(int temperatureUnitId);
        void pressureUnitIdChanged(int pressureUnitId);
        void windSpeedUnitIdChanged(int windSpeedUnitId);
        void visibilityUnitIdChanged(int visibilityUnitId);

        void weatherDataEngineChanged(Plasma::DataEngine* weatherDataEngine);

    protected:
        /**
         * Connects applet to dataengine
         */
        virtual void connectToEngine();

    private:
        class Private;
        Private * const d;

        Q_PRIVATE_SLOT(d, void locationReady(const QString &source))
        Q_PRIVATE_SLOT(d, void giveUpBeingBusy())
};

#endif
