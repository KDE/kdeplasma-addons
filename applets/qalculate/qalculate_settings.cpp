/*
*   Copyright 2009 Matteo Agostinelli <agostinelli@gmail.com>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License as
*   published by the Free Software Foundation; either version 2 or
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
#include <KConfigGroup>
#include <KConfigDialog>

#include <QGridLayout>
#include <QCheckBox>
#include <QLabel>

#include <KComboBox>
#include <QSpinBox>

#include "qalculate_settings.h"
#include "qalculate_applet.h"

QalculateSettings::QalculateSettings(QalculateApplet* applet): QObject(0), m_applet(applet)
{
    readSettings();
}

void QalculateSettings::readSettings()
{
    KConfigGroup cfg = m_applet->config();

    // load settings
    m_convertToBestUnits = cfg.readEntry("convertToBestUnits", true);
    m_structuring = cfg.readEntry("structuring", 1);
    m_fractionDisplay = cfg.readEntry("fractionDisplay", 0);
    m_angleUnit = cfg.readEntry("angleUnit", 1);
    m_readPrecisionMode = cfg.readEntry("readPrecisionMode", 0);
    m_indicateInfiniteSeries = cfg.readEntry("indicateInfiniteSeries", false);
    m_useAllPrefixes = cfg.readEntry("useAllPrefixes", false);
    m_useDenominatorPrefix = cfg.readEntry("useDenominatorPrefix", true);
    m_negativeExponents = cfg.readEntry("negativeExponents", false);
    m_updateExchangeRatesAtStartup = cfg.readEntry("updateExchangeRatesAtStartup", true);
    m_copyToClipboard = cfg.readEntry("copyToClipboard", false);
    m_resultsInline = cfg.readEntry("resultsInline", false);
    m_rpn = cfg.readEntry("rpn", false);
    m_preserveFormat = cfg.readEntry("preserveFormat", false);
    m_liveEvaluation = cfg.readEntry("liveEvaluation", false);
    m_base = cfg.readEntry("base", 10);
    m_baseDisplay = cfg.readEntry("baseDisplay", 10);
    m_minExp = cfg.readEntry("minExp", 0);
}

void QalculateSettings::writeSettings()
{
    KConfigGroup cfg = m_applet->config();

    // write settings
    cfg.writeEntry("convertToBestUnits", m_unitsCheck->checkState() == Qt::Checked);
    cfg.writeEntry("structuring", m_structuringCombo->currentIndex());
    cfg.writeEntry("angleUnit", m_angleUnitCombo->currentIndex());
    cfg.writeEntry("fractionDisplay", m_fractionCombo->currentIndex());
    cfg.writeEntry("indicateInfiniteSeries", m_infiniteSeriesCheck->checkState() == Qt::Checked);
    cfg.writeEntry("useAllPrefixes", m_allPrefixesCheck->checkState() == Qt::Checked);
    cfg.writeEntry("useDenominatorPrefix", m_denominatorPrefixCheck->checkState() == Qt::Checked);
    cfg.writeEntry("negativeExponents", m_negativeExponentsCheck->checkState() == Qt::Checked);
    cfg.writeEntry("updateExchangeRatesAtStartup", m_exchangeRatesCheck->checkState() == Qt::Checked);
    cfg.writeEntry("copyToClipboard", m_copyToClipboardCheck->checkState() == Qt::Checked);
    cfg.writeEntry("resultsInline", m_resultsInlineCheck->checkState() == Qt::Checked);
    cfg.writeEntry("liveEvaluation", m_liveEvaluationCheck->checkState() == Qt::Checked);
    cfg.writeEntry("rpn", m_rpnCheck->checkState() == Qt::Checked);
    cfg.writeEntry("base", m_baseSpin->value());
    cfg.writeEntry("baseDisplay", m_baseDisplaySpin->value());
    cfg.writeEntry("minExp", m_minExpCombo->currentIndex());
}

void QalculateSettings::createConfigurationInterface(KConfigDialog* parent)
{
    m_configDialog = parent;

    m_configDialog->setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Apply);
    connect(m_configDialog, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(m_configDialog, SIGNAL(okClicked()), this, SLOT(configAccepted()));

    QWidget *page = new QWidget();

    QGridLayout *layout = new QGridLayout(page);
    // convert to best units

    m_unitsCheck = new QCheckBox(i18n("Convert to &best units"), page);
    m_unitsCheck->setCheckState(m_convertToBestUnits ? Qt::Checked : Qt::Unchecked);

    m_copyToClipboardCheck = new QCheckBox(i18n("Copy result to clipboard"), page);
    m_copyToClipboardCheck->setCheckState(m_copyToClipboard ? Qt::Checked : Qt::Unchecked);
    m_resultsInlineCheck = new QCheckBox(i18n("Write results in input line edit"), page);
    m_resultsInlineCheck->setCheckState(m_resultsInline ? Qt::Checked : Qt::Unchecked);
    m_liveEvaluationCheck = new QCheckBox(i18n("Live evaluation"), page);
    m_liveEvaluationCheck->setCheckState(m_liveEvaluation ? Qt::Checked : Qt::Unchecked);
    connect(m_liveEvaluationCheck, SIGNAL(stateChanged(int)), this, SLOT(checkValidity()));
    m_rpnCheck = new QCheckBox(i18n("Enable reverse Polish notation"), page);
    m_rpnCheck->setCheckState(m_rpn ? Qt::Checked : Qt::Unchecked);

    QLabel *baseLabel = new QLabel(i18n("Expression base:"), page);
    m_baseSpin = new QSpinBox(page);
    m_baseSpin->setValue(m_base);
    m_baseSpin->setMinimum(2);
    m_baseSpin->setMaximum(32);

    QLabel *baseDisplayLabel = new QLabel(i18n("Result base:"), page);
    m_baseDisplaySpin = new QSpinBox(page);
    m_baseDisplaySpin->setValue(m_baseDisplay);
    m_baseDisplaySpin->setMinimum(2);
    m_baseDisplaySpin->setMaximum(32);

    QLabel *structuringLabel = new QLabel(i18n("Structuring mode:"), page);

    m_structuringCombo = new KComboBox(page);
    m_structuringCombo->addItem(i18n("None"));
    m_structuringCombo->addItem(i18n("Simplify"));
    m_structuringCombo->addItem(i18n("Factorize"));

    QLabel *angleUnitLabel = new QLabel(i18n("Angle unit:"), page);
    m_angleUnitCombo = new KComboBox(page);
    m_angleUnitCombo->addItem(i18n("None"));
    m_angleUnitCombo->addItem(i18n("Radians"));
    m_angleUnitCombo->addItem(i18n("Degrees"));
    m_angleUnitCombo->addItem(i18n("Gradians"));

    m_structuringCombo->setCurrentIndex(m_structuring);
    m_angleUnitCombo->setCurrentIndex(m_angleUnit);

    layout->addWidget(m_unitsCheck, 0, 0);
    layout->addWidget(m_copyToClipboardCheck, 1, 0);
    layout->addWidget(m_resultsInlineCheck, 2, 0);
    layout->addWidget(m_liveEvaluationCheck, 3, 0);
    layout->addWidget(m_rpnCheck, 4, 0);
    layout->addWidget(structuringLabel, 5, 0);
    layout->addWidget(m_structuringCombo, 5, 1);
    layout->addWidget(angleUnitLabel, 6, 0);
    layout->addWidget(m_angleUnitCombo, 6, 1);
    layout->addWidget(baseLabel, 7, 0);
    layout->addWidget(m_baseSpin, 7, 1);
    layout->addWidget(baseDisplayLabel, 8, 0);
    layout->addWidget(m_baseDisplaySpin, 8, 1);
    layout->setRowStretch(9, 10);

    m_configDialog->addPage(page, i18nc("Evaluation", "Evaluation Settings"), m_applet->icon());
    
    QWidget *printPage = new QWidget();

    QGridLayout *printLayout = new QGridLayout(printPage);

    QLabel *fractionLabel = new QLabel(i18n("Number fraction format:"), printPage);

    m_fractionCombo = new KComboBox(printPage);
    m_fractionCombo->addItem(i18n("Decimal"));
    m_fractionCombo->addItem(i18n("Exact"));
    m_fractionCombo->addItem(i18n("Fractional"));
    m_fractionCombo->addItem(i18n("Combined"));
    m_fractionCombo->setCurrentIndex(m_fractionDisplay);

    QLabel *minExpLabel = new QLabel(i18n("Numerical display:"), printPage);

    m_minExpCombo = new KComboBox(printPage);
    m_minExpCombo->addItem(i18n("None"));
    m_minExpCombo->addItem(i18n("Pure"));
    m_minExpCombo->addItem(i18n("Scientific"));
    m_minExpCombo->addItem(i18n("Precision"));
    m_minExpCombo->addItem(i18n("Engineering"));

    m_minExpCombo->setCurrentIndex(m_minExp);

    m_infiniteSeriesCheck = new QCheckBox(i18n("Indicate infinite series"), printPage);
    m_infiniteSeriesCheck->setCheckState(m_indicateInfiniteSeries ? Qt::Checked : Qt::Unchecked);
    m_allPrefixesCheck = new QCheckBox(i18n("Use all prefixes"), printPage);
    m_allPrefixesCheck->setCheckState(m_useAllPrefixes ? Qt::Checked : Qt::Unchecked);
    m_denominatorPrefixCheck = new QCheckBox(i18n("Use denominator prefix"), printPage);
    m_denominatorPrefixCheck->setCheckState(m_useDenominatorPrefix ? Qt::Checked : Qt::Unchecked);
    m_negativeExponentsCheck = new QCheckBox(i18n("Negative exponents"), printPage);
    m_negativeExponentsCheck->setCheckState(m_negativeExponents ? Qt::Checked : Qt::Unchecked);

    printLayout->addWidget(fractionLabel, 0, 0);
    printLayout->addWidget(m_fractionCombo, 0, 1);
    printLayout->addWidget(minExpLabel, 1, 0);
    printLayout->addWidget(m_minExpCombo, 1, 1);

    printLayout->addWidget(m_infiniteSeriesCheck, 2, 0);
    printLayout->addWidget(m_allPrefixesCheck, 3, 0);
    printLayout->addWidget(m_denominatorPrefixCheck, 4, 0);
    printLayout->addWidget(m_negativeExponentsCheck, 5, 0);
    printLayout->setRowStretch(6, 10);

    m_configDialog->addPage(printPage, i18nc("Print", "Print Settings"), m_applet->icon());

    QWidget *currencyPage = new QWidget();
    QGridLayout *currencyLayout = new QGridLayout(currencyPage);

    m_exchangeRatesCheck = new QCheckBox(i18n("Update exchange rates at startup"), currencyPage);
    m_exchangeRatesCheck->setCheckState(m_updateExchangeRatesAtStartup ? Qt::Checked : Qt::Unchecked);
    currencyLayout->addWidget(m_exchangeRatesCheck, 0, 0);
    currencyLayout->setRowStretch(1, 10);

    m_configDialog->addPage(currencyPage, i18nc("Currency", "Currency Settings"), m_applet->icon());
}

void QalculateSettings::configAccepted()
{
    writeSettings();
    readSettings();
    emit configChanged();
}

void QalculateSettings::checkValidity()
{
    if (m_liveEvaluationCheck->checkState() == Qt::Checked) {
        m_resultsInlineCheck->setCheckState(Qt::Unchecked);
        m_resultsInlineCheck->setEnabled(false);
    } else {
        m_resultsInlineCheck->setEnabled(true);
        m_resultsInlineCheck->setCheckState(m_resultsInline ? Qt::Checked : Qt::Unchecked);
    }
}




