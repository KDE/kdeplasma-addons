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

#ifndef QALCULATESETTINGS_H
#define QALCULATESETTINGS_H

#include <QObject>

#include "qalculate_applet.h"

class QCheckBox;
class KComboBox;
class QSpinBox;

class QalculateSettings : public QObject
{
    Q_OBJECT
public:
    QalculateSettings(QalculateApplet* applet);

    //! Convert the result to the best units
    bool convertToBestUnits() const {
        return m_convertToBestUnits;
    }
    //! Indicate infinite series
    bool indicateInfiniteSeries() const {
        return m_indicateInfiniteSeries;
    }
    //! Use all prefixes
    bool useAllPrefixes() const {
        return m_useAllPrefixes;
    }
    //! Use denominator prefixes
    bool useDenominatorPrefix() const {
        return m_useDenominatorPrefix;
    }
    //! Use negative exponents
    bool negativeExponents() const {
        return m_negativeExponents;
    }
    //! Update exchange rates at startup
    bool updateExchangeRatesAtStartup() const {
        return m_updateExchangeRatesAtStartup;
    }
    //! Copy the result of the evaluation to clipboard
    bool copyToClipboard() const {
        return m_copyToClipboard;
    }
    //! Write the results in the input line-edit
    bool resultsInline() const {
        return m_resultsInline;
    }
    //! Live evaluation
    bool liveEvaluation() const {
        return m_liveEvaluation;
    }

    //! Structuring of the resultsInline
    //! @return 0, None
    //! @return 1, Simplify
    //! @return 2, Factorize
    int structuring() const {
        return m_structuring;
    }
    //! How to display fractions
    //! @return 0, Decimal
    //! @return 1, Exact
    //! @return 2, Fractional
    //! @return 3, Combined
    int fractionDisplay() const {
        return m_fractionDisplay;
    }
    //! Default unit for angles
    //! @return 0, ANGLE_UNIT_NONE
    //! @return 1, ANGLE_UNIT_RADIANS
    //! @return 2, ANGLE_UNIT_DEGREES
    //! @return 3, ANGLE_UNIT_GRADIANS
    int angleUnit() const {
        return m_angleUnit;
    }
    //! Use Reversed Polish Notation
    bool rpn() const {
        return m_rpn;
    }
    //! Base of parsed numbers
    int base() const {
        return m_base;
    }
    //! Base of displayed numbers
    int baseDisplay() const {
        return m_baseDisplay;
    }
    //! Read precision mode
    //! @return 0, DONT_READ_PRECISION
    //! @return 1, ALWAYS_READ_PRECISION
    //! @return 2, READ_PRECISION_WHEN_DECIMALS
    int readPrecisionMode() const {
        return m_readPrecisionMode;
    }
    //! Preserve the expression structure as much as possible
    bool preserveFormat() const {
        return m_preserveFormat;
    }
    //! Numerical display
    //! @return 0, NONE
    //! @return 1, PURE
    //! @return 2, SCIENTIFIC
    //! @return 3, PRECISION
    //! @return 4, ENGINEERING
    int minExp() const {
        return m_minExp;
    }
    //! History items
    QStringList historyItems() const {
        return m_historyItems;
    }

    bool showBinary() const {
      return m_showBinary;
    }
    bool showOctal() const {
      return m_showOctal;
    }
    bool showDecimal() const {
      return m_showDecimal;
    }
    bool showHexadecimal() const {
      return m_showHexadecimal;
    }
    bool showOtherBases() const {
      return m_showBinary || m_showOctal || m_showDecimal || m_showHexadecimal;
    }

    void setHistoryItems(QStringList items);

public slots:
    void readSettings();
    void writeSettings();

    //! Creates the configuration dialog contents.
    void createConfigurationInterface(KConfigDialog *parent);

protected slots:
    void configAccepted();
    void checkValidity();

signals:
    void accepted();

private:
    QalculateApplet* m_applet;
    KConfigDialog *m_configDialog;

    bool m_convertToBestUnits, m_indicateInfiniteSeries, m_useAllPrefixes, m_useDenominatorPrefix, m_negativeExponents;
    bool m_updateExchangeRatesAtStartup;
    bool m_copyToClipboard, m_resultsInline;
    bool m_rpn;
    bool m_preserveFormat;
    bool m_liveEvaluation;
    bool m_showBinary, m_showOctal, m_showDecimal, m_showHexadecimal;
    int m_structuring;
    int m_fractionDisplay;
    int m_angleUnit;
    int m_base, m_baseDisplay;
    int m_minExp;
    int m_readPrecisionMode;
    QStringList m_historyItems;

    QCheckBox *m_unitsCheck, *m_infiniteSeriesCheck, *m_allPrefixesCheck, *m_denominatorPrefixCheck, *m_negativeExponentsCheck, *m_rpnCheck;
    QCheckBox *m_exchangeRatesCheck;
    QCheckBox *m_copyToClipboardCheck, *m_resultsInlineCheck, *m_liveEvaluationCheck;
    KComboBox *m_structuringCombo, *m_fractionCombo, *m_angleUnitCombo, *m_minExpCombo;
    QSpinBox *m_baseSpin, *m_baseDisplaySpin;
    QCheckBox *m_showBinaryCheck, *m_showOctalCheck, *m_showDecimalCheck, *m_showHexadecimalCheck;
};

#endif // QALCULATESETTINGS_H
