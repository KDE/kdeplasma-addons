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
        QString pressureUnit();

        /**
         * @return temperature unit
         **/
        QString temperatureUnit();

        /**
         * @return speed unit
         **/
        QString speedUnit();

        /**
         * @return visibility unit
         **/
        QString visibilityUnit();

        /**
         * @return pressure unit
         **/
        int pressureUnitInt();

        /**
         * @return temperature unit
         **/
        int temperatureUnitInt();

        /**
         * @return speed unit
         **/
        int speedUnitInt();

        /**
         * @return visibility unit
         **/
        int visibilityUnitInt();

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
