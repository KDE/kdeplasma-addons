/*/***************************************************************************
 *   Copyright (C) 2010 by Björn Ruberg <bjoern@ruberg-wegener.de>    *
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


#include "StickyKey.h"

StickyKey::StickyKey(QPoint relativePosition, QSize relativeSize, unsigned int keycode, QString label)
    : FuncKey(relativePosition, relativeSize, keycode, label),
      m_toggled(false)
{
}

void StickyKey::pressed()
{
    if (!m_toggled) {
        sendKeyPress(); // if the key has not pressed, send immediately a press to X server
    }
}

void StickyKey::released()
{
    if (m_toggled) {
        sendKeyRelease();
    }

    m_toggled = !m_toggled;
}

void StickyKey::reset()
{
    if (m_toggled) {
        sendKeyRelease();
        m_toggled = false;
    }
}

void StickyKey::setPixmap(QPixmap *pixmap)
{
    // if toggled we want to keep the pressed pixmap
    if (!m_toggled) {
        FuncKey::setPixmap(pixmap);
    }
}

