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

#ifndef WEATHERVALIDATOR_HEADER
#define WEATHERVALIDATOR_HEADER

#include <Plasma/DataEngine>

class WeatherValidator : public QObject
{
    Q_OBJECT
public:
    WeatherValidator(QWidget *parent = 0);
    virtual ~WeatherValidator();

    void validate(const QString& plugin, const QString& citybool, bool silent = false);
    void setDataEngine(Plasma::DataEngine* dataengine);
    
signals:
    void finished(const QString& source);
    
public slots:
    void dataUpdated(const QString &source, const Plasma::DataEngine::Data &data);

private:
    class Private;
    Private * const d;
};

#endif
