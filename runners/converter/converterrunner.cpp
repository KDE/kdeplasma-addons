/*
 * SPDX-FileCopyrightText: 2007, 2008 Petri Damstén <damu@iki.fi>
 * SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "converterrunner.h"

#include <KLocalizedString>
#include <QClipboard>
#include <QDebug>
#include <QDesktopServices>
#include <QGuiApplication>

#include <cmath>

K_PLUGIN_CLASS_WITH_JSON(ConverterRunner, "plasma-runner-converter.json")

ConverterRunner::ConverterRunner(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
    : Plasma::AbstractRunner(parent, metaData, args)
{
    setObjectName(QStringLiteral("Converter"));

    const QString description = i18n(
        "Converts the value of :q: when :q: is made up of "
        "\"value unit [>, to, as, in] unit\". You can use the "
        "Unit converter applet to find all available units.");
    addSyntax(Plasma::RunnerSyntax(QStringLiteral(":q:"), description));
}

void ConverterRunner::init()
{
    valueRegex = QRegularExpression(QStringLiteral("^([0-9,./+-]+)"));
    const QStringList conversionWords = i18nc("list of words that can used as amount of 'unit1' [in|to|as] 'unit2'", "in;to;as").split(QLatin1Char(';'));
    QString conversionRegex;
    for (const auto &word : conversionWords) {
        conversionRegex.append(QLatin1Char(' ') + word + QStringLiteral(" |"));
    }
    conversionRegex.append(QStringLiteral(" ?> ?"));
    unitSeperatorRegex = QRegularExpression(conversionRegex);
    valueRegex.optimize();
    unitSeperatorRegex.optimize();

    insertCompatibleUnits();

    actionList = {new QAction(QIcon::fromTheme(QStringLiteral("edit-copy")), i18n("Copy unit and number"), this)};
    setMinLetterCount(2);
    setMatchRegex(valueRegex);
}

ConverterRunner::~ConverterRunner() = default;

void ConverterRunner::match(Plasma::RunnerContext &context)
{
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
    for (const KUnitConversion::Unit &outputUnit : outputUnits) {
        KUnitConversion::Value outputValue = inputCategory.convert(KUnitConversion::Value(numberValue, inputUnit), outputUnit);
        if (!outputValue.isValid() || inputUnit == outputUnit) {
            continue;
        }

        Plasma::QueryMatch match(this);
        match.setType(Plasma::QueryMatch::HelperMatch);
        match.setIconName(QStringLiteral("accessories-calculator"));
        if (outputUnit.categoryId() == KUnitConversion::CurrencyCategory) {
            match.setText(QStringLiteral("%1 (%2)").arg(outputValue.toString(0, 'f', 2), outputUnit.symbol()));
        } else {
            match.setText(QStringLiteral("%1 (%2)").arg(outputValue.toString(), outputUnit.symbol()));
        }
        match.setData(outputValue.number());
        match.setRelevance(1.0 - std::abs(std::log10(outputValue.number())) / 50.0);
        match.setActions(actionList);
        matches.append(match);
    }

    context.addMatches(matches);
}

void ConverterRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context)

    if (match.selectedAction()) {
        const QString text = match.text();
        QGuiApplication::clipboard()->setText(text.left(text.indexOf(QLatin1String(" ("))));
    } else {
        QGuiApplication::clipboard()->setText(match.data().toString());
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
    const auto fractionParts = value.splitRef(QLatin1Char('/'), Qt::SkipEmptyParts);
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

QList<KUnitConversion::Unit> ConverterRunner::createResultUnits(QString &outputUnitString, const KUnitConversion::UnitCategory &category)
{
    QList<KUnitConversion::Unit> units;
    if (!outputUnitString.isEmpty()) {
        KUnitConversion::Unit outputUnit = category.unit(outputUnitString);
        if (!outputUnit.isNull() && outputUnit.isValid()) {
            units.append(outputUnit);
        } else {
            // Autocompletion for the target units
            outputUnitString = outputUnitString.toUpper();
            for (auto it = compatibleUnits.constBegin(); it != compatibleUnits.constEnd(); it++) {
                if (it.key().startsWith(outputUnitString)) {
                    outputUnit = category.unit(it.value());
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
    const QList<QLocale> allLocales = QLocale::matchingLocales(QLocale::AnyLanguage, QLocale::AnyScript, QLocale::AnyCountry);
    KUnitConversion::UnitCategory currencyCategory = converter.category(QStringLiteral("Currency"));
    const QStringList availableISOCodes = currencyCategory.allUnits();
    QRegularExpression hasCurrencyRegex = QRegularExpression(QStringLiteral("\\p{Sc}"));
    hasCurrencyRegex.optimize();
    for (const auto &currencyLocale : allLocales) {
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
    const auto categories = converter.categories();
    for (const auto &category : categories) {
        const auto allUnits = category.allUnits();
        for (const auto &unit : allUnits) {
            compatibleUnits.insert(unit.toUpper(), unit);
        }
    }
}
#include "converterrunner.moc"
