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


#include "SwitchKey.h"

SwitchKey::SwitchKey(QPoint relativePosition, QSize relativeSize, unsigned int keycode, PlasmaboardWidget* widget)
    : StickyKey(relativePosition, relativeSize, keycode, QString(i18nc("String to indicate that this button shifts layout to special tokens", "?123+"))),
      m_keyboard(widget)
{

}

void SwitchKey::sendKeyPress()
{
    m_keyboard->switchAlternative(true);
}

void SwitchKey::sendKeyRelease()
{
    m_keyboard->switchAlternative(false);
}
