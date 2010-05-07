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
     * Validate location 
     *
     * @param plugin the name of the ion
     * @param location the name of the location to find
     * @param silent if true don't show any dialogs
     **/
    void validate(const QString& plugin, const QString& location, bool silent = true);

    /**
     * Validate location; plugin must already have been set
     *
     * @param location the name of the location to find
     * @param silent if true don't show any dialogs
     */
    void validate(const QString& location, bool silent = false);

    /**
     * Sets the ion to use
     * @arg plugin the name of the ion
     */
    void setIon(const QString &plugin);

    /**
     * @return the ion currently set to validate with
     */
    QString ion() const;

    /**
     * Sets dataengine to use
     *
     * @param dataengine use this dataengine
     **/
    void setDataEngine(Plasma::DataEngine* dataengine);

Q_SIGNALS:
    /**
     * Emitted when an error in validation occurs
     **/
    void error(const QString& message);

    /**
     * Emitted when validation is done
     * @arg sources a mapping of user-friendly names to the DataEngine source
     **/
    void finished(const QMap<QString, QString> &sources);

public Q_SLOTS:
    void dataUpdated(const QString &source, const Plasma::DataEngine::Data &data);

private:
    class Private;
    Private * const d;
};

#endif
