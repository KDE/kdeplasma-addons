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

#include "weathervalidator.h"
#include "weatheri18ncatalog.h"
#include <KInputDialog>
#include <KMessageBox>

class WeatherValidator::Private
{
public:
    Private()
        : dataengine(0),
          ion(QLatin1String( "bbcukmet" ))
    {}

    Plasma::DataEngine* dataengine;
    QString ion;
    QString validating;
    bool silent;
};

WeatherValidator::WeatherValidator(QWidget *parent)
    : QObject(parent)
    , d(new Private())
{
    Weatheri18nCatalog::loadCatalog();
}

WeatherValidator::~WeatherValidator()
{
    delete d;
}

QString WeatherValidator::ion() const
{
    return d->ion;
}

void WeatherValidator::setIon(const QString &ion)
{
    d->ion = ion;
}

void WeatherValidator::validate(const QString& location, bool silent)
{
    if (d->ion.isEmpty() || !d->dataengine) {
        return;
    }

    d->silent = silent;
    QString validation = QString(QLatin1String( "%1|validate|%2" )).arg(d->ion).arg(location);
    if (d->validating != validation) {
        d->dataengine->disconnectSource(d->validating, this);
    }

    d->validating = validation;
    d->dataengine->connectSource(validation, this);
}

void WeatherValidator::validate(const QString& ion, const QString& location, bool silent)
{
    d->ion = ion;
    validate(location, silent);
}

void WeatherValidator::setDataEngine(Plasma::DataEngine* dataengine)
{
    d->dataengine = dataengine;
}

void WeatherValidator::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    QMap<QString, QString> locations;
    d->dataengine->disconnectSource(source, this);
    QStringList result = data[QLatin1String( "validate" )].toString().split(QLatin1Char( '|' ));

    if (result.count() < 2) {
        QString message = i18n("Cannot find '%1' using %2.", source, d->ion);
        emit error(message);
        if (!d->silent) {
            KMessageBox::error(0, message);
        }
    } else if (result[1] == QLatin1String( "valid" ) && result.count() > 2) {
        QString weatherSource = result[0] + QLatin1String( "|weather|%1|%2" );
        QString singleWeatherSource = result[0] + QLatin1String( "|weather|%1" );
        int i = 3;
        //kDebug() << d->ion << result.count() << result;
        while (i < result.count() - 1) {
            if (result[i] == QLatin1String( "place" )) {
                if (i + 1 > result.count()) {
                    continue;
                }

                QString name = result[i + 1];
                if (i + 2 < result.count() && result[i + 2] == QLatin1String( "extra" )) {
                    QString id = result[i + 3];
                    locations.insert(name, weatherSource.arg(name, id));
                    i += 4;
                } else {
                    locations.insert(name, singleWeatherSource.arg(name));
                    i += 2;
                }
            } else {
                ++i;
            }
        }

    } else if (result[1] == QLatin1String( "timeout" )) {
        QString message = i18n("Connection to %1 weather server timed out.", d->ion);
        emit error(message);
        if (!d->silent) {
            KMessageBox::error(0, message);
        }
    } else {
        QString message = i18n("Cannot find '%1' using %2.", result.count() > 3 ? result[3] : source, d->ion);
        emit error(message);
        if (!d->silent) {
            KMessageBox::error(0, message);
        }
    }

    emit finished(locations);
}

#include "weathervalidator.moc"
