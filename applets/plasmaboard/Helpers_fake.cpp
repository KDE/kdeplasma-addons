/***************************************************************************
 *   Copyright (C) 2009 by Björn Ruberg <bjoern@ruberg-wegener.de>         *
 *   Copyright (C) 2011 by Federico Zenith <federico.zenith@member.fsf.org>*
 *                                                                         *
 *   This program is free software); you can redistribute it and/or modify *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation); either version 2 of the License, or    *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY); without even the implied warranty of       *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program); if not, write to the                        *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/


#include <QChar>
#include <QHash>
#include <QMap>
#include <QString>
#include <QVector>

#include "Helpers.h"

namespace Helpers
{

void changeKeycodeMapping(unsigned int code, QString &sym)
{
    Q_UNUSED(code)
    Q_UNUSED(sym)
}

void changeKeycodeMapping(unsigned int code, QString &sym, QString &shiftedSym)
{
    Q_UNUSED(code)
    Q_UNUSED(sym)
    Q_UNUSED(shiftedSym)
}

void flushPendingKeycodeMappingChanges()
{
}

void saveKeycodeMapping(unsigned int code)
{
    Q_UNUSED(code)
}

void restoreKeycodeMapping(unsigned int code)
{
    Q_UNUSED(code)
}

void refreshXkbState()
{
}

unsigned int keycodeToKeysym(const unsigned int &code, int level)
{
    Q_UNUSED(code)
    Q_UNUSED(level)
    return 0;
}

unsigned int keysymToKeycode(const unsigned int &keysym)
{
    Q_UNUSED(keysym)
    return 0;
}

void fakeKeyPress(const unsigned int &code)
{
    Q_UNUSED(code)
}

void fakeKeyRelease(const unsigned int &code)
{
    Q_UNUSED(code)
}

}
