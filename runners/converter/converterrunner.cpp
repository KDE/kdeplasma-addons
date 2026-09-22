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
#include <QLocale>
#include <QMimeData>

#include <chrono>
#include <cmath>

using namespace std::chrono_literals;

K_PLUGIN_CLASS_WITH_JSON(ConverterRunner, "plasma-runner-converter.json")

Q_DECLARE_METATYPE(KUnitConversion::Value)

ConverterRunner::ConverterRunner(QObject *parent, const KPluginMetaData &metaData)
    : AbstractRunner(parent, metaData)
    , m_currencyUpdateTimer(new QTimer(this))
    , actionList({Action(QStringLiteral("copy"), QStringLiteral("edit-copy"), i18n("Copy unit and number"))})
{
    addSyntax(QStringLiteral(":q:"),
              i18n("Converts the value of :q: when :q: is made up of value unit [>, to, as, in] unit."
                   "You can use the Unit converter applet to find all available units."));
}

void ConverterRunner::init()
{
    // Splits the query into 3 groups: everything before the number, the number and everything after.
    // The number group will not match a '-' at the end to prevent "eating" it from the "->" conversion symbol, in queries like: "usd10->eur"
    splitRegex = QRegularExpression(QStringLiteral("^(.*?)([0-9,./+-]+(?:[eE][+-]?[0-9]+)?(?<!-))(.*)"));

    const QStringList conversionWordList =
        i18nc(
            "list of words that can be used as amount of 'unit1' [in|to|as] 'unit2' (e.g. 'km [in|to|as] miles'). Can be more (or less) than three words, if "
            "it makes sense. Do not put spaces before or after the `;`!",
            "in;to;as")
            .split(QLatin1Char(';'));
    QString conversionWords;
    for (qsizetype i = 0; i < conversionWordList.length(); i++) {
        if (i > 0) {
            conversionWords.append(QLatin1Char('|'));
        }
        conversionWords.append(QRegularExpression::escape(conversionWordList[i].trimmed()));
    }

    // Matches the conversion words/symbols, the conversion words must be surrounded by spaces (or the string boundary), symbols dont have to.
    conversionOperatorRegex = QRegularExpression(QStringLiteral("(?:^| )(?:%1)(?= |$)|[=-]?>|=+").arg(conversionWords));

    setMinLetterCount(2);
    setMatchRegex(splitRegex);

    converter = std::make_unique<KUnitConversion::Converter>();
    checkCompatibleUnits();

    m_currencyUpdateTimer->setInterval(24h);
    connect(m_currencyUpdateTimer, &QTimer::timeout, this, &ConverterRunner::checkCompatibleUnits);
    m_currencyUpdateTimer->start();
}

ConverterRunner::~ConverterRunner() = default;

void ConverterRunner::match(RunnerContext &context)
{
    const QRegularExpressionMatch splitRegexMatch = splitRegex.match(context.query().simplified());
    if (!splitRegexMatch.hasMatch()) {
        return;
    }
    const QStringView beforeValue = splitRegexMatch.capturedView(1).trimmed();
    const QString inputValueString = splitRegexMatch.captured(2).trimmed();
    const QStringView afterValue = splitRegexMatch.capturedView(3).trimmed();
    if (beforeValue.isEmpty() && afterValue.isEmpty()) {
        return;
    }

    const auto parsedUnitStrings = parseQueryUnits(beforeValue, afterValue);
    // Return if input unit is empty
    if (parsedUnitStrings.first.isEmpty()) {
        return;
    }
    QString inputUnitString = parsedUnitStrings.first.toString();
    QString outputUnitString = parsedUnitStrings.second.toString();

    // Check if unit is valid, otherwise check for the value in the compatibleUnits map
    KUnitConversion::UnitCategory inputCategory = converter->categoryForUnit(inputUnitString);
    if (inputCategory.id() == KUnitConversion::InvalidCategory) {
        inputUnitString = compatibleUnits.value(inputUnitString.toUpper());
        inputCategory = converter->categoryForUnit(inputUnitString);
        if (inputCategory.id() == KUnitConversion::InvalidCategory) {
            return;
        }
    }

    // Return if the unit is before the value and the unit is not a currency to avoid ambiguous situations like: 'm23' ('3m2' vs '23m')
    if (!beforeValue.isEmpty() && inputCategory.id() != KUnitConversion::CurrencyCategory) {
        return;
    }

    const KUnitConversion::Unit inputUnit = inputCategory.unit(inputUnitString);
    const QList<KUnitConversion::Unit> outputUnits = createResultUnits(outputUnitString, inputCategory);
    const auto numberDataPair = getValidatedNumberValue(inputValueString);
    // Return on invalid user input
    if (!numberDataPair.first) {
        return;
    }

    const double numberValue = numberDataPair.second;
    QList<QueryMatch> matches;
    for (const KUnitConversion::Unit &outputUnit : outputUnits) {
        KUnitConversion::Value outputValue = inputCategory.convert(KUnitConversion::Value(numberValue, inputUnit), outputUnit);
        if (!outputValue.isValid() || inputUnit == outputUnit) {
            continue;
        }

        QueryMatch match(this);
        match.setCategoryRelevance(QueryMatch::CategoryRelevance::Moderate);
        match.setIconName(QStringLiteral("accessories-calculator"));
        if (outputUnit.categoryId() == KUnitConversion::CurrencyCategory) {
            match.setText(QStringLiteral("%1 (%2)").arg(outputValue.toString(0, 'f', 2), outputUnit.symbol()));
        } else {
            match.setText(QStringLiteral("%1 (%2)").arg(outputValue.toString(), outputUnit.symbol()));
        }
        match.setData(QVariant::fromValue(outputValue));
        match.setRelevance(1.0 - std::abs(std::log10(outputValue.number())) / 50.0);
        match.setActions(actionList);
        matches.append(match);
    }

    context.addMatches(matches);
}

