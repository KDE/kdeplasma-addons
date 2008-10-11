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

#include "weatherconfig.h"
#include <KMessageBox>
#include <QHeaderView>

WeatherConfig::WeatherConfig(QWidget *parent)
    : QWidget(parent),
      m_cmodel(0),
      m_amodel(0),
      m_selectedItem(0),
      m_addDialog(0)
{
    setupUi(this);

    // Set up QTreeView with model/view
    m_cmodel = new QStandardItemModel();

    weathersourceTree->setModel(m_cmodel);
    weathersourceTree->show();

    pluginComboList->setEditable(false);

    removeButton->setEnabled(false);
    validateButton->setEnabled(false);

    windOptionsComboList->addItem(i18nc("Units of wind speed", "Kilometers Per Hour"), 0);
    windOptionsComboList->addItem(i18nc("Units of wind speed", "Metres Per Second"), 1);
    windOptionsComboList->addItem(i18nc("Units of wind speed", "Miles Per Hour"), 2);
    windOptionsComboList->addItem(i18nc("Units of wind speed", "Knots"), 3);
    windOptionsComboList->addItem(i18nc("Units of wind speed", "Beaufort Scale"), 4);
    //windOptionsComboList->model()->sort(0, Qt::AscendingOrder);
    windOptionsComboList->setCurrentIndex(0);

    locationEdit->setTrapReturnKey(true);

    connect(validateButton, SIGNAL(clicked()), this, SLOT(getValidation()));
    connect(removeButton, SIGNAL(clicked()), this, SLOT(removePlace()));
    connect(locationEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(placeEditChanged(const QString &)));
    connect(locationEdit, SIGNAL(returnPressed()), this, SLOT(getValidation()));
    connect(weathersourceTree->selectionModel(),
            SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)),
            this, SLOT(activeTreeItem(const QModelIndex &)));

    pluginComboList->setCurrentIndex(0);
    locationEdit->clear();
}

void WeatherConfig::pluginChanged(int idx)
{
    while(m_cmodel->rowCount() > 0)
        m_cmodel->removeRows(0, 1);
        
    QString comboData = pluginComboList->itemData(idx).toString();
    c.weatherEngine->connectSource(QString("%1|list").arg(comboData), this);
}

void WeatherConfig::setData(const ConfigData& data)
{
    c = data;

    QStringList places;
    QStringList pluginInfo;

    QStringList headers;
    headers << i18n("Data Source") << i18n("Place") << i18n("Plugin");
    m_cmodel->clear();
    m_cmodel->setHorizontalHeaderLabels(headers);
    m_cmodel->setColumnCount(3);
    weathersourceTree->header()->setResizeMode(QHeaderView::ResizeToContents);

    m_items.clear();
    foreach(const ConfigData::PlaceInfo& info, c.placeList) {
        m_items.append(new QStandardItem(c.ionPlugins[info.ion].toString().split("|")[0]));
        m_items.append(new QStandardItem(info.place));
        m_items.append(new QStandardItem(info.ion));
        m_cmodel->appendRow(m_items);
    }

    pluginComboList->clear();
    foreach(const QVariant &item, c.ionPlugins) {
        QStringList pluginInfo = item.toString().split("|");
        if (pluginInfo.count() > 1) {
            pluginComboList->addItem(pluginInfo[0], pluginInfo[1]);
        }
    }
    pluginComboList->model()->sort(0, Qt::AscendingOrder);
    pluginComboList->setCurrentIndex(0);
    
    // the list is now full, let's connect to see if it changes..
    connect(pluginComboList, SIGNAL(activated(int)), this, SLOT(pluginChanged(int)));

    weatherUpdateSpin->setValue(c.weatherUpdateTime);
    windOptionsComboList->setCurrentIndex(0);
    for (int i = 0; i < windOptionsComboList->count(); ++i) {
        if (data.weatherWindFormat == windOptionsComboList->itemData(i).toInt()) {
            windOptionsComboList->setCurrentIndex(i);
            break;
        }
    }
}

void WeatherConfig::getData(ConfigData* data)
{
    ConfigData::PlaceInfo placeInfo;
    QString pluginName;
    QString placeName;

    data->placeList.clear();
    for (int row = 0; row < m_cmodel->rowCount(); row++) {
        for (int column = m_cmodel->columnCount() - 1; column >= 1; column--) {
            QModelIndex index = m_cmodel->index(row, column);
            QVariant item = m_cmodel->data(index, Qt::DisplayRole);
            if (column % 2) {
                placeName = item.toString();
            } else {
                pluginName = item.toString();
            }
        }
        placeName[0] = placeName[0].toUpper();
        placeInfo.place = placeName;
        placeInfo.ion = pluginName;
        data->placeList[placeName] = placeInfo;
    }
    data->extraData = c.extraData;

    data->weatherWindFormat = windOptionsComboList->itemData(
            windOptionsComboList->currentIndex()).toInt();
    data->weatherUpdateTime = weatherUpdateSpin->value();
}

