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

#include "weatherconfig.h"
#include "weatherconfigsearch.h"
#include "weathervalidator.h"
#include "weatheri18ncatalog.h"
#include "ui_weatherconfig.h"
#include <KMessageBox>
#include <KInputDialog>

class WeatherConfig::Private
{
public:
    Private(WeatherConfig *weatherconfig)
        : q(weatherconfig)
        , searchDlg(weatherconfig)
    {
    }
    
    void changePressed()
    {
        searchDlg.setSource(source);
        if (searchDlg.exec() == QDialog::Accepted) {
            q->setSource(searchDlg.source());
        }
    }

    void enableOK()
    {
        if (dlg) {
            dlg->enableButton(KDialog::Ok, !source.isEmpty());
        }
    }

    WeatherConfig *q;
    WeatherConfigSearch searchDlg;
    QString source;
    Ui::WeatherConfig ui;
    KDialog *dlg;
};

WeatherConfig::WeatherConfig(QWidget *parent)
    : QWidget(parent)
    , d(new Private(this))
{
    Weatheri18nCatalog::loadCatalog();

    d->dlg = qobject_cast<KDialog*>(parent);
    d->ui.setupUi(this);
    d->ui.temperatureComboBox->addItem(i18n("Celsius \302\260C"), "C");
    d->ui.temperatureComboBox->addItem(i18n("Fahrenheit \302\260F"), "F");
    d->ui.temperatureComboBox->addItem(i18n("Kelvin K"), "K");
    d->ui.pressureComboBox->addItem(i18n("Hectopascals hPa"), "hPa");
    d->ui.pressureComboBox->addItem(i18n("Kilopascals kPa"), "kPa");
    d->ui.pressureComboBox->addItem(i18n("Millibars mbar"), "mbar");
    d->ui.pressureComboBox->addItem(i18n("Inches of Mercury inHg"), "inHg");
    d->ui.speedComboBox->addItem(i18n("Meters per Second m/s"), "ms");
    d->ui.speedComboBox->addItem(i18n("Kilometers per Hour km/h"), "km/h");
    d->ui.speedComboBox->addItem(i18n("Miles per Hour mph"), "mph");
    d->ui.speedComboBox->addItem(i18n("Knots kt"), "kt");
    d->ui.speedComboBox->addItem(i18n("Beaufort scale bft"), "bft");
    d->ui.visibilityComboBox->addItem(i18n("Kilometers"), "km");
    d->ui.visibilityComboBox->addItem(i18n("Miles"), "ml");

    connect(d->ui.changeButton, SIGNAL(clicked()), this, SLOT(changePressed()));
    connect(d->ui.updateIntervalSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(setUpdateInterval(int)));
            
    connect(d->ui.updateIntervalSpinBox, SIGNAL(valueChanged(int)), 
            this, SIGNAL(settingsChanged()));
    connect(d->ui.temperatureComboBox, SIGNAL(currentIndexChanged(int)), 
            this, SIGNAL(settingsChanged()));
    connect(d->ui.pressureComboBox, SIGNAL(currentIndexChanged(int)), 
            this, SIGNAL(settingsChanged()));
    connect(d->ui.speedComboBox, SIGNAL(currentIndexChanged(int)), 
            this, SIGNAL(settingsChanged()));
    connect(d->ui.visibilityComboBox, SIGNAL(currentIndexChanged(int)), 
            this, SIGNAL(settingsChanged()));
            
}

WeatherConfig::~WeatherConfig()
{
    delete d;
}

void WeatherConfig::setDataEngine(Plasma::DataEngine* dataengine)
{
    d->searchDlg.setDataEngine(dataengine);
}

void WeatherConfig::setSource(const QString& source)
{
    d->source = source;
    QStringList list = source.split('|', QString::SkipEmptyParts);
    if (list.count() > 0) {
        d->ui.providerTextLabel->setText(d->searchDlg.nameForPlugin(list[0]));
    }
    if (list.count() > 2) {
        d->ui.cityTextLabel->setText(list[2]);
    }
    d->enableOK();
    emit settingsChanged();
}

void WeatherConfig::setUpdateInterval(int interval)
{
    d->ui.updateIntervalSpinBox->setValue(interval);
    d->ui.updateIntervalSpinBox->setSuffix(i18np(" minute", " minutes", interval));
}

void WeatherConfig::setTemperatureUnit(const QString& unit)
{
    d->ui.temperatureComboBox->setCurrentIndex(d->ui.temperatureComboBox->findData(unit));
}

void WeatherConfig::setPressureUnit(const QString& unit)
{
    d->ui.pressureComboBox->setCurrentIndex(d->ui.pressureComboBox->findData(unit));
}

void WeatherConfig::setSpeedUnit(const QString& unit)
{
    d->ui.speedComboBox->setCurrentIndex(d->ui.speedComboBox->findData(unit));
}

void WeatherConfig::setVisibilityUnit(const QString& unit)
{
    d->ui.visibilityComboBox->setCurrentIndex(d->ui.visibilityComboBox->findData(unit));
}

QString WeatherConfig::source()
{
    return d->source;
}

int WeatherConfig::updateInterval()
{
    return d->ui.updateIntervalSpinBox->value();
}

QString WeatherConfig::temperatureUnit()
{
    return d->ui.temperatureComboBox->itemData(
            d->ui.temperatureComboBox->currentIndex()).toString();
}

QString WeatherConfig::pressureUnit()
{
    return d->ui.pressureComboBox->itemData(d->ui.pressureComboBox->currentIndex()).toString();
}

QString WeatherConfig::speedUnit()
{
    return d->ui.speedComboBox->itemData(d->ui.speedComboBox->currentIndex()).toString();
}

QString WeatherConfig::visibilityUnit()
{
    return d->ui.visibilityComboBox->itemData(d->ui.visibilityComboBox->currentIndex()).toString();
}

void WeatherConfig::setConfigurableUnits(const ConfigurableUnits units)
{
    d->ui.unitsLabel->setVisible(units != None);
    d->ui.temperatureLabel->setVisible(units & Temperature);
    d->ui.temperatureComboBox->setVisible(units & Temperature);
    d->ui.pressureLabel->setVisible(units & Pressure);
    d->ui.pressureComboBox->setVisible(units & Pressure);
    d->ui.speedLabel->setVisible(units & Speed);
    d->ui.speedComboBox->setVisible(units & Speed);
    d->ui.visibilityLabel->setVisible(units & Visibility);
    d->ui.visibilityComboBox->setVisible(units & Visibility);
}

void WeatherConfig::setHeadersVisible(bool visible)
{
    d->ui.locationLabel->setVisible(visible);
    d->ui.unitsLabel->setVisible(visible);
}

#include "weatherconfig.moc"
