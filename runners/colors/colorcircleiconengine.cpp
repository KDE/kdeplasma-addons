/*
 * SPDX-FileCopyrightText: 2025 Kai Uwe Broulik <kde@broulik.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "colorcircleiconengine.h"

#include <QGuiApplication>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QSize>

using namespace Qt::StringLiterals;

ColorCircleIconEngine::ColorCircleIconEngine(const QColor &color)
    : QIconEngine()
    , m_color(color)
{
}

QIconEngine *ColorCircleIconEngine::clone() const
{
    return new ColorCircleIconEngine(*this);
}

QSize ColorCircleIconEngine::actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    Q_UNUSED(mode);
    Q_UNUSED(state);
    int squareSize = std::min(size.width(), size.height());
    return QSize{squareSize, squareSize};
}

QPixmap ColorCircleIconEngine::pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    return scaledPixmap(size, mode, state, 1.0);
}

QPixmap ColorCircleIconEngine::scaledPixmap(const QSize &size, QIcon::Mode mode, QIcon::State state, qreal scale)
{
    QPixmap pixmap{size * scale};
    pixmap.setDevicePixelRatio(scale);
    pixmap.fill(Qt::transparent);

    QPainter p{&pixmap};
    paint(&p, QRect{QPoint{0, 0}, pixmap.deviceIndependentSize().toSize()}, mode, state);

    return pixmap;
}

void ColorCircleIconEngine::paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state)
{
    Q_UNUSED(mode);
    Q_UNUSED(state);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    // Matches colorpicker plasmoid.
    const qreal strokeWidth = std::max(1.0, std::ceil(std::min(rect.width(), rect.height()) / 20.0));

    const QRectF adjustedRect = QRectF{rect}.adjusted(0.5 + strokeWidth / 2.0, 0.5 + strokeWidth / 2.0, -0.5 - strokeWidth / 2.0, -0.5 - strokeWidth / 2.0);

    QBrush brush(m_color);
    painter->setBrush(brush);

    // TODO use Plasma/Kirigami theme colors.
    QPen pen(qGuiApp->palette().windowText(), strokeWidth);
    painter->setPen(pen);

    // TODO: checkerboard pattern for alpha channel?

    painter->drawEllipse(adjustedRect);

    painter->restore();
}
