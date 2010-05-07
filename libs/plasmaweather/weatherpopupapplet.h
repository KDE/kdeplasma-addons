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
#include <Plasma/PopupApplet>
#include <Plasma/DataEngine>

#include "plasmaweather_export.h"

class WeatherConfig;

/**
 * @class WeatherPopupApplet <plasmaweather/weatherpopupapplet.h>
 *
 * @short Base class for Weather Applets
 */
class PLASMAWEATHER_EXPORT WeatherPopupApplet : public Plasma::PopupApplet
{
    Q_OBJECT
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
        virtual void createConfigurationInterface(KConfigDialog *parent);

        /**
         * @return pressure unit
         **/
        KUnitConversion::UnitPtr pressureUnit();

        /**
         * @return temperature unit
         **/
        KUnitConversion::UnitPtr temperatureUnit();

        /**
         * @return speed unit
         **/
        KUnitConversion::UnitPtr speedUnit();

        /**
         * @return visibility unit
         **/
        KUnitConversion::UnitPtr visibilityUnit();

        /**
         * @return condition icon with guessed value if it was empty
         **/
        QString conditionIcon();

        /**
         * @return weather config dialog widget
         **/
        WeatherConfig* weatherConfig();

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