void WeatherConfig::getValidation()
{
    if (locationEdit->text().size() >= 3 && m_cmodel->rowCount() < 1) {
        QString comboData = pluginComboList->itemData(pluginComboList->currentIndex()).toString();
        QString locPlace = locationEdit->text();
        locPlace[0] = locPlace[0].toUpper();
        c.weatherEngine->connectSource(QString("%1|validate|%2").arg(comboData).arg(locPlace), this);
    }
}

void WeatherConfig::removePlace()
{
    if (m_cmodel->rowCount()) {

        QModelIndex index_plugin = m_cmodel->index(m_selectedItem->row(), 2);
        QModelIndex index_place = m_cmodel->index(m_selectedItem->row(), 1);
        QString itemplugin = m_cmodel->data(index_plugin, Qt::DisplayRole).toString();
        QString itemplace = m_cmodel->data(index_place, Qt::DisplayRole).toString();
        itemplace[0] = itemplace[0].toUpper();

        // Remove the source validation and or weather if found it's safe to disconnect it even if its not there.
        c.weatherEngine->disconnectSource(QString("%1|validate|%2").arg(itemplugin).arg(itemplace), this);
        c.weatherEngine->disconnectSource(QString("%1|weather|%2").arg(itemplugin).arg(itemplace), this);
        if (!c.extraData[itemplace].isEmpty()) {
            c.weatherEngine->disconnectSource(QString("%1|weather|%2|%3").arg(itemplugin).arg(itemplace).arg(c.extraData[itemplace]), this);
        }
        if (!m_activeValidation.isEmpty()) {
            c.weatherEngine->disconnectSource(m_activeValidation, this);
        }
        kDebug() << "DANGLING SOURCES WHEN REMOVING ITEM: " << c.weatherEngine->sources();

        m_cmodel->removeRow(m_selectedItem->row());
        if (!m_cmodel->rowCount()) {
            removeButton->setEnabled(false);
            if (locationEdit->text().size() >= 3) {
                validateButton->setEnabled(true);
            }
        }

        foreach(const ConfigData::PlaceInfo& info, c.placeList) {
            if (info.place == itemplace) {
                c.placeList.remove(info.place);
                break;
            }
        }
    }

    if (c.placeList.isEmpty()) {
        windOptionsComboList->setCurrentIndex(0);
    }
    //m_weatherWindFormat = windOptionsComboList->itemData(windOptionsComboList->currentIndex()).toInt();
    return;
}

void WeatherConfig::placeEditChanged(const QString& text)
{
    if (text.size() < 3) {
        validateButton->setEnabled(false);
    } else {
        if (m_cmodel->rowCount() < 1) {
            validateButton->setEnabled(true);
        }
    }
}

void WeatherConfig::activeTreeItem(const QModelIndex& index)
{
    m_selectedItem = m_cmodel->itemFromIndex(index);
    weathersourceTree->selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
    removeButton->setEnabled(true);
    return;
}

void WeatherConfig::dataUpdated(const QString &name, const Plasma::DataEngine::Data &data)
{
    // Check if we validate or not
    QString location = name;
    location[0] = location[0].toUpper();
    if (data.value("validate").toBool()) {
        if (data["validate"].type() == QVariant::String) {
            validate(location, data["validate"]);
        }
    }
}

void WeatherConfig::validate(const QString& source, const QVariant& data)
{
    QStringList tokens = data.toString().split("|");
    QString location = source;
    bool extraflag = false;
    location[0] = location[0].toUpper();
    kDebug() << "*** DANGLING SOURCES BEFORE ***"  << c.weatherEngine->sources();

    // If the place is valid, check if there is one place or multiple places returned. The user will have
    // to select the place that best matches what they are looking for.
    if (tokens[1] == QString("valid")) {

        // Plugin returns only one matching place
        if (tokens[2] == QString("single")) {
            m_activeValidation = location;
            m_items.clear();

            foreach (const QString& val, tokens) {
            if (val.contains("extra")) {
                extraflag = true;
            continue;
            }

            if (extraflag) {
                c.extraData[tokens[4]] = val;
                break;
            }
        }

            QList<QStandardItem *> foundDuplicate = m_cmodel->findItems(tokens[4], Qt::MatchExactly, 1);
            if (foundDuplicate.count() < 1) {
                m_items.append(new QStandardItem(pluginComboList->currentText()));
                m_items.append(new QStandardItem(tokens[4]));
                m_items.append(new QStandardItem(pluginComboList->itemData(pluginComboList->currentIndex()).toString()));
                m_cmodel->appendRow(m_items);
                // TODO: Disable search button once a place has been added, remove this for KDE 4.1
                validateButton->setEnabled(false);
            }
        }

        if (tokens[2] == QString("multiple")) {
            m_activeValidation = location;

            // Extra metadata from ion - store it
            bool placeflag = false;
            QString place;
            foreach(const QString& item, tokens) {
                if (!item.isEmpty()) {
                    if (item.contains("place")) {
                        placeflag = true;
                        continue;
                    }

                    if (placeflag) {
                        place = item;
                        placeflag = false;
                    }

                    if (item.contains("extra")) {
                        extraflag = true;
                        continue;
                    }

                    if (extraflag) {
                        extraflag = false;
                        c.extraData[place] = item;
                    }
                }
            }
            kDebug() << tokens;
            // Pop up dialog and allow user to choose places
            showAddPlaceDialog(tokens);
            return;
        }

    } else if (tokens[1] == QString("timeout")) {
        c.weatherEngine->disconnectSource(location, this);
        KMessageBox::error(0, i18n("The applet was not able to contact the server, please try again later"));
        kDebug() << "*** DANGLING SOURCES AFTER ***"  << c.weatherEngine->sources();
        return;
    } else {
        c.weatherEngine->disconnectSource(location, this);
        KMessageBox::error(0, i18n("The place '%1' is not valid. The data source is not able to find this place.", tokens[3]), i18n("Invalid Place"));
        kDebug() << "*** DANGLING SOURCES AFTER ***"  << c.weatherEngine->sources();
        return;
    }

}

