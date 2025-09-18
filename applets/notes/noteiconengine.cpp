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

static std::shared_ptr<KSvg::Svg> noteSvg()
{
    static std::weak_ptr<KSvg::Svg> s_instance;
    if (s_instance.expired()) {
        auto svg = std::make_shared<KSvg::Svg>();
        svg->setImagePath(u"widgets/notes"_s);
        svg->setContainsMultipleImages(true);
        s_instance = svg;
        return svg;
    }
    return s_instance.lock();
}

NoteIconEngine::NoteIconEngine(const QString &color)
    : QIconEngine()
    , m_color(color)
    , m_svg(noteSvg())
{
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
