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
#include <qx11info_x11.h>
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>

#include "Helpers.h"

namespace Helpers
{

int keysymsPerKeycode;

/// Contains the non-predictable keysyms and their corresponding QChar's
QHash<unsigned int, QChar> symbolMap;
QHash<unsigned int, QVector<KeySym> > savedMappings;
QMap<unsigned int, QVector<KeySym> > pendingKeycodeChanges;
XkbStateRec xkbState;
bool xkbStateSetup = false;

/// Initialises the keysym-QChar map
void initialiseMap(QHash<unsigned int, QChar>& map);

void changeKeycodeMapping(unsigned int code, QString &sym)
{
    KeySym keysym = XStringToKeysym(sym.toAscii());
    QVector<KeySym> keysyms(keysymsPerKeycode);
    for (int i = 0; i < keysymsPerKeycode; ++i) {
        keysyms[i] = keysym;
    }

    pendingKeycodeChanges.insert(code, keysyms);
}

void changeKeycodeMapping(unsigned int code, QString &sym, QString &shiftedSym)
{
    KeySym keysym = XStringToKeysym(sym.toAscii());
    QVector<KeySym> keysyms(keysymsPerKeycode);
    for (int i = 0; i < keysymsPerKeycode; ++i) {
        keysyms[i] = keysym;
    }

    keysyms[1] = XStringToKeysym(shiftedSym.toAscii());
    pendingKeycodeChanges.insert(code, keysyms);
}

void flushPendingKeycodeMappingChanges()
{
    QVector<KeySym> keysyms;

    QMapIterator<unsigned int, QVector<KeySym> > it(pendingKeycodeChanges);
    unsigned int startingCode = 0;
    unsigned int lastCode = 0;
    int count = 0;
    while (it.hasNext()) {
        it.next();
        if (startingCode == 0) {
            startingCode = it.key();
        } else if (lastCode + 1 != it.key()) {
            XChangeKeyboardMapping(QX11Info::display(), startingCode, keysymsPerKeycode, keysyms.data(), count);

            keysyms.clear();
            startingCode = it.key();
            count = 0;
        }

        lastCode = it.key();
        keysyms += it.value();
        ++count;
    }

    pendingKeycodeChanges.clear();
    XChangeKeyboardMapping(QX11Info::display(), startingCode, keysymsPerKeycode, keysyms.data(), count);
    XSync(QX11Info::display(), False);
}

void saveKeycodeMapping(unsigned int code)
{
    KeySym *syms = XGetKeyboardMapping(QX11Info::display(), code, 1, &keysymsPerKeycode);
    QVector<KeySym> v(keysymsPerKeycode);
    for (int i = 0; i < keysymsPerKeycode; ++i) {
        v[i] = syms[i];
    }
    XFree(syms);
    savedMappings.insert(code, v);
}

void restoreKeycodeMapping(unsigned int code)
{
    if (savedMappings.contains(code)) {
        pendingKeycodeChanges.insert(code, savedMappings[code]);
    }
}

void refreshXkbState()
{
    XkbGetState(QX11Info::display(), XkbUseCoreKbd, &xkbState);
    xkbStateSetup = true;
}

unsigned int keycodeToKeysym(const unsigned int &code, int level)
{
    if (!xkbStateSetup) {
        refreshXkbState();
    }

    int vector = xkbState.group * 2 + level;
    return (unsigned int)XKeycodeToKeysym(QX11Info::display(), code, vector);
}

unsigned int keysymToKeycode(const unsigned int &keysym)
{
    return ((unsigned int) XKeysymToKeycode(QX11Info::display(), keysym));
}

void fakeKeyPress(const unsigned int &code)
{
    XTestFakeKeyEvent(QX11Info::display(), code, true, 0);
    XSync(QX11Info::display(), False);
}

void fakeKeyRelease(const unsigned int &code)
{
    XTestFakeKeyEvent(QX11Info::display(), code, false, 0);
    XSync(QX11Info::display(), False);
}

/**
 * This function maps keysyms to QChars exploiting as long as possible the
 * current situation: if the keysym is one of the "guessable" ones, i.e.
 * less than 0x100 or larger than 0x01000100, it calculates the
 * corresponding Unicode value and returns the corresponding QChar.
 * Otherwise, it looks the keysym up in the symbolMap, and if it is found
 * it returns the QChar, otherwise the null QChar.
 *
 * Note that symbolMap is lazily initialised only the first time it is
 * needed, so on advanced X implementations that do away with all old-age,
 * non-Unicodish keysyms the map initialisation will never waste CPU time.
 *
 * \sa http://www.x.org/wiki/KeySyms
 */
QChar mapToUnicode(const unsigned int &keysym)
{
    if (keysym < 0x100) {
        return QChar(keysym);
    } else if (keysym > 0x01000100) {
        return QChar(keysym - 0x01000000);
    } else {
        if (symbolMap.isEmpty()) {
            initialiseMap(symbolMap);
        }

        if (symbolMap.contains(keysym)) {
            return symbolMap[keysym];
        }
    }

    return QChar(0x2204); // Symbol for "There does not exist"
}

/**
 * The data can be extracted from the keysymdef.h file, in most cases this
 * will be at /usr/include/X11/keysymdef.h.
 *
 * BEWARE OF EDITING THIS FUNCTION MANUALLY. It will be a beast to maintain.
 * Note that, in theory, the body of this function should never change as
 * all new X characters will be added, consistently with Unicode standards,
 * with keysyms above 0x01000100.
 *
 * The script to generate the full body of this function is:
 *
 * #!/bin/bash
 * # Select only lines defining a macro
 * grep "^#" /usr/include/X11/keysymdef.h | \
 *     # Take only values that have a Unicode equivalent
 *     grep "/\* U+" | \
 *     # Extract the codes (in parentheses)
 *     sed -r "s/^.*0x(\w*)\s.*U\+(\w+)\s.*$/map[0x\1] = QChar( 0x\2 );/" | \
 *     # Remove all codes starting with 0x00 (less than 0x100)
 *     # or with 0x100 (larger than 0x1000100)
 *     grep -v -E "^map\[0x1?00" | \
 *     # Put some order to ease manual lookup
 *    sort
 */
void initialiseMap(QHash<unsigned int, QChar>& map)
{
    map[0x01a1] = QChar(0x0104);
    map[0x01a2] = QChar(0x02D8);
    map[0x01a3] = QChar(0x0141);
    map[0x01a5] = QChar(0x013D);
    map[0x01a6] = QChar(0x015A);
    map[0x01a9] = QChar(0x0160);
    map[0x01aa] = QChar(0x015E);
    map[0x01ab] = QChar(0x0164);
    map[0x01ac] = QChar(0x0179);
    map[0x01ae] = QChar(0x017D);
    map[0x01af] = QChar(0x017B);
    map[0x01b1] = QChar(0x0105);
    map[0x01b2] = QChar(0x02DB);
    map[0x01b3] = QChar(0x0142);
    map[0x01b5] = QChar(0x013E);
    map[0x01b6] = QChar(0x015B);
    map[0x01b7] = QChar(0x02C7);
    map[0x01b9] = QChar(0x0161);
    map[0x01ba] = QChar(0x015F);
    map[0x01bb] = QChar(0x0165);
    map[0x01bc] = QChar(0x017A);
    map[0x01bd] = QChar(0x02DD);
    map[0x01be] = QChar(0x017E);
    map[0x01bf] = QChar(0x017C);
    map[0x01c0] = QChar(0x0154);
    map[0x01c3] = QChar(0x0102);
    map[0x01c5] = QChar(0x0139);
    map[0x01c6] = QChar(0x0106);
    map[0x01c8] = QChar(0x010C);
    map[0x01ca] = QChar(0x0118);
    map[0x01cc] = QChar(0x011A);
    map[0x01cf] = QChar(0x010E);
    map[0x01d0] = QChar(0x0110);
    map[0x01d1] = QChar(0x0143);
    map[0x01d2] = QChar(0x0147);
    map[0x01d5] = QChar(0x0150);
    map[0x01d8] = QChar(0x0158);
    map[0x01d9] = QChar(0x016E);
    map[0x01db] = QChar(0x0170);
    map[0x01de] = QChar(0x0162);
    map[0x01e0] = QChar(0x0155);
    map[0x01e3] = QChar(0x0103);
    map[0x01e5] = QChar(0x013A);
    map[0x01e6] = QChar(0x0107);
    map[0x01e8] = QChar(0x010D);
    map[0x01ea] = QChar(0x0119);
    map[0x01ec] = QChar(0x011B);
    map[0x01ef] = QChar(0x010F);
    map[0x01f0] = QChar(0x0111);
    map[0x01f1] = QChar(0x0144);
    map[0x01f2] = QChar(0x0148);
    map[0x01f5] = QChar(0x0151);
    map[0x01f8] = QChar(0x0159);
    map[0x01f9] = QChar(0x016F);
    map[0x01fb] = QChar(0x0171);
    map[0x01fe] = QChar(0x0163);
    map[0x01ff] = QChar(0x02D9);
    map[0x02a1] = QChar(0x0126);
    map[0x02a6] = QChar(0x0124);
    map[0x02a9] = QChar(0x0130);
    map[0x02ab] = QChar(0x011E);
    map[0x02ac] = QChar(0x0134);
    map[0x02b1] = QChar(0x0127);
    map[0x02b6] = QChar(0x0125);
    map[0x02b9] = QChar(0x0131);
    map[0x02bb] = QChar(0x011F);
    map[0x02bc] = QChar(0x0135);
    map[0x02c5] = QChar(0x010A);
    map[0x02c6] = QChar(0x0108);
    map[0x02d5] = QChar(0x0120);
    map[0x02d8] = QChar(0x011C);
    map[0x02dd] = QChar(0x016C);
    map[0x02de] = QChar(0x015C);
    map[0x02e5] = QChar(0x010B);
    map[0x02e6] = QChar(0x0109);
    map[0x02f5] = QChar(0x0121);
    map[0x02f8] = QChar(0x011D);
    map[0x02fd] = QChar(0x016D);
    map[0x02fe] = QChar(0x015D);
    map[0x03a2] = QChar(0x0138);
    map[0x03a3] = QChar(0x0156);
    map[0x03a5] = QChar(0x0128);
    map[0x03a6] = QChar(0x013B);
    map[0x03aa] = QChar(0x0112);
    map[0x03ab] = QChar(0x0122);
    map[0x03ac] = QChar(0x0166);
    map[0x03b3] = QChar(0x0157);
    map[0x03b5] = QChar(0x0129);
    map[0x03b6] = QChar(0x013C);
    map[0x03ba] = QChar(0x0113);
    map[0x03bb] = QChar(0x0123);
    map[0x03bc] = QChar(0x0167);
    map[0x03bd] = QChar(0x014A);
    map[0x03bf] = QChar(0x014B);
    map[0x03c0] = QChar(0x0100);
    map[0x03c7] = QChar(0x012E);
    map[0x03cc] = QChar(0x0116);
    map[0x03cf] = QChar(0x012A);
    map[0x03d1] = QChar(0x0145);
    map[0x03d2] = QChar(0x014C);
    map[0x03d3] = QChar(0x0136);
    map[0x03d9] = QChar(0x0172);
    map[0x03dd] = QChar(0x0168);
    map[0x03de] = QChar(0x016A);
    map[0x03e0] = QChar(0x0101);
    map[0x03e7] = QChar(0x012F);
    map[0x03ec] = QChar(0x0117);
    map[0x03ef] = QChar(0x012B);
    map[0x03f1] = QChar(0x0146);
    map[0x03f2] = QChar(0x014D);
    map[0x03f3] = QChar(0x0137);
    map[0x03f9] = QChar(0x0173);
    map[0x03fd] = QChar(0x0169);
    map[0x03fe] = QChar(0x016B);
    map[0x047e] = QChar(0x203E);
    map[0x04a1] = QChar(0x3002);
    map[0x04a2] = QChar(0x300C);
    map[0x04a3] = QChar(0x300D);
    map[0x04a4] = QChar(0x3001);
    map[0x04a5] = QChar(0x30FB);
    map[0x04a6] = QChar(0x30F2);
    map[0x04a7] = QChar(0x30A1);
    map[0x04a8] = QChar(0x30A3);
    map[0x04a9] = QChar(0x30A5);
    map[0x04aa] = QChar(0x30A7);
    map[0x04ab] = QChar(0x30A9);
    map[0x04ac] = QChar(0x30E3);
    map[0x04ad] = QChar(0x30E5);
    map[0x04ae] = QChar(0x30E7);
    map[0x04af] = QChar(0x30C3);
    map[0x04b0] = QChar(0x30FC);
    map[0x04b1] = QChar(0x30A2);
    map[0x04b2] = QChar(0x30A4);
    map[0x04b3] = QChar(0x30A6);
    map[0x04b4] = QChar(0x30A8);
    map[0x04b5] = QChar(0x30AA);
    map[0x04b6] = QChar(0x30AB);
    map[0x04b7] = QChar(0x30AD);
    map[0x04b8] = QChar(0x30AF);
    map[0x04b9] = QChar(0x30B1);
    map[0x04ba] = QChar(0x30B3);
    map[0x04bb] = QChar(0x30B5);
    map[0x04bc] = QChar(0x30B7);
    map[0x04bd] = QChar(0x30B9);
    map[0x04be] = QChar(0x30BB);
    map[0x04bf] = QChar(0x30BD);
    map[0x04c0] = QChar(0x30BF);
    map[0x04c1] = QChar(0x30C1);
    map[0x04c2] = QChar(0x30C4);
    map[0x04c3] = QChar(0x30C6);
    map[0x04c4] = QChar(0x30C8);
    map[0x04c5] = QChar(0x30CA);
    map[0x04c6] = QChar(0x30CB);
    map[0x04c7] = QChar(0x30CC);
    map[0x04c8] = QChar(0x30CD);
    map[0x04c9] = QChar(0x30CE);
    map[0x04ca] = QChar(0x30CF);
    map[0x04cb] = QChar(0x30D2);
    map[0x04cc] = QChar(0x30D5);
    map[0x04cd] = QChar(0x30D8);
    map[0x04ce] = QChar(0x30DB);
    map[0x04cf] = QChar(0x30DE);
    map[0x04d0] = QChar(0x30DF);
    map[0x04d1] = QChar(0x30E0);
    map[0x04d2] = QChar(0x30E1);
    map[0x04d3] = QChar(0x30E2);
    map[0x04d4] = QChar(0x30E4);
    map[0x04d5] = QChar(0x30E6);
    map[0x04d6] = QChar(0x30E8);
    map[0x04d7] = QChar(0x30E9);
    map[0x04d8] = QChar(0x30EA);
    map[0x04d9] = QChar(0x30EB);
    map[0x04da] = QChar(0x30EC);
    map[0x04db] = QChar(0x30ED);
    map[0x04dc] = QChar(0x30EF);
    map[0x04dd] = QChar(0x30F3);
    map[0x04de] = QChar(0x309B);
    map[0x04df] = QChar(0x309C);
    map[0x05ac] = QChar(0x060C);
    map[0x05bb] = QChar(0x061B);
    map[0x05bf] = QChar(0x061F);
    map[0x05c1] = QChar(0x0621);
    map[0x05c2] = QChar(0x0622);
    map[0x05c3] = QChar(0x0623);
    map[0x05c4] = QChar(0x0624);
    map[0x05c5] = QChar(0x0625);
    map[0x05c6] = QChar(0x0626);
    map[0x05c7] = QChar(0x0627);
    map[0x05c8] = QChar(0x0628);
    map[0x05c9] = QChar(0x0629);
    map[0x05ca] = QChar(0x062A);
    map[0x05cb] = QChar(0x062B);
    map[0x05cc] = QChar(0x062C);
    map[0x05cd] = QChar(0x062D);
    map[0x05ce] = QChar(0x062E);
    map[0x05cf] = QChar(0x062F);
    map[0x05d0] = QChar(0x0630);
    map[0x05d1] = QChar(0x0631);
    map[0x05d2] = QChar(0x0632);
    map[0x05d3] = QChar(0x0633);
    map[0x05d4] = QChar(0x0634);
    map[0x05d5] = QChar(0x0635);
    map[0x05d6] = QChar(0x0636);
    map[0x05d7] = QChar(0x0637);
    map[0x05d8] = QChar(0x0638);
    map[0x05d9] = QChar(0x0639);
    map[0x05da] = QChar(0x063A);
    map[0x05e0] = QChar(0x0640);
    map[0x05e1] = QChar(0x0641);
    map[0x05e2] = QChar(0x0642);
    map[0x05e3] = QChar(0x0643);
    map[0x05e4] = QChar(0x0644);
    map[0x05e5] = QChar(0x0645);
    map[0x05e6] = QChar(0x0646);
    map[0x05e7] = QChar(0x0647);
    map[0x05e8] = QChar(0x0648);
    map[0x05e9] = QChar(0x0649);
    map[0x05ea] = QChar(0x064A);
    map[0x05eb] = QChar(0x064B);
    map[0x05ec] = QChar(0x064C);
    map[0x05ed] = QChar(0x064D);
    map[0x05ee] = QChar(0x064E);
    map[0x05ef] = QChar(0x064F);
    map[0x05f0] = QChar(0x0650);
    map[0x05f1] = QChar(0x0651);
    map[0x05f2] = QChar(0x0652);
    map[0x06a1] = QChar(0x0452);
    map[0x06a2] = QChar(0x0453);
    map[0x06a3] = QChar(0x0451);
    map[0x06a4] = QChar(0x0454);
    map[0x06a5] = QChar(0x0455);
    map[0x06a6] = QChar(0x0456);
    map[0x06a7] = QChar(0x0457);
    map[0x06a8] = QChar(0x0458);
    map[0x06a9] = QChar(0x0459);
    map[0x06aa] = QChar(0x045A);
    map[0x06ab] = QChar(0x045B);
    map[0x06ac] = QChar(0x045C);
    map[0x06ad] = QChar(0x0491);
    map[0x06ae] = QChar(0x045E);
    map[0x06af] = QChar(0x045F);
    map[0x06b0] = QChar(0x2116);
    map[0x06b1] = QChar(0x0402);
    map[0x06b2] = QChar(0x0403);
    map[0x06b3] = QChar(0x0401);
    map[0x06b4] = QChar(0x0404);
    map[0x06b5] = QChar(0x0405);
    map[0x06b6] = QChar(0x0406);
    map[0x06b7] = QChar(0x0407);
    map[0x06b8] = QChar(0x0408);
    map[0x06b9] = QChar(0x0409);
    map[0x06ba] = QChar(0x040A);
    map[0x06bb] = QChar(0x040B);
    map[0x06bc] = QChar(0x040C);
    map[0x06bd] = QChar(0x0490);
    map[0x06be] = QChar(0x040E);
    map[0x06bf] = QChar(0x040F);
    map[0x06c0] = QChar(0x044E);
    map[0x06c1] = QChar(0x0430);
    map[0x06c2] = QChar(0x0431);
    map[0x06c3] = QChar(0x0446);
    map[0x06c4] = QChar(0x0434);
    map[0x06c5] = QChar(0x0435);
    map[0x06c6] = QChar(0x0444);
    map[0x06c7] = QChar(0x0433);
    map[0x06c8] = QChar(0x0445);
    map[0x06c9] = QChar(0x0438);
    map[0x06ca] = QChar(0x0439);
    map[0x06cb] = QChar(0x043A);
    map[0x06cc] = QChar(0x043B);
    map[0x06cd] = QChar(0x043C);
    map[0x06ce] = QChar(0x043D);
    map[0x06cf] = QChar(0x043E);
    map[0x06d0] = QChar(0x043F);
    map[0x06d1] = QChar(0x044F);
    map[0x06d2] = QChar(0x0440);
    map[0x06d3] = QChar(0x0441);
    map[0x06d4] = QChar(0x0442);
    map[0x06d5] = QChar(0x0443);
    map[0x06d6] = QChar(0x0436);
    map[0x06d7] = QChar(0x0432);
    map[0x06d8] = QChar(0x044C);
    map[0x06d9] = QChar(0x044B);
    map[0x06da] = QChar(0x0437);
    map[0x06db] = QChar(0x0448);
    map[0x06dc] = QChar(0x044D);
    map[0x06dd] = QChar(0x0449);
    map[0x06de] = QChar(0x0447);
    map[0x06df] = QChar(0x044A);
    map[0x06e0] = QChar(0x042E);
    map[0x06e1] = QChar(0x0410);
    map[0x06e2] = QChar(0x0411);
    map[0x06e3] = QChar(0x0426);
    map[0x06e4] = QChar(0x0414);
    map[0x06e5] = QChar(0x0415);
    map[0x06e6] = QChar(0x0424);
    map[0x06e7] = QChar(0x0413);
    map[0x06e8] = QChar(0x0425);
    map[0x06e9] = QChar(0x0418);
    map[0x06ea] = QChar(0x0419);
    map[0x06eb] = QChar(0x041A);
    map[0x06ec] = QChar(0x041B);
    map[0x06ed] = QChar(0x041C);
    map[0x06ee] = QChar(0x041D);
    map[0x06ef] = QChar(0x041E);
    map[0x06f0] = QChar(0x041F);
    map[0x06f1] = QChar(0x042F);
    map[0x06f2] = QChar(0x0420);
    map[0x06f3] = QChar(0x0421);
    map[0x06f4] = QChar(0x0422);
    map[0x06f5] = QChar(0x0423);
    map[0x06f6] = QChar(0x0416);
    map[0x06f7] = QChar(0x0412);
    map[0x06f8] = QChar(0x042C);
    map[0x06f9] = QChar(0x042B);
    map[0x06fa] = QChar(0x0417);
    map[0x06fb] = QChar(0x0428);
    map[0x06fc] = QChar(0x042D);
    map[0x06fd] = QChar(0x0429);
    map[0x06fe] = QChar(0x0427);
    map[0x06ff] = QChar(0x042A);
    map[0x07a1] = QChar(0x0386);
    map[0x07a2] = QChar(0x0388);
    map[0x07a3] = QChar(0x0389);
    map[0x07a4] = QChar(0x038A);
    map[0x07a5] = QChar(0x03AA);
    map[0x07a7] = QChar(0x038C);
    map[0x07a8] = QChar(0x038E);
    map[0x07a9] = QChar(0x03AB);
    map[0x07ab] = QChar(0x038F);
    map[0x07ae] = QChar(0x0385);
    map[0x07af] = QChar(0x2015);
    map[0x07b1] = QChar(0x03AC);
    map[0x07b2] = QChar(0x03AD);
    map[0x07b3] = QChar(0x03AE);
    map[0x07b4] = QChar(0x03AF);
    map[0x07b5] = QChar(0x03CA);
    map[0x07b6] = QChar(0x0390);
    map[0x07b7] = QChar(0x03CC);
    map[0x07b8] = QChar(0x03CD);
    map[0x07b9] = QChar(0x03CB);
    map[0x07ba] = QChar(0x03B0);
    map[0x07bb] = QChar(0x03CE);
    map[0x07c1] = QChar(0x0391);
    map[0x07c2] = QChar(0x0392);
    map[0x07c3] = QChar(0x0393);
    map[0x07c4] = QChar(0x0394);
    map[0x07c5] = QChar(0x0395);
    map[0x07c6] = QChar(0x0396);
    map[0x07c7] = QChar(0x0397);
    map[0x07c8] = QChar(0x0398);
    map[0x07c9] = QChar(0x0399);
    map[0x07ca] = QChar(0x039A);
    map[0x07cb] = QChar(0x039B);
    map[0x07cb] = QChar(0x039B);
    map[0x07cc] = QChar(0x039C);
    map[0x07cd] = QChar(0x039D);
    map[0x07ce] = QChar(0x039E);
    map[0x07cf] = QChar(0x039F);
    map[0x07d0] = QChar(0x03A0);
    map[0x07d1] = QChar(0x03A1);
    map[0x07d2] = QChar(0x03A3);
    map[0x07d4] = QChar(0x03A4);
    map[0x07d5] = QChar(0x03A5);
    map[0x07d6] = QChar(0x03A6);
    map[0x07d7] = QChar(0x03A7);
    map[0x07d8] = QChar(0x03A8);
    map[0x07d9] = QChar(0x03A9);
    map[0x07e1] = QChar(0x03B1);
    map[0x07e2] = QChar(0x03B2);
    map[0x07e3] = QChar(0x03B3);
    map[0x07e4] = QChar(0x03B4);
    map[0x07e5] = QChar(0x03B5);
    map[0x07e6] = QChar(0x03B6);
    map[0x07e7] = QChar(0x03B7);
    map[0x07e8] = QChar(0x03B8);
    map[0x07e9] = QChar(0x03B9);
    map[0x07ea] = QChar(0x03BA);
    map[0x07eb] = QChar(0x03BB);
    map[0x07eb] = QChar(0x03BB);
    map[0x07ec] = QChar(0x03BC);
    map[0x07ed] = QChar(0x03BD);
    map[0x07ee] = QChar(0x03BE);
    map[0x07ef] = QChar(0x03BF);
    map[0x07f0] = QChar(0x03C0);
    map[0x07f1] = QChar(0x03C1);
    map[0x07f2] = QChar(0x03C3);
    map[0x07f3] = QChar(0x03C2);
    map[0x07f4] = QChar(0x03C4);
    map[0x07f5] = QChar(0x03C5);
    map[0x07f6] = QChar(0x03C6);
    map[0x07f7] = QChar(0x03C7);
    map[0x07f8] = QChar(0x03C8);
    map[0x07f9] = QChar(0x03C9);
    map[0x08a1] = QChar(0x23B7);
    map[0x08a4] = QChar(0x2320);
    map[0x08a5] = QChar(0x2321);
    map[0x08a7] = QChar(0x23A1);
    map[0x08a8] = QChar(0x23A3);
    map[0x08a9] = QChar(0x23A4);
    map[0x08aa] = QChar(0x23A6);
    map[0x08ab] = QChar(0x239B);
    map[0x08ac] = QChar(0x239D);
    map[0x08ad] = QChar(0x239E);
    map[0x08ae] = QChar(0x23A0);
    map[0x08af] = QChar(0x23A8);
    map[0x08b0] = QChar(0x23AC);
    map[0x08bc] = QChar(0x2264);
    map[0x08bd] = QChar(0x2260);
    map[0x08be] = QChar(0x2265);
    map[0x08bf] = QChar(0x222B);
    map[0x08c0] = QChar(0x2234);
    map[0x08c1] = QChar(0x221D);
    map[0x08c2] = QChar(0x221E);
    map[0x08c5] = QChar(0x2207);
    map[0x08c8] = QChar(0x223C);
    map[0x08c9] = QChar(0x2243);
    map[0x08cd] = QChar(0x21D4);
    map[0x08ce] = QChar(0x21D2);
    map[0x08cf] = QChar(0x2261);
    map[0x08d6] = QChar(0x221A);
    map[0x08da] = QChar(0x2282);
    map[0x08db] = QChar(0x2283);
    map[0x08dc] = QChar(0x2229);
    map[0x08dd] = QChar(0x222A);
    map[0x08de] = QChar(0x2227);
    map[0x08df] = QChar(0x2228);
    map[0x08ef] = QChar(0x2202);
    map[0x08f6] = QChar(0x0192);
    map[0x08fb] = QChar(0x2190);
    map[0x08fc] = QChar(0x2191);
    map[0x08fd] = QChar(0x2192);
    map[0x08fe] = QChar(0x2193);
    map[0x09e0] = QChar(0x25C6);
    map[0x09e1] = QChar(0x2592);
    map[0x09e2] = QChar(0x2409);
    map[0x09e3] = QChar(0x240C);
    map[0x09e4] = QChar(0x240D);
    map[0x09e5] = QChar(0x240A);
    map[0x09e8] = QChar(0x2424);
    map[0x09e9] = QChar(0x240B);
    map[0x09ea] = QChar(0x2518);
    map[0x09eb] = QChar(0x2510);
    map[0x09ec] = QChar(0x250C);
    map[0x09ed] = QChar(0x2514);
    map[0x09ee] = QChar(0x253C);
    map[0x09ef] = QChar(0x23BA);
    map[0x09f0] = QChar(0x23BB);
    map[0x09f1] = QChar(0x2500);
    map[0x09f2] = QChar(0x23BC);
    map[0x09f3] = QChar(0x23BD);
    map[0x09f4] = QChar(0x251C);
    map[0x09f5] = QChar(0x2524);
    map[0x09f6] = QChar(0x2534);
    map[0x09f7] = QChar(0x252C);
    map[0x09f8] = QChar(0x2502);
    map[0x0aa1] = QChar(0x2003);
    map[0x0aa2] = QChar(0x2002);
    map[0x0aa3] = QChar(0x2004);
    map[0x0aa4] = QChar(0x2005);
    map[0x0aa5] = QChar(0x2007);
    map[0x0aa6] = QChar(0x2008);
    map[0x0aa7] = QChar(0x2009);
    map[0x0aa8] = QChar(0x200A);
    map[0x0aa9] = QChar(0x2014);
    map[0x0aaa] = QChar(0x2013);
    map[0x0aae] = QChar(0x2026);
    map[0x0aaf] = QChar(0x2025);
    map[0x0ab0] = QChar(0x2153);
    map[0x0ab1] = QChar(0x2154);
    map[0x0ab2] = QChar(0x2155);
    map[0x0ab3] = QChar(0x2156);
    map[0x0ab4] = QChar(0x2157);
    map[0x0ab5] = QChar(0x2158);
    map[0x0ab6] = QChar(0x2159);
    map[0x0ab7] = QChar(0x215A);
    map[0x0ab8] = QChar(0x2105);
    map[0x0abb] = QChar(0x2012);
    map[0x0ac3] = QChar(0x215B);
    map[0x0ac4] = QChar(0x215C);
    map[0x0ac5] = QChar(0x215D);
    map[0x0ac6] = QChar(0x215E);
    map[0x0ac9] = QChar(0x2122);
    map[0x0ad0] = QChar(0x2018);
    map[0x0ad1] = QChar(0x2019);
    map[0x0ad2] = QChar(0x201C);
    map[0x0ad3] = QChar(0x201D);
    map[0x0ad4] = QChar(0x211E);
    map[0x0ad6] = QChar(0x2032);
    map[0x0ad7] = QChar(0x2033);
    map[0x0ad9] = QChar(0x271D);
    map[0x0aec] = QChar(0x2663);
    map[0x0aed] = QChar(0x2666);
    map[0x0aee] = QChar(0x2665);
    map[0x0af0] = QChar(0x2720);
    map[0x0af1] = QChar(0x2020);
    map[0x0af2] = QChar(0x2021);
    map[0x0af3] = QChar(0x2713);
    map[0x0af4] = QChar(0x2717);
    map[0x0af5] = QChar(0x266F);
    map[0x0af6] = QChar(0x266D);
    map[0x0af7] = QChar(0x2642);
    map[0x0af8] = QChar(0x2640);
    map[0x0af9] = QChar(0x260E);
    map[0x0afa] = QChar(0x2315);
    map[0x0afb] = QChar(0x2117);
    map[0x0afc] = QChar(0x2038);
    map[0x0afd] = QChar(0x201A);
    map[0x0afe] = QChar(0x201E);
    map[0x0bc2] = QChar(0x22A4);
    map[0x0bc4] = QChar(0x230A);
    map[0x0bca] = QChar(0x2218);
    map[0x0bcc] = QChar(0x2395);
    map[0x0bce] = QChar(0x22A5);
    map[0x0bcf] = QChar(0x25CB);
    map[0x0bd3] = QChar(0x2308);
    map[0x0bdc] = QChar(0x22A3);
    map[0x0bfc] = QChar(0x22A2);
    map[0x0cdf] = QChar(0x2017);
    map[0x0ce0] = QChar(0x05D0);
    map[0x0ce1] = QChar(0x05D1);
    map[0x0ce2] = QChar(0x05D2);
    map[0x0ce3] = QChar(0x05D3);
    map[0x0ce4] = QChar(0x05D4);
    map[0x0ce5] = QChar(0x05D5);
    map[0x0ce6] = QChar(0x05D6);
    map[0x0ce7] = QChar(0x05D7);
    map[0x0ce8] = QChar(0x05D8);
    map[0x0ce9] = QChar(0x05D9);
    map[0x0cea] = QChar(0x05DA);
    map[0x0ceb] = QChar(0x05DB);
    map[0x0cec] = QChar(0x05DC);
    map[0x0ced] = QChar(0x05DD);
    map[0x0cee] = QChar(0x05DE);
    map[0x0cef] = QChar(0x05DF);
    map[0x0cf0] = QChar(0x05E0);
    map[0x0cf1] = QChar(0x05E1);
    map[0x0cf2] = QChar(0x05E2);
    map[0x0cf3] = QChar(0x05E3);
    map[0x0cf4] = QChar(0x05E4);
    map[0x0cf5] = QChar(0x05E5);
    map[0x0cf6] = QChar(0x05E6);
    map[0x0cf7] = QChar(0x05E7);
    map[0x0cf8] = QChar(0x05E8);
    map[0x0cf9] = QChar(0x05E9);
    map[0x0cfa] = QChar(0x05EA);
    map[0x0da1] = QChar(0x0E01);
    map[0x0da2] = QChar(0x0E02);
    map[0x0da3] = QChar(0x0E03);
    map[0x0da4] = QChar(0x0E04);
    map[0x0da5] = QChar(0x0E05);
    map[0x0da6] = QChar(0x0E06);
    map[0x0da7] = QChar(0x0E07);
    map[0x0da8] = QChar(0x0E08);
    map[0x0da9] = QChar(0x0E09);
    map[0x0daa] = QChar(0x0E0A);
    map[0x0dab] = QChar(0x0E0B);
    map[0x0dac] = QChar(0x0E0C);
    map[0x0dad] = QChar(0x0E0D);
    map[0x0dae] = QChar(0x0E0E);
    map[0x0daf] = QChar(0x0E0F);
    map[0x0db0] = QChar(0x0E10);
    map[0x0db1] = QChar(0x0E11);
    map[0x0db2] = QChar(0x0E12);
    map[0x0db3] = QChar(0x0E13);
    map[0x0db4] = QChar(0x0E14);
    map[0x0db5] = QChar(0x0E15);
    map[0x0db6] = QChar(0x0E16);
    map[0x0db7] = QChar(0x0E17);
    map[0x0db8] = QChar(0x0E18);
    map[0x0db9] = QChar(0x0E19);
    map[0x0dba] = QChar(0x0E1A);
    map[0x0dbb] = QChar(0x0E1B);
    map[0x0dbc] = QChar(0x0E1C);
    map[0x0dbd] = QChar(0x0E1D);
    map[0x0dbe] = QChar(0x0E1E);
    map[0x0dbf] = QChar(0x0E1F);
    map[0x0dc0] = QChar(0x0E20);
    map[0x0dc1] = QChar(0x0E21);
    map[0x0dc2] = QChar(0x0E22);
    map[0x0dc3] = QChar(0x0E23);
    map[0x0dc4] = QChar(0x0E24);
    map[0x0dc5] = QChar(0x0E25);
    map[0x0dc6] = QChar(0x0E26);
    map[0x0dc7] = QChar(0x0E27);
    map[0x0dc8] = QChar(0x0E28);
    map[0x0dc9] = QChar(0x0E29);
    map[0x0dca] = QChar(0x0E2A);
    map[0x0dcb] = QChar(0x0E2B);
    map[0x0dcc] = QChar(0x0E2C);
    map[0x0dcd] = QChar(0x0E2D);
    map[0x0dce] = QChar(0x0E2E);
    map[0x0dcf] = QChar(0x0E2F);
    map[0x0dd0] = QChar(0x0E30);
    map[0x0dd1] = QChar(0x0E31);
    map[0x0dd2] = QChar(0x0E32);
    map[0x0dd3] = QChar(0x0E33);
    map[0x0dd4] = QChar(0x0E34);
    map[0x0dd5] = QChar(0x0E35);
    map[0x0dd6] = QChar(0x0E36);
    map[0x0dd7] = QChar(0x0E37);
    map[0x0dd8] = QChar(0x0E38);
    map[0x0dd9] = QChar(0x0E39);
    map[0x0dda] = QChar(0x0E3A);
    map[0x0ddf] = QChar(0x0E3F);
    map[0x0de0] = QChar(0x0E40);
    map[0x0de1] = QChar(0x0E41);
    map[0x0de2] = QChar(0x0E42);
    map[0x0de3] = QChar(0x0E43);
    map[0x0de4] = QChar(0x0E44);
    map[0x0de5] = QChar(0x0E45);
    map[0x0de6] = QChar(0x0E46);
    map[0x0de7] = QChar(0x0E47);
    map[0x0de8] = QChar(0x0E48);
    map[0x0de9] = QChar(0x0E49);
    map[0x0dea] = QChar(0x0E4A);
    map[0x0deb] = QChar(0x0E4B);
    map[0x0dec] = QChar(0x0E4C);
    map[0x0ded] = QChar(0x0E4D);
    map[0x0df0] = QChar(0x0E50);
    map[0x0df1] = QChar(0x0E51);
    map[0x0df2] = QChar(0x0E52);
    map[0x0df3] = QChar(0x0E53);
    map[0x0df4] = QChar(0x0E54);
    map[0x0df5] = QChar(0x0E55);
    map[0x0df6] = QChar(0x0E56);
    map[0x0df7] = QChar(0x0E57);
    map[0x0df8] = QChar(0x0E58);
    map[0x0df9] = QChar(0x0E59);
    map[0x13bc] = QChar(0x0152);
    map[0x13bd] = QChar(0x0153);
    map[0x13be] = QChar(0x0178);
    map[0x20ac] = QChar(0x20AC);
}
}
