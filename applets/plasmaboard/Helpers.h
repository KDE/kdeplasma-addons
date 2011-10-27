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

#ifndef HELPERS_H
#define HELPERS_H

#include <X11/XKBlib.h>

class QChar;
class QString;

namespace Helpers
{
void changeKeycodeMapping(unsigned int code, QString &sym);

void changeKeycodeMapping(unsigned int code, QString &sym,
                          QString &shiftedSym);

//void changeKeycodeMapping(unsigned int code, KeySym* keysyms);
void flushPendingKeycodeMappingChanges();

//QVector<KeySym> getKeycodeMapping(unsigned int code);
void saveKeycodeMapping(unsigned int code);
void restoreKeycodeMapping(unsigned int code);
void refreshXkbState();

unsigned int keycodeToKeysym(const unsigned int &code, int level);

unsigned int keysymToKeycode(const unsigned int &keysym);

void fakeKeyPress(const unsigned int &code);

void fakeKeyRelease(const unsigned int &code);

QChar mapToUnicode(const unsigned int &keysym);

}

#endif /* HELPERS_H */
