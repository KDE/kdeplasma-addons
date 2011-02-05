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

#include <QLineEdit>

#include <KDebug>
#include <KInputDialog>
#include <KPixmapSequence>
#include <kpixmapsequencewidget.h>
//#include <KPixmapSequenceWidget>

#include <KNS3/DownloadDialog>
#include <KUnitConversion/Converter>

#include "weathervalidator.h"
#include "weatheri18ncatalog.h"
#include "ui_weatherconfig.h"
using namespace KUnitConversion;

class WeatherConfig::Private
{
public:
    Private(WeatherConfig *weatherconfig)
        : q(weatherconfig),
          dataengine(0),
          dlg(0),
          busyWidget(0),
          checkedInCount(0)
    {
    }

    void setSource(int index)
    {
        QString text = ui.locationCombo->itemData(index).toString();
        if (text.isEmpty()) {
            ui.locationCombo->lineEdit()->setText(QString());
        } else {
            source = text;
            emit q->settingsChanged();
        }
    }

    void changePressed()
    {
        checkedInCount = 0;
        QString text = ui.locationCombo->currentText();

        if (text.isEmpty()) {
            return;
        }

        ui.locationCombo->clear();
        ui.locationCombo->lineEdit()->setText(text);

        if (!busyWidget) {
            busyWidget = new KPixmapSequenceWidget(q);
            KPixmapSequence seq(QLatin1String( "process-working" ), 22);
            busyWidget->setSequence(seq);
            ui.locationSearchLayout->insertWidget(1, busyWidget);
        }

        foreach (WeatherValidator *validator, validators) {
            validator->validate(text, true);
        }
    }

    void enableOK()
    {
        if (dlg) {
            dlg->enableButton(KDialog::Ok, !source.isEmpty());
        }
    }

    void addSources(const QMap<QString, QString> &sources);
    void validatorError(const QString &error);

    WeatherConfig *q;
    //QHash<QString, QString> ions;
    QList<WeatherValidator *> validators;
    Plasma::DataEngine *dataengine;
    QString source;
    Ui::WeatherConfig ui;
    KDialog *dlg;
    KPixmapSequenceWidget *busyWidget;
    int checkedInCount;
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
    connect(d->ui.locationCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(setSource(int)));
    
    connect(d->ui.locationCombo, SIGNAL(currentIndexChanged(int)) , this , SIGNAL(configValueChanged()));
    connect(d->ui.pressureComboBox, SIGNAL(currentIndexChanged(int)) , this , SIGNAL(configValueChanged()));
    connect(d->ui.updateIntervalSpinBox, SIGNAL(valueChanged(int)) , this , SIGNAL(configValueChanged()));
    connect(d->ui.temperatureComboBox, SIGNAL(currentIndexChanged(int)) , this , SIGNAL(configValueChanged()));
    connect(d->ui.pressureComboBox, SIGNAL(currentIndexChanged(int)) , this , SIGNAL(configValueChanged()));
    connect(d->ui.speedComboBox, SIGNAL(currentIndexChanged(int)) , this , SIGNAL(configValueChanged()));
    connect(d->ui.visibilityComboBox, SIGNAL(currentIndexChanged(int)) , this , SIGNAL(configValueChanged()));
}

WeatherConfig::~WeatherConfig()
{
    delete d;
}

void WeatherConfig::getNewStuff()
{
    KNS3::DownloadDialog dialog(QLatin1String( "kmediafactory.knsrc" ), this);
    dialog.exec();
    KNS3::Entry::List entries = dialog.changedEntries();
}

void WeatherConfig::setDataEngine(Plasma::DataEngine* dataengine)
{
    d->dataengine = dataengine;
    //d->ions.clear();
    qDeleteAll(d->validators);
    d->validators.clear();
    if (d->dataengine) {
        const QVariantList plugins = d->dataengine->query(QLatin1String( "ions" )).values();
        foreach (const QVariant& plugin, plugins) {
            const QStringList pluginInfo = plugin.toString().split(QLatin1Char( '|' ));
            if (pluginInfo.count() > 1) {
                //kDebug() << "ion: " << pluginInfo[0] << pluginInfo[1];
                //d->ions.insert(pluginInfo[1], pluginInfo[0]);
                WeatherValidator *validator = new WeatherValidator(this);
                connect(validator, SIGNAL(error(QString)), this, SLOT(validatorError(QString)));
                connect(validator, SIGNAL(finished(QMap<QString,QString>)), this, SLOT(addSources(QMap<QString,QString>)));
                validator->setDataEngine(dataengine);
                validator->setIon(pluginInfo[1]);
                d->validators.append(validator);
            }
        }
    }
}

void WeatherConfig::Private::validatorError(const QString &error)
{
    kDebug() << error;
}

void WeatherConfig::Private::addSources(const QMap<QString, QString> &sources)
{
    QMapIterator<QString, QString> it(sources);

    while (it.hasNext()) {
        it.next();
        QStringList list = it.value().split(QLatin1Char( '|' ), QString::SkipEmptyParts);
        if (list.count() > 2) {
            //kDebug() << list;
            QString result = i18nc("A weather station location and the weather service it comes from",
                                   "%1 (%2)", list[2], list[0]); // the names are too looong ions.value(list[0]));
            ui.locationCombo->addItem(result, it.value());
        }
    }

    ++checkedInCount;
    if (checkedInCount >= validators.count()) {
        delete busyWidget;
        busyWidget = 0;
        kDebug() << ui.locationCombo->count();
        if (ui.locationCombo->count() == 0 && ui.locationCombo->itemText(0).isEmpty()) {
            const QString current = ui.locationCombo->currentText();
            ui.locationCombo->addItem(i18n("No weather stations found for '%1'", current));
            ui.locationCombo->lineEdit()->setText(current);
        }
        ui.locationCombo->showPopup();
    }
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

void WeatherConfig::setSource(const QString &source)
{
    //kDebug() << "source set to" << source;
    const QStringList list = source.split(QLatin1Char( '|' ));
    if (list.count() > 2) {
        QString result = i18nc("A weather station location and the weather service it comes from",
                               "%1 (%2)", list[2], list[0]);
        d->ui.locationCombo->lineEdit()->setText(result);
    }
    d->source = source;
}

QString WeatherConfig::source() const
{
    return d->source;
}

int WeatherConfig::updateInterval() const
{
    return d->ui.updateIntervalSpinBox->value();
}

int WeatherConfig::temperatureUnit() const
{
    return d->ui.temperatureComboBox->itemData(d->ui.temperatureComboBox->currentIndex()).toInt();
}

int WeatherConfig::pressureUnit() const
{
    return d->ui.pressureComboBox->itemData(d->ui.pressureComboBox->currentIndex()).toInt();
}

int WeatherConfig::speedUnit() const
{
    return d->ui.speedComboBox->itemData(d->ui.speedComboBox->currentIndex()).toInt();
}

int WeatherConfig::visibilityUnit() const
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