void ConverterRunner::run(const RunnerContext & /*context*/, const QueryMatch &match)
{
    const auto value = match.data().value<KUnitConversion::Value>();

    if (match.selectedAction()) {
        QGuiApplication::clipboard()->setText(value.toString());
    } else {
        QGuiApplication::clipboard()->setText(QString::number(value.number(), 'f', QLocale::FloatingPointShortest));
    }
}

QMimeData *ConverterRunner::mimeDataForMatch(const QueryMatch &match)
{
    const auto value = match.data().value<KUnitConversion::Value>();

    auto *mimeData = new QMimeData();
    mimeData->setText(value.toSymbolString());
    return mimeData;
}

QPair<bool, double> ConverterRunner::stringToDouble(const QStringView &value)
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
    const auto fractionParts = QStringView(value).split(QLatin1Char('/'), Qt::SkipEmptyParts);
    if (fractionParts.isEmpty() || fractionParts.count() > 2) {
        return {false, 0};
    }

    if (fractionParts.count() == 2) {
        // Dont allow a number containing both a fraction and scientific notation
        if (value.contains(QLatin1Char('e'), Qt::CaseInsensitive)) {
            return {false, 0};
        }
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

QPair<QStringView, QStringView> ConverterRunner::parseQueryUnits(const QStringView beforeValue, const QStringView afterValue)
{
    if (!beforeValue.isEmpty()) {
        if (afterValue.isEmpty()) {
            return {beforeValue, QStringView()};
        }
        const QRegularExpressionMatch conversionWordMatch = conversionOperatorRegex.matchView(afterValue);
        if (conversionWordMatch.hasMatch() && conversionWordMatch.capturedStart() == 0) {
            return {beforeValue, afterValue.sliced(conversionWordMatch.capturedEnd()).trimmed()};
        }
        return {QStringView(), QStringView()};
    }

    // Searches for a suitable match for the conversion operator
    QRegularExpressionMatchIterator it = conversionOperatorRegex.globalMatchView(afterValue);
    while (it.hasNext()) {
        const auto match = it.next();
        if (match.capturedStart() == 0) {
            continue;
        }

        // Continue only if the next match is directly behind this match and is not at the end of the string
        if (it.hasNext() && match.capturedEnd() == it.peekNext().capturedStart() && it.peekNext().capturedEnd() != afterValue.length()) {
            continue;
        }
        return {afterValue.first(match.capturedStart()).trimmed(), afterValue.sliced(match.capturedEnd()).trimmed()};
    }

    return {afterValue, QStringView()};
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

void ConverterRunner::checkCompatibleUnits()
{
    // Add all currency symbols to the map, if their ISO code is supported by backend
    QMetaObject::invokeMethod(
        QCoreApplication::instance(),
        [this] {
            KUnitConversion::UnitCategory currencyCategory = converter->category(KUnitConversion::CurrencyCategory);
            auto updateJob = currencyCategory.syncConversionTable();
            if (!updateJob) [[unlikely]] {
                if (compatibleUnits.empty()) {
                    QMetaObject::invokeMethod(this, &ConverterRunner::updateCompatibleUnits);
                }
                return;
            }
            connect(updateJob, &KUnitConversion::UpdateJob::finished, this, &ConverterRunner::updateCompatibleUnits);
        },
        Qt::BlockingQueuedConnection);
}

void ConverterRunner::updateCompatibleUnits()
{
    KUnitConversion::UnitCategory currencyCategory = converter->category(KUnitConversion::CurrencyCategory);
    const QList<QLocale> allLocales = QLocale::matchingLocales(QLocale::AnyLanguage, QLocale::AnyScript, QLocale::AnyCountry);
    const QStringList availableISOCodes = currencyCategory.allUnits();
    const QRegularExpression hasCurrencyRegex = QRegularExpression(QStringLiteral("\\p{Sc}")); // clazy:exclude=use-static-qregularexpression
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
    const auto categories = converter->categories();
    for (const auto &category : categories) {
        const auto allUnits = category.allUnits();
        for (const auto &unit : allUnits) {
            compatibleUnits.insert(unit.toUpper(), unit);
        }
    }
}

#include "converterrunner.moc"
