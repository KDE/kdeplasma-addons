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

class PLASMAWEATHER_EXPORT WeatherPopupApplet : public Plasma::PopupApplet
{
    Q_OBJECT
    public:
        WeatherPopupApplet(QObject *parent, const QVariantList &args);
        ~WeatherPopupApplet();

        virtual void init();
        virtual void createConfigurationInterface(KConfigDialog *parent);

        QString pressureUnit();
        QString temperatureUnit();
        QString speedUnit();
        QString visibilityUnit();

        int pressureUnitInt();
        int temperatureUnitInt();
        int speedUnitInt();
        int visibilityUnitInt();

        QString conditionIcon();

        WeatherConfig* weatherConfig();

    public Q_SLOTS:
        virtual void configAccepted();
        virtual void dataUpdated(const QString &name,
                                 const Plasma::DataEngine::Data &data);
    
    protected:
        virtual void connectToEngine();

    private:
        class Private;
        Private * const d;
        
        Q_PRIVATE_SLOT(d, void validationFinished(const QString &source))
};

#endif
