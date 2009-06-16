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

#include "weatherlocation.h"
#include "weathervalidator.h"
#include "weatheri18ncatalog.h"

class WeatherLocation::Private
{
public:
    Private()
    {}
    Plasma::DataEngine *locationEngine;
    WeatherValidator validator;
};

WeatherLocation::WeatherLocation(QObject *parent)
    : QObject(parent)
    , d(new Private())
{
    Weatheri18nCatalog::loadCatalog();
    QObject::connect(&d->validator, SIGNAL(finished(const QString&)),
                     this, SIGNAL(finished(const QString&)));
}

WeatherLocation::~WeatherLocation()
{
    delete d;
}

void WeatherLocation::setDataEngines(Plasma::DataEngine* location, Plasma::DataEngine* weather)
{
    d->locationEngine = location;
    d->validator.setDataEngine(weather);
}

void WeatherLocation::getDefault()
{
    if (d->locationEngine->isValid()) {
        d->locationEngine->connectSource("location", this);
    } else {
        emit finished(QString());
    }
}

void WeatherLocation::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    d->locationEngine->disconnectSource(source, this);

    QString city = data["city"].toString();
    city.truncate(city.indexOf(',') - 1);
    if (!city.isEmpty()) {
        d->validator.validate("bbcukmet", city, true);
        return;
    }

    emit finished(QString());
}

#include "weatherlocation.moc"
