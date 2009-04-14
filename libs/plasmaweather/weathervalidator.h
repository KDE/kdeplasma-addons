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

#ifndef WEATHERVALIDATOR_HEADER
#define WEATHERVALIDATOR_HEADER

#include <Plasma/DataEngine>

#include "plasmaweather_export.h"

/**
 * @class WeatherValidator <plasmaweather/weathervalidator.h>
 *
 * @short Weather validator class
 */
class PLASMAWEATHER_EXPORT WeatherValidator : public QObject
{
    Q_OBJECT
public:
    WeatherValidator(QWidget *parent = 0);
    virtual ~WeatherValidator();

    /**
     * Validate city
     *
     * @param plugin the name of the ion
     * @param city the name of the city to find
     * @param silent if true don't show any dialogs
     **/
    void validate(const QString& plugin, const QString& city, bool silent = false);
    
    /**
     * Sets dataengine to use
     *
     * @param dataengine use this dataengine
     **/
    void setDataEngine(Plasma::DataEngine* dataengine);
    
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
