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

#ifndef WEATHERCONFIGSEARCH_HEADER
#define WEATHERCONFIGSEARCH_HEADER

#include <Plasma/DataEngine>
#include <KDialog>
#include "ui_weatherconfigsearch.h"
#include "weathervalidator.h"

class WeatherConfigSearch : public KDialog, public Ui::WeatherConfigSearch
{
    Q_OBJECT
public:
    WeatherConfigSearch(QWidget *parent = 0);
    virtual ~WeatherConfigSearch();

    void setDataEngine(Plasma::DataEngine* dataengine);
    void setSource(const QString& source);
    QString source() const;
    QString nameForPlugin(const QString& plugin);

public slots:
    void finished(const QString &source);

protected slots:
    void searchPressed();
    void textChanged(const QString& txt);

private:
    Plasma::DataEngine* m_dataengine;
    WeatherValidator m_validator;
    QString m_source;
};

#endif