void WeatherConfig::showAddPlaceDialog(const QStringList& tokens)
{
    if (m_addDialog == 0) {
        m_addDialog = new KDialog;
        aui.setupUi(m_addDialog->mainWidget());
        m_addDialog->mainWidget()->layout()->setMargin(0);

        // Set up QListView with model/view
        m_amodel = new QStandardItemModel();
        aui.foundPlacesTreeView->setModel(m_amodel);
        // TODO: For now, force the treeview to be single selection until KDE 4.1
        aui.foundPlacesTreeView->setSelectionMode(QAbstractItemView::SingleSelection);

        // end note
        aui.foundPlacesTreeView->show();

        m_addDialog->setCaption(i18n("Found Places"));
        m_addDialog->setButtons(KDialog::Ok | KDialog::Cancel);
        m_addDialog->setButtonText(KDialog::Ok, i18n("&Add"));
        m_addDialog->setDefaultButton(KDialog::NoDefault);


        connect(m_addDialog, SIGNAL(okClicked()), this, SLOT(addPlace()));
        connect(m_addDialog, SIGNAL(cancelClicked()), this, SLOT(cancelAddClicked()));
        connect(aui.foundPlacesTreeView, SIGNAL(doubleClicked (const QModelIndex &)),
                this , SLOT(selectPlace()));
        //connect( afoundPlaceslistView->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(activeTreeItem(const QModelIndex &)) );
    }
    bool placeflag = false;
    QStringList headers;
    m_amodel->clear();
    headers << i18n("Found Places");
    m_amodel->setHorizontalHeaderLabels(headers);
    m_amodel->setColumnCount(1);
    aui.foundPlacesTreeView->header()->setResizeMode(QHeaderView::ResizeToContents);

    foreach(const QString &item, tokens) {
        if (item.contains("place")) {
            placeflag = true;
            continue;
        }
        if (placeflag) {
            m_items.clear();
            m_items.append(new QStandardItem(item));
            m_amodel->appendRow(m_items);
            placeflag = false;
        }
    }
    KDialog::centerOnScreen(m_addDialog);
    m_addDialog->show();
}

void WeatherConfig::addPlace()
{
    QString pluginName = pluginComboList->itemData(pluginComboList->currentIndex()).toString();
    QList<QStandardItem *> foundDuplicate;

    QModelIndex item = aui.foundPlacesTreeView->currentIndex();
    foundDuplicate = m_cmodel->findItems(item.data().toString(), Qt::MatchExactly, 1);
    if (foundDuplicate.count() < 1) {
        m_items.clear();
        m_items.append(new QStandardItem(pluginComboList->currentText()));
        m_items.append(new QStandardItem(item.data().toString()));
        m_items.append(new QStandardItem(pluginName));
        m_cmodel->appendRow(m_items);

        // TODO: Disable search button for now, til we support multiple selections officially.
        validateButton->setEnabled(false);
    }

    /* TODO: Turn this on once we support multiple selections per applet for KDE 4.1
       foreach (QModelIndex item, aui.foundPlacesTreeView->selectionModel()->selectedIndexes()) {
          foundDuplicate = m_cmodel->findItems(item.data().toString(), Qt::MatchExactly, 1);
          if (foundDuplicate.count() < 1) {
              m_items.clear();
              m_items.append(new QStandardItem(ui.pluginComboList->currentText()));
              m_items.append(new QStandardItem(item.data().toString()));
              m_items.append(new QStandardItem(pluginName));
              m_cmodel->appendRow(m_items);
          }
       foundDuplicate.clear();
       }

    */
}

void WeatherConfig::cancelAddClicked()
{
    c.weatherEngine->disconnectSource(m_activeValidation, this);
}

void WeatherConfig::selectPlace()
{
   addPlace();
   //until 4.1 when we can select multi station
   m_addDialog->close();
}

#include "weatherconfig.moc"
