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

#ifndef WEATHERLOCATION_HEADER
#define WEATHERLOCATION_HEADER

#include <Plasma/DataEngine>

#include "plasmaweather_export.h"

/**
 * @class WeatherLocation <plasmaweather/weatherlocation.h>
 *
 * @short Class to get default source for weather
 */
class PLASMAWEATHER_EXPORT WeatherLocation : public QObject
{
    Q_OBJECT
public:
    WeatherLocation(QObject *parent = 0);
    virtual ~WeatherLocation();

    /**
     * Get default source
     **/
    void getDefault();
    
    /**
     * Sets dataengines to use
     *
     * @param location location dataengine
     * @param weather weather dataengine
     **/
    void setDataEngines(Plasma::DataEngine* location, Plasma::DataEngine* weather);

Q_SIGNALS:
    /**
     * Emitted when validation is done
     **/
    void finished(const QString& source);
    
public Q_SLOTS:
    void dataUpdated(const QString &source, const Plasma::DataEngine::Data &data);

private:
    class Private;
    Private * const d;
};

#endif
