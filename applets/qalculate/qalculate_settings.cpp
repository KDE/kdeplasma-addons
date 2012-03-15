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

#include "qalculate_settings.h"
#include "qalculate_applet.h"

#include <KConfigGroup>
#include <KConfigDialog>

#include <QFormLayout>
#include <QCheckBox>
#include <QSpinBox>
#include <QLabel>
#include <QGroupBox>

#include <KComboBox>


QalculateSettings::QalculateSettings(QalculateApplet* applet): QObject(0), m_applet(applet)
{
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
    m_showBinary = cfg.readEntry("showBinary", false);
    m_showOctal = cfg.readEntry("showOctal", false);
    m_showDecimal = cfg.readEntry("showDecimal", false);
    m_showHexadecimal = cfg.readEntry("showHexadecimal", false);
    m_historyItems = cfg.readEntry("historyItems", QStringList());
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
    cfg.writeEntry("showBinary", m_showBinaryCheck->checkState() == Qt::Checked);
    cfg.writeEntry("showOctal", m_showOctalCheck->checkState() == Qt::Checked);
    cfg.writeEntry("showDecimal", m_showDecimalCheck->checkState() == Qt::Checked);
    cfg.writeEntry("showHexadecimal", m_showHexadecimalCheck->checkState() == Qt::Checked);
}

