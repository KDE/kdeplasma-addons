/***************************************************************************
 *   Copyright (C) 2009 by Björn Ruberg <bjoern@ruberg-wegener.de>         *
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

#include "BoardKey.h"
#include "Helpers.h"
#include <QPainter>
#include <plasma/theme.h>

BoardKey::BoardKey(QPoint relativePosition, QSize relativeSize, unsigned int keycode) :
    m_relativePosition(relativePosition), m_relativeSize(relativeSize), m_keycode(keycode){

}

BoardKey::~BoardKey()
{

}

const bool BoardKey::contains (const QPoint &point) const
{
    return m_rect.contains(point);
}

const bool BoardKey::intersects (const QRectF &rect) const
{
    return m_rect.intersects(rect);
}


const unsigned int BoardKey::getKeycode() const
{
    return m_keycode;
}

const QString BoardKey::label() const
{
    return QString();
}

void BoardKey::paint(QPainter *painter)
{
    painter->eraseRect(m_rect);
    //painter->fillRect(m_rect, QColor(Qt::transparent));
    painter->drawPixmap(m_position, *m_pixmap);
    //painter->drawPixmap(m_rect.toRect(), *m_pixmap);
    //painter->drawRect(QRect(m_position, QPoint( frames[m_size].width() + m_position.x(), frames[m_size].height() + m_position.y() )));

}

const QPoint BoardKey::position() const
{
    return m_position;
}

void BoardKey::pressed()
{

}

void BoardKey::released()
{	
    sendKeycode();
}

const QRectF BoardKey::rect() const
{
    return m_rect;
}

const QSize BoardKey::relativeSize() const
{
    return m_relativeSize;
}

void BoardKey::reset()
{
}

void BoardKey::sendKeycode()
{
    sendKeycodePress();
    sendKeycodeRelease();
}

void BoardKey::sendKeycodePress()
{
     Helpers::fakeKeyPress(getKeycode());
}

void BoardKey::sendKeycodeRelease()
{
    Helpers::fakeKeyRelease(getKeycode());
}

void BoardKey::setPixmap(QPixmap *pixmap)
{
    m_pixmap = pixmap;
}

void BoardKey::setUpPainter(QPainter *painter)
{
    painter->translate(position() + QPoint(size().width()/2, size().height()/2) );
    painter->setBrush(Plasma::Theme::defaultTheme()->color(Plasma::Theme::ButtonTextColor));
}

const QSize BoardKey::size() const
{
    return m_size;
}

void BoardKey::unpressed()
{

}

void BoardKey::updateDimensions(double factor_x, double factor_y)
{
    m_position = QPoint(m_relativePosition.x() * factor_x, m_relativePosition.y() * factor_y);
    m_size = QSize(m_relativeSize.width() * factor_x, m_relativeSize.height() * factor_y);
    m_rect = QRect(m_position, m_size);
}
