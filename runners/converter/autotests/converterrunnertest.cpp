/*
 * Copyright (C) 2020 Alexander Lohnau <alexander.lohnau@gmx.de>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) version 3, or any
 *   later version accepted by the membership of KDE e.V. (or its
 *   successor approved by the membership of KDE e.V.), which shall
 *   act as a proxy defined in Section 6 of version 3 of the license.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QTest>

#include "../converterrunner.h"

#include <clocale>

using namespace KUnitConversion;

class ConverterRunnerTest : public QObject
{
Q_OBJECT
private Q_SLOTS:
    void initTestCase();

    void testMostCommonUnits();
    void testSpecificTargetUnit();
    void testUnitsCaseInsensitive();
    void testCaseSensitiveUnits();
    void testCurrency();
    void testLettersAndCurrency();
    void testInvalidCurrency();
    void testFractions();
    void testFractionsWithoutSpecifiedTarget();
    void testInvalidFractionsWithoutSourceUnit();
    void testInvalidFractionsWithoutAnyUnit();
    void testSymbolsInUnits();
    void testNegativeValue();

private:
    ConverterRunner *runner = nullptr;
};

void ConverterRunnerTest::initTestCase()
{
    setlocale(LC_ALL, "C.utf8");
    qputenv("LANG", "en_US");
    QLocale::setDefault(QLocale::English);
    runner = new ConverterRunner(this, QVariantList());
    runner->init();
}

/**
 * Test if the most common units are displayed
 */
void ConverterRunnerTest::testMostCommonUnits()
{
    Plasma::RunnerContext context;
    context.setQuery(QStringLiteral("1m"));
    runner->match(context);

    Converter converter;
    const auto lengthCategory = converter.category(KUnitConversion::LengthCategory);
    QCOMPARE(context.matches().count(), lengthCategory.mostCommonUnits().count() -1);
}

/*
 * Test if specifying a target unit works
 */
void ConverterRunnerTest::testSpecificTargetUnit()
{
    Plasma::RunnerContext context;
    context.setQuery(QStringLiteral("1m > cm"));
    runner->match(context);

    QCOMPARE(context.matches().count(), 1);
    QCOMPARE(context.matches().first().text(), QStringLiteral("100 centimeters (cm)"));
}

/**
 * Test if the units are case insensitive
 */
void ConverterRunnerTest::testUnitsCaseInsensitive()
{
    Plasma::RunnerContext context;
    context.setQuery(QStringLiteral("1Liter in ML"));
    runner->match(context);

    QCOMPARE(context.matches().count(), 1);
}

/**
 * Test if the units that are case sensitive are correctly parsed
 */
void ConverterRunnerTest::testCaseSensitiveUnits()
{
    Plasma::RunnerContext context;
    context.setQuery(QStringLiteral("1Ms as ms"));
    runner->match(context);

    QCOMPARE(context.matches().count(), 1);
    QCOMPARE(context.matches().first().text(), QStringLiteral("1,000,000,000 milliseconds (ms)"));

    Plasma::RunnerContext context2;
    context2.setQuery(QStringLiteral("1,000,000,000milliseconds>Ms"));
    runner->match(context2);
    QCOMPARE(context2.matches().count(), 1);
    QCOMPARE(context2.matches().first().text(), "1 megasecond (Ms)");
}

/**
 * Test of a currency gets converted to the most common currencies
 */
void ConverterRunnerTest::testCurrency()
{
    Plasma::RunnerContext context;
    context.setQuery(QStringLiteral("1$"));
    runner->match(context);

    Converter converter;
    const auto currencyCategory = converter.category(KUnitConversion::CurrencyCategory);
    QList<Unit> currencyUnits = currencyCategory.mostCommonUnits();

    const QString currencyIsoCode = QLocale().currencySymbol(QLocale::CurrencyIsoCode);
    const KUnitConversion::Unit localCurrency = currencyCategory.unit(currencyIsoCode);
    if (localCurrency.isValid() && !currencyUnits.contains(localCurrency)) {
        currencyUnits << localCurrency;
    }
    QCOMPARE(context.matches().count(), currencyUnits.count() - 1);

}

/**
 * Test a combination of currency symbols and letters that is not directly supported by the conversion backend
 */
void ConverterRunnerTest::testLettersAndCurrency()
{
    Plasma::RunnerContext context;
    context.setQuery(QStringLiteral("4us$>ca$"));
    runner->match(context);

    QCOMPARE(context.matches().count(), 1);
    QVERIFY(context.matches().first().text().contains(QLatin1String("Canadian dollars (CAD)")));
}

/**
 * Test a query that matches the regex but is not valid
 */
void ConverterRunnerTest::testInvalidCurrency()
{
    Plasma::RunnerContext context;
    context.setQuery(QStringLiteral("4us$>abc$"));
    runner->match(context);

    QCOMPARE(context.matches().count(), 0);
}

/**
 * Test if the fractions are correctly parsed
 */
void ConverterRunnerTest::testFractions()
{
    Plasma::RunnerContext context;
    context.setQuery(QStringLiteral("6/3m>cm"));
    runner->match(context);

    QCOMPARE(context.matches().count(), 1);
    QCOMPARE(context.matches().first().text(), QStringLiteral("200 centimeters (cm)"));
}

/**
 * Test if fractions with source unit, but without target unit get parsed
 */
void ConverterRunnerTest::testFractionsWithoutSpecifiedTarget()
{
    Plasma::RunnerContext context;
    context.setQuery(QStringLiteral("6/3 m"));
    runner->match(context);

    Converter converter;
    const auto lengthCategory = converter.category(KUnitConversion::LengthCategory);
    QCOMPARE(context.matches().count(), lengthCategory.mostCommonUnits().count() - 1);
}

/**
 * Test if an invalid query with a fraction gets rejected
 */
void ConverterRunnerTest::testInvalidFractionsWithoutSourceUnit()
{
    Plasma::RunnerContext context;
    context.setQuery(QStringLiteral("4/4>cm"));
    runner->match(context);

    QCOMPARE(context.matches().count(), 0);
}

/**
 * Test if an invalid query with a fraction but no unit gets rejected
 */
void ConverterRunnerTest::testInvalidFractionsWithoutAnyUnit()
{
    Plasma::RunnerContext context;
    context.setQuery(QStringLiteral("1/2"));
    runner->match(context);

    QCOMPARE(context.matches().count(), 0);
}

/**
 * Test if symbols (other than currencies) are accepted
 */
void ConverterRunnerTest::testSymbolsInUnits()
{
    Plasma::RunnerContext context;
    context.setQuery(QStringLiteral("1000 µs as year"));
    runner->match(context);

    QCOMPARE(context.matches().count(), 1);
}

/**
 * Test if negative values are accepted
 */
void ConverterRunnerTest::testNegativeValue()
{
    Plasma::RunnerContext context;
    context.setQuery(QStringLiteral("-4m as cm"));
    runner->match(context);

    QCOMPARE(context.matches().count(), 1);
    QCOMPARE(context.matches().first().text(), "-400 centimeters (cm)");
}

QTEST_MAIN(ConverterRunnerTest)

#include "converterrunnertest.moc"