void QalculateSettings::createConfigurationInterface(KConfigDialog* parent)
{
    m_configDialog = parent;

    m_configDialog->setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Apply);
    connect(m_configDialog, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(m_configDialog, SIGNAL(okClicked()), this, SLOT(configAccepted()));

    QWidget *page = new QWidget();

    QFormLayout *layout = new QFormLayout(page);
    // convert to best units

    m_unitsCheck = new QCheckBox(i18n("Convert to &best units"), page);
    m_unitsCheck->setCheckState(m_convertToBestUnits ? Qt::Checked : Qt::Unchecked);
    connect(m_unitsCheck, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));

    m_copyToClipboardCheck = new QCheckBox(i18n("Copy result to clipboard"), page);
    m_copyToClipboardCheck->setCheckState(m_copyToClipboard ? Qt::Checked : Qt::Unchecked);
    connect(m_copyToClipboardCheck, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    m_resultsInlineCheck = new QCheckBox(i18n("Write results in input line edit"), page);
    m_resultsInlineCheck->setCheckState(m_resultsInline ? Qt::Checked : Qt::Unchecked);
    connect(m_resultsInlineCheck, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    m_liveEvaluationCheck = new QCheckBox(i18n("Live evaluation"), page);
    m_liveEvaluationCheck->setCheckState(m_liveEvaluation ? Qt::Checked : Qt::Unchecked);
    connect(m_liveEvaluationCheck, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(m_liveEvaluationCheck, SIGNAL(stateChanged(int)), this, SLOT(checkValidity()));
    m_rpnCheck = new QCheckBox(i18n("Enable reverse Polish notation"), page);
    m_rpnCheck->setCheckState(m_rpn ? Qt::Checked : Qt::Unchecked);
    connect(m_rpnCheck, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));

    m_baseSpin = new QSpinBox(page);
    m_baseSpin->setValue(m_base);
    m_baseSpin->setMinimum(2);
    m_baseSpin->setMaximum(32);
    connect(m_baseSpin, SIGNAL(valueChanged(int)), parent, SLOT(settingsModified()));

    m_baseDisplaySpin = new QSpinBox(page);
    m_baseDisplaySpin->setValue(m_baseDisplay);
    m_baseDisplaySpin->setMinimum(2);
    m_baseDisplaySpin->setMaximum(32);
    connect(m_baseDisplaySpin, SIGNAL(valueChanged(int)), parent, SLOT(settingsModified()));

    m_structuringCombo = new KComboBox(page);
    m_structuringCombo->addItem(i18n("None"));
    m_structuringCombo->addItem(i18n("Simplify"));
    m_structuringCombo->addItem(i18n("Factorize"));
    m_structuringCombo->setCurrentIndex(m_structuring);
    connect(m_structuringCombo, SIGNAL(currentIndexChanged(int)), parent, SLOT(settingsModified()));

    m_angleUnitCombo = new KComboBox(page);
    m_angleUnitCombo->addItem(i18n("None"));
    m_angleUnitCombo->addItem(i18n("Radians"));
    m_angleUnitCombo->addItem(i18n("Degrees"));
    m_angleUnitCombo->addItem(i18n("Gradians"));
    m_angleUnitCombo->setCurrentIndex(m_angleUnit);
    connect(m_angleUnitCombo, SIGNAL(currentIndexChanged(int)), parent, SLOT(settingsModified()));

    layout->addRow(m_unitsCheck);
    layout->addRow(m_copyToClipboardCheck);
    layout->addRow(m_resultsInlineCheck);
    layout->addRow(m_liveEvaluationCheck);
    layout->addRow(m_rpnCheck);
    layout->addRow(i18n("Structuring mode:"), m_structuringCombo);
    layout->addRow(i18n("Angle unit:"), m_angleUnitCombo);
    layout->addRow(i18n("Expression base:"), m_baseSpin);
    layout->addRow(i18n("Result base:"), m_baseDisplaySpin);

    m_configDialog->addPage(page, i18nc("Evaluation", "Evaluation Settings"), m_applet->icon());

    QWidget *printPage = new QWidget();

    QFormLayout *printLayout = new QFormLayout(printPage);

    m_fractionCombo = new KComboBox(printPage);
    m_fractionCombo->addItem(i18n("Decimal"));
    m_fractionCombo->addItem(i18n("Exact"));
    m_fractionCombo->addItem(i18n("Fractional"));
    m_fractionCombo->addItem(i18n("Combined"));
    m_fractionCombo->setCurrentIndex(m_fractionDisplay);
    connect(m_fractionCombo, SIGNAL(currentIndexChanged(int)), parent, SLOT(settingsModified()));

    m_minExpCombo = new KComboBox(printPage);
    m_minExpCombo->addItem(i18n("None"));
    m_minExpCombo->addItem(i18n("Pure"));
    m_minExpCombo->addItem(i18n("Scientific"));
    m_minExpCombo->addItem(i18n("Precision"));
    m_minExpCombo->addItem(i18n("Engineering"));
    m_minExpCombo->setCurrentIndex(m_minExp);
    connect(m_minExpCombo, SIGNAL(currentIndexChanged(int)), parent, SLOT(settingsModified()));

    m_infiniteSeriesCheck = new QCheckBox(i18n("Indicate infinite series"), printPage);
    m_infiniteSeriesCheck->setCheckState(m_indicateInfiniteSeries ? Qt::Checked : Qt::Unchecked);
    connect(m_infiniteSeriesCheck, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    m_allPrefixesCheck = new QCheckBox(i18n("Use all prefixes"), printPage);
    m_allPrefixesCheck->setCheckState(m_useAllPrefixes ? Qt::Checked : Qt::Unchecked);
    connect(m_allPrefixesCheck, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    m_denominatorPrefixCheck = new QCheckBox(i18n("Use denominator prefix"), printPage);
    m_denominatorPrefixCheck->setCheckState(m_useDenominatorPrefix ? Qt::Checked : Qt::Unchecked);
    connect(m_denominatorPrefixCheck, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    m_negativeExponentsCheck = new QCheckBox(i18n("Negative exponents"), printPage);
    m_negativeExponentsCheck->setCheckState(m_negativeExponents ? Qt::Checked : Qt::Unchecked);
    connect(m_negativeExponentsCheck, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));

    QGroupBox *showBasesBox = new QGroupBox(i18n("Show integers also in base:"), printPage);
    QGridLayout *layoutBases = new QGridLayout(showBasesBox);

    m_showBinaryCheck = new QCheckBox(i18n("Binary"), printPage);
    m_showBinaryCheck->setCheckState(m_showBinary ? Qt::Checked : Qt::Unchecked);
    connect(m_showBinaryCheck, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    m_showOctalCheck = new QCheckBox(i18n("Octal"), printPage);
    m_showOctalCheck->setCheckState(m_showOctal ? Qt::Checked : Qt::Unchecked);
    connect(m_showOctalCheck, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    m_showDecimalCheck = new QCheckBox(i18n("Decimal"), printPage);
    m_showDecimalCheck->setCheckState(m_showDecimal ? Qt::Checked : Qt::Unchecked);
    connect(m_showDecimalCheck, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    m_showHexadecimalCheck = new QCheckBox(i18n("Hexadecimal"), printPage);
    m_showHexadecimalCheck->setCheckState(m_showHexadecimal ? Qt::Checked : Qt::Unchecked);
    connect(m_showHexadecimalCheck, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));

    layoutBases->addWidget(m_showBinaryCheck, 0, 0);
    layoutBases->addWidget(m_showOctalCheck, 1, 0);
    layoutBases->addWidget(m_showDecimalCheck, 2, 0);
    layoutBases->addWidget(m_showHexadecimalCheck, 3, 0);

    printLayout->addRow(i18n("Number fraction format:"), m_fractionCombo);
    printLayout->addRow(i18n("Numerical display:"), m_minExpCombo);

    printLayout->addRow(m_infiniteSeriesCheck);
    printLayout->addRow(m_allPrefixesCheck);
    printLayout->addRow(m_denominatorPrefixCheck);
    printLayout->addRow(m_negativeExponentsCheck);
    printLayout->addRow(showBasesBox);

    m_configDialog->addPage(printPage, i18nc("Print", "Print Settings"), m_applet->icon());

    QWidget *currencyPage = new QWidget();
    QFormLayout *currencyLayout = new QFormLayout(currencyPage);

    m_exchangeRatesCheck = new QCheckBox(i18n("Update exchange rates at startup"), currencyPage);
    m_exchangeRatesCheck->setCheckState(m_updateExchangeRatesAtStartup ? Qt::Checked : Qt::Unchecked);
    connect(m_exchangeRatesCheck, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    currencyLayout->addRow(m_exchangeRatesCheck);

    m_configDialog->addPage(currencyPage, i18nc("Currency", "Currency Settings"), m_applet->icon());
}

void QalculateSettings::configAccepted()
{
    writeSettings();
    readSettings();
    emit accepted();
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

void QalculateSettings::setHistoryItems(QStringList items)
{
    m_historyItems = items;
    KConfigGroup cfg = m_applet->config();
    cfg.writeEntry("historyItems", m_historyItems);
    cfg.sync();
}




