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

#include "qalculate_engine.h"
#include "qalculate_settings.h"
#include "qalculate_labels.h"

#include <libqalculate/Calculator.h>
#include <libqalculate/ExpressionItem.h>
#include <libqalculate/Unit.h>
#include <libqalculate/Prefix.h>
#include <libqalculate/Variable.h>
#include <libqalculate/Function.h>

#include <QApplication>
#include <QClipboard>
#include <QString>

#include <KLocale>
#include <KIO/Job>
#include <KIO/NetAccess>

QalculateEngine::QalculateEngine(QalculateSettings* settings, QObject* parent):
        QObject(parent),
        m_settings(settings)
{
    m_lastResult = "";
    
    new Calculator();
    CALCULATOR->loadGlobalDefinitions();
    CALCULATOR->loadLocalDefinitions();
    CALCULATOR->loadGlobalCurrencies();
    m_currenciesLoaded = CALCULATOR->loadExchangeRates();
}

void QalculateEngine::updateExchangeRates()
{
    KUrl source = KUrl("http://www.ecb.int/stats/eurofxref/eurofxref-daily.xml");
    KUrl dest = KUrl("~/.qalculate/eurofxref-daily.xml");
        
    KIO::Job* getJob = KIO::file_copy(source, dest, -1, KIO::Overwrite | KIO::HideProgressInfo);
    connect( getJob, SIGNAL(result(KJob*)), this, SLOT(updateResult(KJob*)) );
}

void QalculateEngine::updateResult(KJob* job)
{
    if (job->error()) {
        kDebug() << i18n("The exchange rates could not be updated. The following error has been reported: %1",job->errorString());
    } else {
        // the exchange rates have been successfully updated, now load them
        m_currenciesLoaded = CALCULATOR->loadExchangeRates();
    }
}

void QalculateEngine::evaluate(const QString& expression)
{
    if (expression.isEmpty()) {
        return;
    }

    QString input = expression;
    QByteArray ba = input.replace(QChar(0xA3), "GBP").replace(QChar(0xA5), "JPY").replace('$', "USD").replace(QChar(0x20AC), "EUR").toLatin1();
    const char *ctext = ba.data();

    EvaluationOptions eo;

    eo.auto_post_conversion = m_settings->convertToBestUnits() ? POST_CONVERSION_BEST : POST_CONVERSION_NONE;
    eo.keep_zero_units = false;

    switch (m_settings->angleUnit()) {
    case 0:
        eo.parse_options.angle_unit = ANGLE_UNIT_NONE;
        break;
    case 1:
        eo.parse_options.angle_unit = ANGLE_UNIT_RADIANS;
        break;
    case 2:
        eo.parse_options.angle_unit = ANGLE_UNIT_DEGREES;
        break;
    case 3:
        eo.parse_options.angle_unit = ANGLE_UNIT_GRADIANS;
        break;
    }
    eo.parse_options.rpn = m_settings->rpn();
    eo.parse_options.base = m_settings->base();
    eo.parse_options.preserve_format = m_settings->preserveFormat();
    eo.parse_options.read_precision = (ReadPrecisionMode) m_settings->readPrecisionMode();

    switch (m_settings->structuring()) {
    case 0:
        eo.structuring = STRUCTURING_NONE;
        break;
    case 1:
        eo.structuring = STRUCTURING_SIMPLIFY;
        break;
    case 2:
        eo.structuring = STRUCTURING_FACTORIZE;
        break;
    }

    MathStructure result = CALCULATOR->calculate(ctext, eo);

    PrintOptions po;
    switch (m_settings->fractionDisplay()) {
    case 0:
        po.number_fraction_format = FRACTION_DECIMAL;
        break;
    case 1:
        po.number_fraction_format = FRACTION_DECIMAL_EXACT;
        break;
    case 2:
        po.number_fraction_format = FRACTION_FRACTIONAL;
        break;
    case 3:
        po.number_fraction_format = FRACTION_COMBINED;
        break;
    }
    po.indicate_infinite_series = m_settings->indicateInfiniteSeries();
    po.use_all_prefixes = m_settings->useAllPrefixes();
    po.use_denominator_prefix = m_settings->useDenominatorPrefix();
    po.negative_exponents = m_settings->negativeExponents();
    po.lower_case_e = true;
    po.base = m_settings->baseDisplay();
    po.decimalpoint_sign = KGlobal::locale()->decimalSymbol().toLocal8Bit().data();

    switch (m_settings->minExp()) {
    case 0:
        po.min_exp = EXP_NONE;
        break;
    case 1:
        po.min_exp = EXP_PURE;
        break;
    case 2:
        po.min_exp = EXP_SCIENTIFIC;
        break;
    case 3:
        po.min_exp = EXP_PRECISION;
        break;
    case 4:
        po.min_exp = EXP_BASE_3;
        break;
    }

    result.format(po);
    
    m_lastResult = result.print(po).c_str();
    emit resultReady(m_lastResult);

    QalculateLabels label(m_settings);
    emit formattedResultReady(label.drawStructure(result, po));
}

void QalculateEngine::copyToClipboard(bool flag)
{
    Q_UNUSED(flag);

    QApplication::clipboard()->setText(m_lastResult);
}

