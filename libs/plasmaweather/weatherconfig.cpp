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

#include <KNS3/DownloadDialog>
#include <KUnitConversion/Converter>

using namespace KUnitConversion;

class WeatherConfig::Private
{
public:
    Private(WeatherConfig *weatherconfig)
        : q(weatherconfig),
          searchDlg(weatherconfig)
    {
    }

    void changePressed()
    {
        searchDlg.setSource(source);
        searchDlg.show();
    }

    void searchResult(int result)
    {
        if (result == QDialog::Accepted) {
            q->setSource(searchDlg.source());
            searchDlg.hide();
        } else {
            searchDlg.hide();
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
    Plasma::DataEngine *m_engine;
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
    d->ui.temperatureComboBox->addItem(i18n("Celsius \302\260C"), Celsius);
    d->ui.temperatureComboBox->addItem(i18n("Fahrenheit \302\260F"), Fahrenheit);
    d->ui.temperatureComboBox->addItem(i18n("Kelvin K"), Kelvin);
    d->ui.pressureComboBox->addItem(i18n("Hectopascals hPa"), Hectopascal);
    d->ui.pressureComboBox->addItem(i18n("Kilopascals kPa"), Kilopascal);
    d->ui.pressureComboBox->addItem(i18n("Millibars mbar"), Millibar);
    d->ui.pressureComboBox->addItem(i18n("Inches of Mercury inHg"), InchesOfMercury);
    d->ui.speedComboBox->addItem(i18n("Meters per Second m/s"), MeterPerSecond);
    d->ui.speedComboBox->addItem(i18n("Kilometers per Hour km/h"), KilometerPerHour);
    d->ui.speedComboBox->addItem(i18n("Miles per Hour mph"), MilePerHour);
    d->ui.speedComboBox->addItem(i18n("Knots kt"), Knot);
    d->ui.speedComboBox->addItem(i18n("Beaufort scale bft"), Beaufort);
    d->ui.visibilityComboBox->addItem(i18n("Kilometers"), Kilometer);
    d->ui.visibilityComboBox->addItem(i18n("Miles"), Mile);

    // Setup GHNS button icon -- TODO: spstarr: Enable for for KDE 4.5, we need some plumbing done in Plasma javascript dataengine first.
    //d->ui.ghnsProviderButton->setIcon(KIcon("get-hot-newstuff"));
    //connect(d->ui.ghnsProviderButton, SIGNAL(clicked()), this, SLOT(getNewStuff()));

    d->ui.providerLabel_2->hide();
    d->ui.ghnsProviderButton->hide();

    connect(d->ui.changeButton, SIGNAL(clicked()), this, SLOT(changePressed()));
    connect(&d->searchDlg, SIGNAL(finished(int)), this, SLOT(searchResult(int)));

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

void WeatherConfig::getNewStuff()
{
    KNS3::DownloadDialog dialog("kmediafactory.knsrc", this);
    dialog.exec();
    KNS3::Entry::List entries = dialog.changedEntries();
    if (entries.size() > 0) {
        kDebug() << "About to ask WeatherEngine for plugin update!";
        d->m_engine->setProperty("update", true);
    }
}

void WeatherConfig::setDataEngine(Plasma::DataEngine* dataengine)
{
    d->m_engine = dataengine;
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
    d->ui.updateIntervalSpinBox->setSuffix(ki18np(" minute", " minutes"));
}

void WeatherConfig::setTemperatureUnit(int unit)
{
    int index = d->ui.temperatureComboBox->findData(unit);

    if (index != -1) {
        d->ui.temperatureComboBox->setCurrentIndex(index);
    }
}

void WeatherConfig::setPressureUnit(int unit)
{
    int index = d->ui.pressureComboBox->findData(unit);

    if (index != -1) {
        d->ui.pressureComboBox->setCurrentIndex(index);
    }
}

void WeatherConfig::setSpeedUnit(int unit)
{
    int index = d->ui.speedComboBox->findData(unit);

    if (index != -1) {
        d->ui.speedComboBox->setCurrentIndex(index);
    }
}

void WeatherConfig::setVisibilityUnit(int unit)
{
    int index = d->ui.visibilityComboBox->findData(unit);

    if (index != -1) {
        d->ui.visibilityComboBox->setCurrentIndex(index);
    }
}

QString WeatherConfig::source()
{
    return d->source;
}

int WeatherConfig::updateInterval()
{
    return d->ui.updateIntervalSpinBox->value();
}

int WeatherConfig::temperatureUnit()
{
    return d->ui.temperatureComboBox->itemData(d->ui.temperatureComboBox->currentIndex()).toInt();
}

int WeatherConfig::pressureUnit()
{
    return d->ui.pressureComboBox->itemData(d->ui.pressureComboBox->currentIndex()).toInt();
}

int WeatherConfig::speedUnit()
{
    return d->ui.speedComboBox->itemData(d->ui.speedComboBox->currentIndex()).toInt();
}

int WeatherConfig::visibilityUnit()
{
    return d->ui.visibilityComboBox->itemData(d->ui.visibilityComboBox->currentIndex()).toInt();
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
