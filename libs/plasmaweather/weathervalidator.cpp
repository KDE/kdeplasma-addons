/*
 * Copyright 2009  Petri Damstén <damu@iki.fi>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "weathervalidator.h"
#include <KInputDialog>
#include <KMessageBox>

class WeatherValidator::Private
{
public:
    Private()
    {}
    
    Plasma::DataEngine* dataengine;
    bool silent;
};

WeatherValidator::WeatherValidator(QWidget *parent)
    : QObject(parent)
    , d(new Private())
{
}

WeatherValidator::~WeatherValidator()
{
    delete d;
}

void WeatherValidator::validate(const QString& plugin, const QString& city, bool silent)
{
    d->silent = silent;
    QString validation = QString("%1|validate|%2").arg(plugin).arg(city);
    d->dataengine->connectSource(validation, this);
}

void WeatherValidator::setDataEngine(Plasma::DataEngine* dataengine)
{
    d->dataengine = dataengine;
}

void WeatherValidator::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    d->dataengine->disconnectSource(source, this);
    QStringList result = data["validate"].toString().split('|');
    QString weatherSource;

    if (result[1] == "valid") {
        QMap<QString, QString> places;
        int i = 3;
        while (i < result.count()) {
            if (result[i] == "place") {
                if (i + 2 < result.count() && result[i + 2] == "extra") {
                    places[result[i + 1]] = result[i + 3];
                    i += 4;
                } else {
                    places[result[i + 1]].clear();
                    i += 2;
                }
            }
        }
        QString place;
        if (result[2] == "multiple" && !d->silent) {
            QStringList selected = KInputDialog::getItemList(i18n("Weather station:"),
                    i18n("Found multiple places:"), places.keys());
            if (selected.isEmpty()) {
                return;
            }
            place = selected[0];
        } else {
            place = places.keys()[0];
        }
        weatherSource = QString("%1|weather|%2").arg(result[0]).arg(place);
        if (!places[place].isEmpty()) {
            weatherSource += QString("|%1").arg(places[place]);
        }
    } else if (result[1] == "timeout" && !d->silent) {
        KMessageBox::error(0, i18n("Timeout happened when trying to connect weather server."));
    } else if (!d->silent) {
        KMessageBox::error(0, i18n("Cannot find '%1'.", result[3]));
    }
    emit finished(weatherSource);
}

#include "weathervalidator.moc"
