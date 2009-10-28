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
/*#include <QImage>
#include <QBitmap>*/
#include <plasma/theme.h>


Tooltip::Tooltip(QString text) : QWidget()
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setWindowFlags( Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint );

    setColors();
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(setColorsu()));

    frame = new Plasma::FrameSvg(this);
    frame->setEnabledBorders(Plasma::FrameSvg::AllBorders);
    frame->setImagePath("dialogs/background");
    frame->resizeFrame(size());

    connect(frame, SIGNAL(repaintNeeded()), this, SLOT(update()));

    label = new QLabel(text, this);
    label->setAlignment(Qt::AlignCenter);

    m_layout = new QHBoxLayout(this);
    m_layout->addWidget(label);
}

Tooltip::~Tooltip() {

}

void Tooltip::setColors(){
    QPalette pal = palette();
    pal.setColor(backgroundRole(), Qt::transparent);
    pal.setColor(QPalette::WindowText, Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
    setPalette(pal);
}

void Tooltip::setText(QString text) {
    label->setText(text);
}

void Tooltip::resize(QSize size) {
    QWidget::resize(size);
    setFont(QFont ( "Helvetica", qMin(size.height(),size.width()) / 3) );
    frame->resizeFrame(size);
    /*
    QImage img( size, QImage::Format_Mono);
    img.fill(0);
    setMask(QBitmap::fromImage(img, Qt::MonoOnly));*/
}

void Tooltip::paintEvent ( QPaintEvent * event ){
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.setRenderHint(QPainter::Antialiasing);

    frame->paintFrame(&painter, event->rect());
}

bool Tooltip::event(QEvent *event)
{
    if (event->type() == QEvent::Paint) {
	QPainter painter(this);
	painter.setCompositionMode(QPainter::CompositionMode_Source);
	painter.fillRect(rect(), Qt::transparent);
    }

    return QWidget::event(event);
}
