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

#ifndef WEATHERCONFIG_HEADER
#define WEATHERCONFIG_HEADER

#include <Plasma/DataEngine>
#include <KDialog>
#include <QWidget>
#include "ui_weatherconfig.h"
#include "ui_weatherconfigsearch.h"

class WeatherConfigSearch : public KDialog, public Ui::WeatherConfigSearch
{
    Q_OBJECT
public:
    WeatherConfigSearch(QWidget *parent = 0);
    virtual ~WeatherConfigSearch();

    void setDataEngine(Plasma::DataEngine* dataengine);
    void setSource(const QString& source);
    QString source();
    QString nameForPlugin(const QString& plugin);

public slots:
    void dataUpdated(const QString &source, const Plasma::DataEngine::Data &data);

protected slots:
    void searchPressed();
    void textChanged(const QString& txt);

private:
    Plasma::DataEngine* m_dataengine;
    QString m_source;
};

class WeatherConfig : public QWidget, public Ui::WeatherConfig
{
    Q_OBJECT
public:
    WeatherConfig(QWidget *parent = 0);
    virtual ~WeatherConfig() {};

    void setDataEngine(Plasma::DataEngine* dataengine);
    void setSource(const QString& source);
    void setUpdateInterval(int interval);
    void setTemperatureUnit(const QString& unit);
    void setPressureUnit(const QString& unit);
    void setSpeedUnit(const QString& unit);

    QString source();
    int updateInterval();
    QString temperatureUnit();
    QString pressureUnit();
    QString speedUnit();

protected slots:
    void changePressed();

private:
    WeatherConfigSearch m_searchDlg;
    QString m_source;
};

#endif // WEATHERCONFIG_HEADER
