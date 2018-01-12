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
#include <Plasma/DataEngineConsumer>
#include <Plasma/Applet>

#include "plasmaweather_export.h"

namespace Plasma {

/**
 * @class WeatherPopupApplet <plasmaweather/weatherpopupapplet.h>
 *
 * @short Base class for Weather Applets
 */
class PLASMAWEATHER_EXPORT WeatherPopupApplet : public Plasma::Applet, public Plasma::DataEngineConsumer
{
    Q_OBJECT

    public:
        WeatherPopupApplet(QObject *parent, const QVariantList &args);
        ~WeatherPopupApplet() override;

    public: // Plasma::Applet API
        /**
         * Called once the applet is loaded and added to a Corona.
         */
        void init() override;

        /**
         * Called when config is chnaged
         */
        void configChanged() override;

    public: // API to be implemented by subclasses
        /**
         * Used by QML config code, so check used keys carefully
         * @return currently used config values
         */
        Q_INVOKABLE virtual QVariantMap configValues() const;

    public Q_SLOTS: // API to be implemented by subclasses
        /**
         * Called when config is accepted
         * @param changedConfigValues config values which got changed
         */
        virtual void saveConfig(const QVariantMap& changedConfigValues);

        /**
         * Called when data is updated.
         * Method with this signature expected by connected dataengines.
         * Virtual, so subclasses can override it to also handle the data.
         */
        virtual void dataUpdated(const QString &name,
                                 const Plasma::DataEngine::Data &data);

    public:
        /**
         * @return pressure unit
         **/
        KUnitConversion::Unit pressureUnit() const;

        /**
         * @return temperature unit
         **/
        KUnitConversion::Unit temperatureUnit() const;

        /**
         * @return speed unit
         **/
        KUnitConversion::Unit speedUnit() const;

        /**
         * @return visibility unit
         **/
        KUnitConversion::Unit visibilityUnit() const;

        /**
         * @return condition icon with guessed value if it was empty
         **/
        QString conditionIcon();

        QString source() const;

    private:
        /**
         * Connects applet to dataengine
         */
        void connectToEngine();

    private:
        const QScopedPointer<class WeatherPopupAppletPrivate> d;

        friend class WeatherPopupAppletPrivate;
};

}

#endif
