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

BoardKey::BoardKey(QPoint relativePosition, QSize relativeSize, unsigned int keycode)
    : m_keycode(keycode),
      m_pixmap(0),
      m_relativePosition(relativePosition),
      m_relativeSize(relativeSize)
{
}

BoardKey::~BoardKey()
{
}

bool BoardKey::contains(const QPoint &point) const
{
    return m_rect.contains(point);
}

bool BoardKey::intersects(const QRectF &rect) const
{
    return m_rect.intersects(rect);
}

unsigned int BoardKey::keycode() const
{
    return m_keycode;
}

unsigned int BoardKey::keysymbol(int level) const
{
    return Helpers::keycodeToKeysym(keycode(), level);
}

QString BoardKey::label() const
{
    return QString();
}

void BoardKey::paint(QPainter *painter)
{
    if (m_pixmap) {
        painter->drawPixmap(m_rect.topLeft(), *m_pixmap);
    }
}

QPoint BoardKey::position() const
{
    return m_rect.topLeft().toPoint();
}

void BoardKey::pressed()
{

}

bool BoardKey::repeats() const
{
    return true;
}

void BoardKey::pressRepeated()
{
    if (repeats()) {
        Helpers::fakeKeyPress(keycode());
    }
}

void BoardKey::released()
{
    sendKey();
}

QRectF BoardKey::rect() const
{
    return m_rect;
}

QSize BoardKey::relativeSize() const
{
    return m_relativeSize;
}

void BoardKey::reset()
{
}

void BoardKey::sendKey()
{
    sendKeyPress();
    sendKeyRelease();
}

void BoardKey::sendKeyPress()
{
    Helpers::fakeKeyPress(keycode());
}

void BoardKey::sendKeyRelease()
{
    Helpers::fakeKeyRelease(keycode());
}

void BoardKey::setKeycode(unsigned int keycode)
{
    m_keycode = keycode;
}

bool BoardKey::setPixmap(QPixmap *pixmap)
{
    if (m_pixmap == pixmap) {
        return false;
    }

    m_pixmap = pixmap;
    return true;
}

void BoardKey::setUpPainter(QPainter *painter) const
{
    painter->translate(position() + QPoint(size().width() / 2, size().height() / 2));
}

QSize BoardKey::size() const
{
    return m_rect.size().toSize();
}

void BoardKey::updateDimensions(double factor_x, double factor_y)
{
    const QPoint position(m_relativePosition.x() * factor_x, m_relativePosition.y() * factor_y);
    const QSize size(m_relativeSize.width() * factor_x, m_relativeSize.height() * factor_y);
    m_rect = QRect(position, size);
}
