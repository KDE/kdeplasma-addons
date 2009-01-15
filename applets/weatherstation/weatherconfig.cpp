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

#include "weatherconfig.h"
#include <KMessageBox>

WeatherConfigList::WeatherConfigList(QWidget *parent)
    : KDialog(parent)
{
    setupUi(mainWidget());
    connect(this, SIGNAL(okClicked()), this, SLOT(okPressed()));
}

WeatherConfigList::~WeatherConfigList()
{
}

void WeatherConfigList::setList(const QStringList& values)
{
    list->clear();
    list->addItems(values);
}

void WeatherConfigList::okPressed()
{
    QListWidgetItem* item = list->currentItem();
    if (!item) {
        KMessageBox::error(this, i18n("Please select one place."));
    } else {
        accept();
    }
}

QString WeatherConfigList::selected()
{
    QListWidgetItem* item = list->currentItem();
    if (item) {
        return item->text();
    }
    return QString();
}

WeatherConfigSearch::WeatherConfigSearch(QWidget *parent)
    : KDialog(parent)
    , m_dataengine(0)
    , m_listDlg(this)
{
    setupUi(mainWidget());
    setButtons(KDialog::User1 | KDialog::Cancel);
    setButtonIcon(KDialog::User1, KIcon("system-search"));
    setButtonText(KDialog::User1, i18n("&Search..."));
    setDefaultButton(KDialog::User1);
    connect(this, SIGNAL(user1Clicked()), this, SLOT(searchPressed()));
}

WeatherConfigSearch::~WeatherConfigSearch()
{
}

void WeatherConfigSearch::searchPressed()
{
    QString plugin = providerComboBox->itemData(providerComboBox->currentIndex()).toString();
    QString city = cityLineEdit->text();
    QString validation = QString("%1|validate|%2").arg(plugin).arg(city);
    m_dataengine->connectSource(validation, this);
}

void WeatherConfigSearch::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    m_dataengine->disconnectSource(source, this);
    QStringList result = data["validate"].toString().split('|');

    if (result[1] == "valid") {
        QMap<QString, QString> places;
        int i = 3;
        while (i < result.count()) {
            if (result[i] == "place") {
                if (i + 2 < result.count() && result[i + 2] == "extra") {
                    places[result[i + 1]] = result[i + 3];
                    i += 4;
                } else {
                    places[result[i + 1]] = QString();
                    i += 2;
                }
            }
        }
        QString place;
        if (result[2] == "multiple") {
            m_listDlg.setList(places.keys());
            if (m_listDlg.exec() == QDialog::Rejected) {
                return;
            }
            place = m_listDlg.selected();
        } else {
            place = places.keys()[0];
        }
        m_source = QString("%1|weather|%2").arg(result[0]).arg(place);
        if (!places[place].isEmpty()) {
            m_source += QString("|%1").arg(places[place]);
        }
        accept();
    } else if (result[1] == "timeout") {
        KMessageBox::error(this, i18n("Timeout happened when trying to connect weather server."));
    } else {
        KMessageBox::error(this, i18n("Cannot find '%1'.", result[3]));
    }
}

QString WeatherConfigSearch::nameForPlugin(const QString& plugin)
{
    return providerComboBox->itemText(providerComboBox->findData(plugin));
}

void WeatherConfigSearch::setDataEngine(Plasma::DataEngine* dataengine)
{
    m_dataengine = dataengine;

    providerComboBox->clear();
    if (m_dataengine) {
        QVariantList plugins = m_dataengine->query("ions").values();
        foreach (const QVariant& plugin, plugins) {
            QStringList pluginInfo = plugin.toString().split("|");
            providerComboBox->addItem(pluginInfo[0], pluginInfo[1]);
        }
    }
}

void WeatherConfigSearch::setSource(const QString& source)
{
    m_source = source;
    QStringList list = source.split('|', QString::SkipEmptyParts);
    if (list.count() > 0) {
        providerComboBox->setCurrentIndex(providerComboBox->findData(list[0]));
    }
    if (list.count() > 2) {
        cityLineEdit->setText(list[2]);
    }
    cityLineEdit->selectAll();
    cityLineEdit->setFocus(Qt::OtherFocusReason);
}

QString WeatherConfigSearch::source()
{
    return m_source;
}

WeatherConfig::WeatherConfig(QWidget *parent)
    : QWidget(parent)
    , m_searchDlg(parent)
{
    setupUi(this);
    temperatureComboBox->addItem(i18n("Celsius \302\260C"), "C");
    temperatureComboBox->addItem(i18n("Fahrenheit \302\260F"), "F");
    temperatureComboBox->addItem(i18n("Kelvin K"), "K");
    pressureComboBox->addItem(i18n("Hectopascals hPa"), "hPa");
    pressureComboBox->addItem(i18n("Kilopascals kPa"), "kPa");
    pressureComboBox->addItem(i18n("Millibars mbar"), "mbar");
    pressureComboBox->addItem(i18n("Inches of Mercury inHg"), "inHg");
    speedComboBox->addItem(i18n("Meters per Second m/s"), "ms");
    speedComboBox->addItem(i18n("Kilometer per Hour km/h"), "kmh");
    speedComboBox->addItem(i18n("Miles per Hour mph"), "mph");
    speedComboBox->addItem(i18n("Knots kt"), "kt");
    speedComboBox->addItem(i18n("Beaufort scale bft"), "bft");
    connect(changeButton, SIGNAL(clicked()), this, SLOT(changePressed()));
}

void WeatherConfig::changePressed()
{
    m_searchDlg.setSource(m_source);
    if (m_searchDlg.exec() == QDialog::Accepted) {
        setSource(m_searchDlg.source());
    }
}

void WeatherConfig::setDataEngine(Plasma::DataEngine* dataengine)
{
    m_searchDlg.setDataEngine(dataengine);
}

void WeatherConfig::setSource(const QString& source)
{
    m_source = source;
    QStringList list = source.split('|', QString::SkipEmptyParts);
    if (list.count() > 0) {
        providerTextLabel->setText(m_searchDlg.nameForPlugin(list[0]));
    }
    if (list.count() > 2) {
        cityTextLabel->setText(list[2]);
    }
}

void WeatherConfig::setUpdateInterval(int interval)
{
    updateIntervalSpinBox->setValue(interval);
}

void WeatherConfig::setTemperatureUnit(const QString& unit)
{
    temperatureComboBox->setCurrentIndex(temperatureComboBox->findData(unit));
}

void WeatherConfig::setPressureUnit(const QString& unit)
{
    pressureComboBox->setCurrentIndex(pressureComboBox->findData(unit));
}

void WeatherConfig::setSpeedUnit(const QString& unit)
{
    speedComboBox->setCurrentIndex(speedComboBox->findData(unit));
}

QString WeatherConfig::source()
{
    return m_source;
}

int WeatherConfig::updateInterval()
{
    return updateIntervalSpinBox->value();
}

QString WeatherConfig::temperatureUnit()
{
    return temperatureComboBox->itemData(temperatureComboBox->currentIndex()).toString();
}

QString WeatherConfig::pressureUnit()
{
    return pressureComboBox->itemData(pressureComboBox->currentIndex()).toString();
}

QString WeatherConfig::speedUnit()
{
    return speedComboBox->itemData(speedComboBox->currentIndex()).toString();
}

#include "weatherconfig.moc"
