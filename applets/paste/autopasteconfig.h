/*
 * Copyright 2008  Petri Damsten <damu@iki.fi>
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

#ifndef AUTOPASTECONFIG_HEADER
#define AUTOPASTECONFIG_HEADER

#include <QStandardItemModel>
#include <QWidget>
#include "ui_autopasteconfig.h"

class ConfigData;

class AutoPasteConfig : public QWidget, public Ui::AutoPasteConfig
{
    Q_OBJECT
public:
    AutoPasteConfig(QWidget *parent = 0);
    virtual ~AutoPasteConfig();

    void getData(ConfigData *data);

public slots:
    void setData(const ConfigData &data);

protected slots:
    void addClicked();
    void removeClicked();
    void editClicked();
    void enableWidgets();

private:
    QStandardItemModel m_appModel;
};

#endif // AUTOPASTECONFIG_HEADER
