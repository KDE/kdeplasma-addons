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


#include "DualKey.h"
#include "Helpers.h"

DualKey::DualKey(QPoint relativePosition, QSize relativeSize, unsigned int keycode, QString altKeyString):
        AlphaNumKey(relativePosition, relativeSize, keycode), m_altKeyString(altKeyString), m_shiftedAltKeyString(altKeyString), m_defaultMapping(0), m_isAlt(false)
{

}

DualKey::DualKey(QPoint relativePosition, QSize relativeSize, unsigned int keycode, QString altKeyString, QString shiftedAltKeyString):
        AlphaNumKey(relativePosition, relativeSize, keycode), m_altKeyString(altKeyString), m_shiftedAltKeyString(shiftedAltKeyString), m_defaultMapping(0), m_isAlt(false)
{

}

DualKey::~DualKey()
{
    XFree(m_defaultMapping);
}

bool DualKey::alternative() const
{
    return m_isAlt;
}

void DualKey::setAlternative(bool alt)
{
    if(alt){
        m_defaultMapping = Helpers::getKeycodeMapping(getKeycode());
        Helpers::changeKeycodeMapping(getKeycode(), m_altKeyString, m_shiftedAltKeyString);
    }
    else {
        Helpers::changeKeycodeMapping(getKeycode(), (KeySym*) m_defaultMapping);
    }
    m_isAlt = alt;
}
