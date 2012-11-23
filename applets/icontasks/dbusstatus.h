/*
 * Icon Task Manager
 *
 * Copyright 2011 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __DBUS_STATUS_H__
#define __DBUS_STATUS_H__

#include <QtCore/QObject>
#include <QtDBus/QDBusArgument>

struct DBusStatus {      // From Amarok.
    int play;            // Playing = 0, Paused = 1, Stopped = 2
    int random;          // Linearly = 0, Randomly = 1
    int repeat;          // Go_To_Next = 0, Repeat_Current = 1
    int repeat_playlist; // Stop_When_Finished = 0, Never_Give_Up_Playing = 1, Never_Let_You_Down = 42

    enum MprisPlayState {
        Mpris_Playing = 0,
        Mpris_Paused = 1,
        Mpris_Stopped = 2,
    };
};

Q_DECLARE_METATYPE(DBusStatus)
QDBusArgument& operator <<(QDBusArgument& arg, const DBusStatus& status);
const QDBusArgument& operator >>(const QDBusArgument& arg, DBusStatus& status);

#endif
