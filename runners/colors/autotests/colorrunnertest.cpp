/*
 * SPDX-FileCopyrightText: 2025 Kai Uwe Broulik <kde@broulik.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include <KRunner/AbstractRunnerTest>

#include <QColor>
#include <QTest>

using namespace KRunner;
using namespace Qt::StringLiterals;

class ColorRunnerTest : public AbstractRunnerTest
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void test_data();
    void test();
    void testInvalid_data();
    void testInvalid();
};

void ColorRunnerTest::initTestCase()
{
    initProperties();
}

void ColorRunnerTest::test_data()
{
    QTest::addColumn<QString>("query");
    QTest::addColumn<QColor>("expectedColor");
    QTest::addColumn<QString>("expectedText");

    QTest::newRow("hex caps") << u"#53A3D8"_s << QColor(0x53A3D8) << u"#53a3d8"_s;
    QTest::newRow("hex lower") << u"#53a3d8"_s << QColor(0x53A3D8) << u"#53a3d8"_s;
    QTest::newRow("hex mixed") << u"#53a3D8"_s << QColor(0x53A3D8) << u"#53a3d8"_s;

    // NOTE QColor constructor taking QRgb calls fromRgb and discards the alpha channel...
    QTest::newRow("hex alpha") << u"#8053A3D8"_s << QColor::fromRgba(0x8053A3D8) << u"#8053a3d8"_s;

    QTest::newRow("hex 3 letters") << u"#5A9"_s << QColor(0x55AA99) << u"#55aa99"_s;

    QTest::newRow("no prefix hex") << u"53A3D8"_s << QColor(0x53A3D8) << u"#53a3d8"_s;
    QTest::newRow("0x prefix hex") << u"0x53A3D8"_s << QColor(0x53A3D8) << u"#53a3d8"_s;

    QTest::newRow("named input lower") << u"fuchsia"_s << QColor(0xff00ff) << u"fuchsia"_s;
    QTest::newRow("named input upper") << u"FUCHSIA"_s << QColor(0xff00ff) << u"fuchsia"_s;
    QTest::newRow("named input mixed") << u"fUcHsIA"_s << QColor(0xff00ff) << u"fuchsia"_s;
    QTest::newRow("named output") << u"#ff00ff"_s << QColor(0xff00ff) << u"fuchsia"_s;

    QTest::newRow("different name") << u"cyan"_s << QColor(0x00ffff) << u"aqua"_s;

    QTest::newRow("numbers spaces") << u"83 163 216"_s << QColor(0x53A3D8) << u"#53a3d8"_s;
    QTest::newRow("numbers comma") << u"83,163,216"_s << QColor(0x53A3D8) << u"#53a3d8"_s;
    QTest::newRow("numbers comma and spaces") << u"83, 163, 216"_s << QColor(0x53A3D8) << u"#53a3d8"_s;
    QTest::newRow("numbers comma and excessive spaces") << u"    83,     163,     216    "_s << QColor(0x53A3D8) << u"#53a3d8"_s;

    QTest::newRow("rgb compact") << u"rgb83 163 216"_s << QColor(0x53A3D8) << u"#53a3d8"_s;
    QTest::newRow("rgb spaces") << u"rgb 83 163 216"_s << QColor(0x53A3D8) << u"#53a3d8"_s;
    QTest::newRow("rgb comma") << u"rgb 83,163,216"_s << QColor(0x53A3D8) << u"#53a3d8"_s;
    QTest::newRow("rgb comma and spaces") << u"rgb 83, 163, 216"_s << QColor(0x53A3D8) << u"#53a3d8"_s;
    QTest::newRow("rgb comma and excessive spaces") << u"rgb     83,    163,    216   "_s << QColor(0x53A3D8) << u"#53a3d8"_s;
    QTest::newRow("rgb parentheses") << u"rgb(83 163 216)"_s << QColor(0x53A3D8) << u"#53a3d8"_s;
    QTest::newRow("rgb parentheses inner padding") << u"rgb( 83 163 216 )"_s << QColor(0x53A3D8) << u"#53a3d8"_s;
    QTest::newRow("rgb parentheses outer padding") << u"rgb (83 163 216) "_s << QColor(0x53A3D8) << u"#53a3d8"_s;
    QTest::newRow("rgb parentheses comma") << u"rgb(83,163,216)"_s << QColor(0x53A3D8) << u"#53a3d8"_s;
    QTest::newRow("rgb parentheses comma and spaces") << u"rgb(83, 163, 216)"_s << QColor(0x53A3D8) << u"#53a3d8"_s;
    QTest::newRow("rgb parentheses missing closing") << u"rgb(83, 163, 216"_s << QColor(0x53A3D8) << u"#53a3d8"_s;

    QTest::newRow("numbers alpha spaces") << u"83 163 216 128"_s << QColor::fromRgba(0x8053A3D8) << u"#8053a3d8"_s;
    QTest::newRow("numbers alpha comma") << u"83,163,216,128"_s << QColor::fromRgba(0x8053A3D8) << u"#8053a3d8"_s;
    QTest::newRow("numbers alpha comma and spaces") << u"83, 163, 216, 128"_s << QColor::fromRgba(0x8053A3D8) << u"#8053a3d8"_s;

    QTest::newRow("rgba compact") << u"rgba83 163 216 128"_s << QColor::fromRgba(0x8053A3D8) << u"#8053a3d8"_s;
    QTest::newRow("rgba spaces") << u"rgba 83 163 216 128"_s << QColor::fromRgba(0x8053A3D8) << u"#8053a3d8"_s;
    QTest::newRow("rgba comma") << u"rgba 83,163,216,128"_s << QColor::fromRgba(0x8053A3D8) << u"#8053a3d8"_s;
    QTest::newRow("rgba comma and spaces") << u"rgba 83, 163, 216, 128"_s << QColor::fromRgba(0x8053A3D8) << u"#8053a3d8"_s;
}

void ColorRunnerTest::test()
{
    QFETCH(QString, query);
    QFETCH(QColor, expectedColor);
    QFETCH(QString, expectedText);

    launchQuery(query);
    const auto matches = manager->matches();
    QCOMPARE(matches.count(), 1);

    const auto match = matches.first();
    QCOMPARE(match.data().metaType(), QMetaType::fromType<QColor>());
    QCOMPARE(match.data().value<QColor>(), expectedColor);
    QCOMPARE(match.text(), expectedText);
}

void ColorRunnerTest::testInvalid_data()
{
    QTest::addColumn<QString>("query");

    QTest::newRow("hex") << "#53A3DG";
    QTest::newRow("no prefix hex") << "53A3DG";
    QTest::newRow("0x prefix hex") << "0x53A3DG";

    QTest::newRow("named input") << u"winered"_s;

    QTest::newRow("numbers excessive") << u"83 163 216 128 123"_s;

    QTest::newRow("rgb too few") << u"rgb 123 123"_s;
    QTest::newRow("rgb too many") << u"rgb 123 123 123 123"_s;
    QTest::newRow("rgb out of range") << u"rgb 123 456 789"_s;
    QTest::newRow("rgb negative") << u"rgb -123 -123 -123"_s;
    QTest::newRow("rgb parentheses too few") << u"rgb(123 123)"_s;
    QTest::newRow("rgb parentheses too many") << u"rgb(123 123 123 123)"_s;

    QTest::newRow("rgba too few") << u"rgba 123 123 123"_s;
    QTest::newRow("rgba too many") << u"rgba 123 123 123 123 123"_s;
}

void ColorRunnerTest::testInvalid()
{
    QFETCH(QString, query);

    launchQuery(query);
    const auto matches = manager->matches();
    QCOMPARE(matches.count(), 0);
}

QTEST_MAIN(ColorRunnerTest)

#include "colorrunnertest.moc"
