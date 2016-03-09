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


/**
 * @class WeatherPopupApplet <plasmaweather/weatherpopupapplet.h>
 *
 * @short Base class for Weather Applets
 */
class PLASMAWEATHER_EXPORT WeatherPopupApplet : public Plasma::Applet
{
    Q_OBJECT
    Q_PROPERTY(Plasma::DataEngine* weatherDataEngine READ weatherDataEngine NOTIFY weatherDataEngineChanged)

    public:
        WeatherPopupApplet(QObject *parent, const QVariantList &args);
        ~WeatherPopupApplet();

        /**
         * Reimplemented from Plasma::Applet
         */
        virtual void init();

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
         * @return condition icon with guessed value if it was empty
         **/
        QString conditionIcon();

        QString source() const;

        Plasma::DataEngine* weatherDataEngine() const;

        /**
         * Used by QML config code, so check used keys carefully
         * @return currently used config values
         */
        Q_INVOKABLE virtual QVariantMap configValues() const;

    public Q_SLOTS:
        /**
         * Called when config is accepted
         * @param changedConfigValues config values which got changed
         */
        virtual void saveConfig(const QVariantMap& changedConfigValues);

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
        void weatherDataEngineChanged(Plasma::DataEngine* weatherDataEngine);

    private:
        /**
         * Connects applet to dataengine
         */
        void connectToEngine();

    private:
        class Private;
        Private * const d;

        Q_PRIVATE_SLOT(d, void locationReady(const QString &source))
        Q_PRIVATE_SLOT(d, void giveUpBeingBusy())
        Q_PRIVATE_SLOT(d, void onTimeoutNotificationClosed())
};

#endif
