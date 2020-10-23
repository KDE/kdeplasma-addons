/*
 * Copyright (C) 2007,2008 Petri Damstén <damu@iki.fi>
 * Copyright (C) 2020 Alexander Lohnau <alexander.lohnau@gmx.de>
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

#include "converterrunner.h"

#include <QGuiApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QDebug>
#include <KLocalizedString>

#include <cmath>

ConverterRunner::ConverterRunner(QObject *parent, const QVariantList &args)
    : Plasma::AbstractRunner(parent, args)
{
    setObjectName(QStringLiteral("Converter"));
    //can not ignore commands: we have things like m4
    setIgnoredTypes(Plasma::RunnerContext::Directory | Plasma::RunnerContext::File |
        Plasma::RunnerContext::NetworkLocation);

    const QString description = i18n("Converts the value of :q: when :q: is made up of "
                                     "\"value unit [>, to, as, in] unit\". You can use the "
                                     "Unit converter applet to find all available units.");
    addSyntax(Plasma::RunnerSyntax(QStringLiteral(":q:"), description));
}

void ConverterRunner::init()
{
    valueRegex = QRegularExpression(QStringLiteral("^([0-9,./+-]+)"));
    const QStringList conversionWords = i18nc("list of words that can used as amount of 'unit1' [in|to|as] 'unit2'",
                                              "in;to;as").split(QLatin1Char(';'));
    QString conversionRegex;
    for (const auto &word: conversionWords) {
        conversionRegex.append(QLatin1Char(' ') + word + QStringLiteral(" |"));
    }
    conversionRegex.append(QStringLiteral(" ?> ?"));
    unitSeperatorRegex = QRegularExpression(conversionRegex);
    valueRegex.optimize();
    unitSeperatorRegex.optimize();

    insertCompatibleUnits();

    addAction(copyActionId, QIcon::fromTheme(QStringLiteral("edit-copy")),
              i18n("Copy number"));
    addAction(copyUnitActionId, QIcon::fromTheme(QStringLiteral("edit-copy")),
              i18n("Copy unit and number"));
    actionList = {action(copyActionId), action(copyUnitActionId)};
}

ConverterRunner::~ConverterRunner() = default;

void ConverterRunner::match(Plasma::RunnerContext &context)
{
    const QString term = context.query();
    if (term.size() < 2 || !context.isValid()) {
        return;
    }

    const QRegularExpressionMatch valueRegexMatch = valueRegex.match(context.query());
    if (!valueRegexMatch.hasMatch()) {
        return;
    }
    const QString inputValueString = valueRegexMatch.captured(1);

    // Get the different units by splitting up the query with the regex
    QStringList unitStrings = context.query().simplified().remove(valueRegex).split(unitSeperatorRegex);
    if (unitStrings.isEmpty() || unitStrings.at(0).isEmpty()) {
        return;
    }
    // Check if unit is valid, otherwise check for the value in the compatibleUnits map
    QString inputUnitString = unitStrings.first().simplified();
    KUnitConversion::UnitCategory inputCategory = converter.categoryForUnit(inputUnitString);
    if (inputCategory.id() == KUnitConversion::InvalidCategory) {
        inputUnitString = compatibleUnits.value(inputUnitString.toUpper());
        inputCategory = converter.categoryForUnit(inputUnitString);
        if (inputCategory.id() == KUnitConversion::InvalidCategory) {
            return;
        }
    }

    QString outputUnitString;
    if (unitStrings.size() == 2) {
        outputUnitString = unitStrings.at(1).simplified();
    }

    const KUnitConversion::Unit inputUnit = inputCategory.unit(inputUnitString);
    const QList<KUnitConversion::Unit> outputUnits = createResultUnits(outputUnitString, inputCategory);
    const auto numberDataPair = getValidatedNumberValue(inputValueString);
    // Return on invalid user input
    if (!numberDataPair.first) {
        return;
    }

    const double numberValue = numberDataPair.second;
    QList<Plasma::QueryMatch> matches;
    for (const KUnitConversion::Unit &outputUnit: outputUnits) {
        KUnitConversion::Value outputValue = inputCategory.convert(
            KUnitConversion::Value(numberValue, inputUnit), outputUnit);
        if (!outputValue.isValid() || inputUnit == outputUnit) {
            continue;
        }

        Plasma::QueryMatch match(this);
        match.setType(Plasma::QueryMatch::InformationalMatch);
        match.setIconName(QStringLiteral("accessories-calculator"));
        if (outputUnit.categoryId() == KUnitConversion::CurrencyCategory) {
            outputValue.round(2);
            match.setText(QStringLiteral("%1 (%2)").arg(outputValue.toString(0, 'f', 2), outputUnit.symbol()));
        } else {
            match.setText(QStringLiteral("%1 (%2)").arg(outputValue.toString(), outputUnit.symbol()));
        }
        match.setData(outputValue.number());
        match.setRelevance(1.0 - std::abs(std::log10(outputValue.number())) / 50.0);
        matches.append(match);
    }

    context.addMatches(matches);
}

QList<QAction *> ConverterRunner::actionsForMatch(const Plasma::QueryMatch &match)
{
    Q_UNUSED(match)

    return actionList;
}

void ConverterRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context)

    if (match.selectedAction() == action(copyActionId)) {
        QGuiApplication::clipboard()->setText(match.data().toString());
    } else {
        QGuiApplication::clipboard()->setText(match.text().split(QLatin1String(" (")).first());
    }
}

QPair<bool, double> ConverterRunner::stringToDouble(const QStringRef &value)
{
    bool ok;
    double numberValue = locale.toDouble(value, &ok);
    if (!ok) {
        numberValue = value.toDouble(&ok);
    }
    return {ok, numberValue};
}

QPair<bool, double> ConverterRunner::getValidatedNumberValue(const QString &value)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    const auto fractionParts = value.splitRef(QLatin1Char('/'), QString::SkipEmptyParts);
#else
    const auto fractionParts = value.splitRef(QLatin1Char('/'), Qt::SkipEmptyParts);
#endif
    if (fractionParts.isEmpty() || fractionParts.count() > 2) {
        return {false, 0};
    }

    if (fractionParts.count() == 2) {
        const QPair<bool, double> doubleFirstResults = stringToDouble(fractionParts.first());
        if (!doubleFirstResults.first) {
            return {false, 0};
        }
        const QPair<bool, double> doubleSecondResult = stringToDouble(fractionParts.last());
        if (!doubleSecondResult.first || qFuzzyIsNull(doubleSecondResult.second)) {
            return {false, 0};
        }
        return {true, doubleFirstResults.second / doubleSecondResult.second};
    } else if (fractionParts.count() == 1) {
        const QPair<bool, double> doubleResult = stringToDouble(fractionParts.first());
        if (!doubleResult.first) {
            return {false, 0};
        }
        return {true, doubleResult.second};
    } else {
        return {true, 0};
    }
}

QList<KUnitConversion::Unit> ConverterRunner::createResultUnits(QString &outputUnitString,
                                                                const KUnitConversion::UnitCategory &category)
{
    QList<KUnitConversion::Unit> units;
    if (!outputUnitString.isEmpty()) {
        KUnitConversion::Unit outputUnit = category.unit(outputUnitString);
        if (!outputUnit.isNull() && outputUnit.isValid()) {
            units.append(outputUnit);
        } else {
            // Autocompletion for the target units
            outputUnitString = outputUnitString.toUpper();
            for (const auto &unitStringKey: compatibleUnits.keys()) {
                if (unitStringKey.startsWith(outputUnitString)) {
                    outputUnit = category.unit(compatibleUnits.value(unitStringKey));
                    if (!units.contains(outputUnit)) {
                        units << outputUnit;
                    }
                }
            }
        }
    } else {
        units = category.mostCommonUnits();
        // suggest converting to the user's local currency
        if (category.id() == KUnitConversion::CurrencyCategory) {
            const QString &currencyIsoCode = QLocale().currencySymbol(QLocale::CurrencyIsoCode);

            const KUnitConversion::Unit localCurrency = category.unit(currencyIsoCode);
            if (localCurrency.isValid() && !units.contains(localCurrency)) {
                units << localCurrency;
            }
        }
    }

    return units;
}
void ConverterRunner::insertCompatibleUnits()
{
    // Add all currency symbols to the map, if their ISO code is supported by backend
    const QList<QLocale> allLocales = QLocale::matchingLocales(
        QLocale::AnyLanguage, QLocale::AnyScript, QLocale::AnyCountry);
    KUnitConversion::UnitCategory currencyCategory = converter.category(QStringLiteral("Currency"));
    const QStringList availableISOCodes = currencyCategory.allUnits();
    QRegularExpression hasCurrencyRegex = QRegularExpression(QStringLiteral("\\p{Sc}"));
    hasCurrencyRegex.optimize();
    for (const auto &currencyLocale: allLocales) {
        const QString symbol = currencyLocale.currencySymbol(QLocale::CurrencySymbol);
        const QString isoCode = currencyLocale.currencySymbol(QLocale::CurrencyIsoCode);

        if (isoCode.isEmpty() || !symbol.contains(hasCurrencyRegex)) {
            continue;
        }
        if (availableISOCodes.contains(isoCode)) {
            compatibleUnits.insert(symbol.toUpper(), isoCode);
        }
    }

    // Add all units as uppercase in the map
    for (const auto &category: converter.categories()) {
        for (const auto &unit: category.allUnits()) {
            compatibleUnits.insert(unit.toUpper(), unit);
        }
    }
}
