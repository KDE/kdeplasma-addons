/*
 * SPDX-FileCopyrightText: 2025 Kai Uwe Broulik <kde@broulik.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "colorsrunner.h"

#include "colorcircleiconengine.h"

#include <KLocalizedString>
#include <KRunner/QueryMatch>

#include <QClipboard>
#include <QColor>
#include <QGuiApplication>
#include <QIcon>
#include <QMimeData>

#include <algorithm>

using namespace Qt::StringLiterals;

ColorsRunner::ColorsRunner(QObject *parent, const KPluginMetaData &metaData)
    : AbstractRunner(parent, metaData)
{
    setObjectName(QStringLiteral("ColorsRunner"));

    addSyntax(QStringLiteral(":q:"), i18n("Diplays the value of :q: as a color when :q: is a hexadecimal color (e.g. #FF00FF) or an SVG named color."));

    addSyntax(QStringLiteral("rgb(:q:)"), i18n("Displays the value of :q: as a color when :q: is an RGB color triplet (e.g. 255,255,255)."));

    addSyntax(QStringLiteral("rgba(:q:)"),
              i18n("Displays the value of :q: as a color with alpha channel when :q: is an RGBA color quadruplet (e.g. 255,255,255,255)."));
}

ColorsRunner::~ColorsRunner() = default;

void ColorsRunner::match(RunnerContext &context)
{
    const QStringView queryView{context.query()};
    // Need at least #123, shortest color name is "red".
    if (queryView.length() < 3) {
        return;
    }

    // Qt parses hex or named color.
    QColor color{queryView};

    // Allow without # prefix.
    if (!color.isValid() && (queryView.size() == 6 || queryView.size() == 8)) {
        color = QColor{'#'_L1 + context.query()};
    }

    // Allow 0x instead of # prefix.
    if (!color.isValid() && queryView.startsWith("0x"_L1)) {
        color = QColor{'#'_L1 + context.query().mid(2)};
    }

    // Try to parse rgb notation.
    if (!color.isValid()) {
        // Match rgb or rgba with 3 or 4 groups of 3 digits, separated by space(s) and/or a comma.
        static QRegularExpression regExp(R"(^(rgba?)?\s*\(?\s*(\d{1,3})[,\s]\s*(\d{1,3})[,\s]\s*(\d{1,3})[,\s]{0,1}\s*(\d{1,3})?\)?$)");
        const auto match = regExp.matchView(queryView);
        if (match.hasMatch()) {
            enum class AlphaBehavior {
                Auto,
                Mandatory,
                Forbidden
            } alphaBehavior = AlphaBehavior::Auto;

            // When rgb(a) prefix is present, alpha must or must not be present.
            // When just typing numbers, it can be whatever.
            const auto prefix = match.capturedView(1);
            if (prefix.startsWith("rgb"_L1)) {
                if (prefix == "rgba"_L1) {
                    alphaBehavior = AlphaBehavior::Mandatory;
                } else {
                    alphaBehavior = AlphaBehavior::Forbidden;
                }
            }

            int r = -1;
            int g = -1;
            int b = -1;
            int a = 255;

            int capture = 2; // 0 is entire string, 1 is "rgb(a)".
            bool ok;
            for (int *out : {&r, &g, &b, &a}) {
                const bool isAlpha = (out == &a);
                const int value = match.capturedView(capture).toInt(&ok);
                if (!isAlpha && !ok) {
                    return;
                }
                if (isAlpha) {
                    if ((alphaBehavior == AlphaBehavior::Mandatory && !ok) || (alphaBehavior == AlphaBehavior::Forbidden && ok)) {
                        return;
                    }
                }
                if (ok) {
                    *out = value;
                }
                ++capture;
            }

            color = QColor{r, g, b, a};
        }
    }

    if (!color.isValid()) {
        return;
    }

    QueryMatch match{this};
    match.setMultiLine(true);

    // Unfortunately there is no way to get the named color from an existing QColor.
    const auto names = QColor::colorNames();
    for (const QString &name : names) {
        if (QColor{name} == color) {
            match.setText(name);
            break;
        }
    }

    const bool hasAlpha = color.alphaF() < 1;
    const QString colorDisplayHex = hasAlpha ? color.name(QColor::HexArgb) : color.name(QColor::HexRgb);

    QStringList descriptions;

    if (match.text().isEmpty()) {
        match.setText(colorDisplayHex);
    } else {
        descriptions.append(colorDisplayHex);
    }

    if (hasAlpha) {
        descriptions.append(i18n("RGBA: %1, %2, %3, %4", color.red(), color.green(), color.blue(), color.alpha()));
    } else {
        descriptions.append(i18n("RGB: %1, %2, %3", color.red(), color.green(), color.blue()));
    }

    float c, m, y, k;
    color.getCmykF(&c, &m, &y, &k);
    descriptions.append(i18n("CMYK: %1%, %2%, %3%, %4%", std::round(c * 100), std::round(m * 100), std::round(y * 100), std::round(k * 100)));

    // descriptions.append(i18n("HSV: %1°, %2%, %3%", std::round(color.hueF() * 360), std::round(color.saturationF() * 100), std::round(color.valueF() * 100)));
    // descriptions.append(i18n("HSL: %1°, %2%, %3%", std::round(color.hslHueF() * 360), std::round(color.hslSaturationF() * 100), std::round(color.lightnessF()
    // * 100)));

    match.setSubtext(descriptions.join('\n'_L1));

    match.setData(color);

    match.setIcon(QIcon{new ColorCircleIconEngine{color}});

    context.addMatch(match);
}

void ColorsRunner::run(const RunnerContext &context, const QueryMatch &match)
{
    Q_UNUSED(context)
    QGuiApplication::clipboard()->setMimeData(mimeDataForMatch(match));
}

QMimeData *ColorsRunner::mimeDataForMatch(const QueryMatch &match)
{
    const QColor color = match.data().value<QColor>();

    auto *mimeData = new QMimeData();
    mimeData->setColorData(color);
    mimeData->setText(color.alphaF() < 1 ? color.name(QColor::HexArgb) : color.name(QColor::HexRgb));
    return mimeData;
}

K_PLUGIN_CLASS_WITH_JSON(ColorsRunner, "plasma-runner-colors.json")

#include "colorsrunner.moc"
