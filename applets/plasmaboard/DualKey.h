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


#ifndef DUALKEY_H
#define DUALKEY_H

#include "AlphaNumKey.h"

class DualKey : public AlphaNumKey
{
public:
    DualKey(QPoint relativePosition, QSize relativeSize, unsigned int keycode, QString altKeyString);
    DualKey(QPoint relativePosition, QSize relativeSize, unsigned int keycode, QString altKeyString, QString shiftedAltKeyString);
    virtual ~DualKey();

    bool alternative() const;
    void setAlternative(bool alt);

private:
    //virtual void paintLabel(QPainter *painter);

private:
    QString m_altKeyString; // String (Xlibs key name) of the alternative key on this
    QString m_shiftedAltKeyString; // shifted alternative key
    bool m_isAlt;
};

#endif // DUALKEY_H
