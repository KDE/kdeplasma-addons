/***************************************************************************
 *   Copyright (C) 2007-2008 by Shawn Starr <shawn.starr@rogers.com>       *
 *                      2008 by Petri Damsten <damu@iki.fi>                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef WEATHERCONFIG_HEADER
#define WEATHERCONFIG_HEADER

#include <Plasma/DataEngine>
#include <QWidget>
#include <QStandardItemModel>
#include "ui_weatherConfig.h"
#include "ui_weatherAddPlace.h"

class KDialog;

class ConfigData
{
    public:
        enum WindFormat { Kilometers = 0, MetersPerSecond, Miles, Knots, Beaufort };

        ConfigData() : weatherUpdateTime(30), weatherWindFormat(0), weatherEngine(0) {};

        struct PlaceInfo {
            QString place;
            QString ion;
        };
        QMap<QString, PlaceInfo> placeList;
        QMap<QString, QString> extraData;
        Plasma::DataEngine::Data ionPlugins;
        int weatherUpdateTime;
        int weatherWindFormat;
        Plasma::DataEngine *weatherEngine;
};

class WeatherConfig : public QWidget, public Ui::weatherConfig
{
    Q_OBJECT
public:
    WeatherConfig(QWidget *parent = 0);
    virtual ~WeatherConfig() {};

    void setData(const ConfigData& data);
    void getData(ConfigData* data);

public slots:
    void dataUpdated(const QString &name, const Plasma::DataEngine::Data &data);
    void pluginChanged(int idx);

protected:
    void validate(const QString& source, const QVariant& data);
    void showAddPlaceDialog(const QStringList& tokens);
    

protected slots:
    void removePlace();
    void placeEditChanged(const QString& text);
    void getValidation();
    void activeTreeItem(const QModelIndex& index);
    void addPlace();
    void cancelAddClicked();
    void selectPlace();

private:
    QStandardItemModel *m_cmodel;
    QStandardItemModel *m_amodel;
    QList<QStandardItem *> m_items;
    QStandardItem *m_selectedItem;
    QString m_activeValidation;
    KDialog *m_addDialog;
    Ui::weatherAddPlace aui;

    ConfigData c;
};

#endif // WEATHERCONFIG_HEADER
