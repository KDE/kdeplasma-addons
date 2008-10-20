#ifndef HEADER_CPP
#define HEADER_CPP
/*
 *   Copyright 2007, 2008  Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "header.h"
#include <kdebug.h>
#include <kicon.h>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsWidget>
#include <plasma/svg.h>

using namespace Plasma;

Header::Header(QGraphicsItem *parent) :
        QGraphicsWidget(parent),
        m_icon(0)
{
    setMaximumSize(QSizeF(600, 48));
    setMinimumSize(QSizeF(100, 48));

    m_icon = new Plasma::Svg(this);
    m_icon->setImagePath("rssnow/rssnow");
    m_icon->resize(96, 48);
    m_icon->setContainsMultipleImages(false);

    setPreferredSize(QSizeF(300, 48));
    setMinimumSize(QSizeF(96, 48));
    setMaximumSize(QSizeF(1000, 48));
}

void Header::paint(QPainter *p,
                   const QStyleOptionGraphicsItem *option,
                   QWidget *widget)
{
    Q_UNUSED(widget);
    Q_UNUSED(option);

    p->setRenderHint(QPainter::SmoothPixmapTransform);

    m_icon->paint( p, 0, 0, "RSSNOW" );
}

#include "header.moc"
#endif // HEADER.CPP

