/****************************************************************************
 *   Copyright (C) 2009 by Björn Ruberg <bjoern@ruberg-wegener.de>    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "tooltip.h"
#include <QPainter>
#include <QPaintEvent>
#include <QPalette>
#include <QLabel>
#include <QHBoxLayout>
#include <plasma/theme.h>


Tooltip::Tooltip(const QString &text)
    : QWidget()
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);

    setColors();
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(setColors()));

    frame = new Plasma::FrameSvg(this);
    frame->setEnabledBorders(Plasma::FrameSvg::AllBorders);
    frame->setImagePath("widgets/tooltip");
    frame->resizeFrame(size());

    connect(frame, SIGNAL(repaintNeeded()), this, SLOT(update()));

    label = new QLabel(text, this);
    label->setAlignment(Qt::AlignCenter);

    m_layout = new QHBoxLayout(this);
    m_layout->addWidget(label);
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(updateMask()));
}

Tooltip::~Tooltip()
{

}

void Tooltip::setColors()
{
    QPalette pal = palette();
    pal.setColor(backgroundRole(), Qt::transparent);
    pal.setColor(QPalette::WindowText, Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
    setPalette(pal);
}

void Tooltip::setText(QString text)
{
    label->setText(text);
}

void Tooltip::resizeEvent(QResizeEvent *event)
{
    QSize size = event->size();
    QWidget::resize(size);
    setFont(QFont("Helvetica", qMin(size.height(), size.width()) / 3));
    frame->resizeFrame(size);
    updateMask();
}

void Tooltip::showEvent(QShowEvent * event)
{
    Q_UNUSED(event);
    Plasma::WindowEffects::overrideShadow(winId(), true);
}

void Tooltip::updateMask()
{
    const bool translucency = Plasma::Theme::defaultTheme()->windowTranslucencyEnabled();
    Plasma::WindowEffects::enableBlurBehind(winId(), translucency,
                                            translucency ? frame->mask() : QRegion());
    if (translucency) {
        clearMask();
    } else {
        setMask(frame->mask());
    }
}

void Tooltip::paintEvent(QPaintEvent * event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.setRenderHint(QPainter::Antialiasing);
    //painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(rect(), Qt::transparent);
    frame->paintFrame(&painter, event->rect());
}

#include "tooltip.moc"

