/*
 * SPDX-FileCopyrightText: 2025 Kai Uwe Broulik <kde@broulik.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "noteiconengine.h"

#include <QPainter>
#include <QPixmap>
#include <QSize>

#include <KSvg/Svg>

using namespace Qt::StringLiterals;

NoteIconEngine::NoteIconEngine(const QString &color)
    : QIconEngine()
    , m_color(color)
    , m_svg(std::make_unique<KSvg::Svg>())
{
    m_svg->setImagePath(u"widgets/notes"_s);
    m_svg->setContainsMultipleImages(true);
}

QIconEngine *NoteIconEngine::clone() const
{
    return new NoteIconEngine{m_color};
}

QSize NoteIconEngine::actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    Q_UNUSED(mode);
    Q_UNUSED(state);
    int squareSize = std::min(size.width(), size.height());
    return QSize{squareSize, squareSize};
}

QPixmap NoteIconEngine::pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    return scaledPixmap(size, mode, state, 1.0);
}

QPixmap NoteIconEngine::scaledPixmap(const QSize &size, QIcon::Mode mode, QIcon::State state, qreal scale)
{
    QPixmap pixmap{size * scale};
    pixmap.setDevicePixelRatio(scale);
    pixmap.fill(Qt::transparent);

    QPainter p{&pixmap};
    paint(&p, QRect{QPoint{0, 0}, pixmap.deviceIndependentSize().toSize()}, mode, state);

    return pixmap;
}

void NoteIconEngine::paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state)
{
    Q_UNUSED(mode);
    Q_UNUSED(state);
    m_svg->paint(painter, rect, m_color + "-notes"_L1);
}
