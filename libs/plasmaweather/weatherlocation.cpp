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
    Private(WeatherLocation *location)
        : q(location),
          locationEngine(0)
    {}

    void validatorFinished(const QMap<QString, QString> &results)
    {
        QString source;
        if (!results.isEmpty()) {
            source = results.begin().value();
        }

        emit q->finished(source);
    }

    WeatherLocation *q;
    Plasma::DataEngine *locationEngine;
    WeatherValidator validator;
};

WeatherLocation::WeatherLocation(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
    Weatheri18nCatalog::loadCatalog();
    QObject::connect(&d->validator, SIGNAL(finished(QMap<QString,QString>)),
                     this, SLOT(validatorFinished(QMap<QString,QString>)));
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
    if (d->locationEngine && d->locationEngine->isValid()) {
        d->locationEngine->connectSource(QLatin1String( "location" ), this);
    } else {
        emit finished(QString());
    }
}

void WeatherLocation::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    if (!d->locationEngine) {
        return;
    }

    d->locationEngine->disconnectSource(source, this);

    QString city = data[QLatin1String( "city" )].toString();

    if (city.contains(QLatin1Char( ',' )))
        city.truncate(city.indexOf(QLatin1Char( ',' )) - 1);

    if (!city.isEmpty()) {
        d->validator.validate(QLatin1String( "bbcukmet" ), city, true);
        return;
    }

    emit finished(QString());
}

#include "weatherlocation.moc"
