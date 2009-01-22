/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "Places.h"
#include <KRun>
#include <KLocalizedString>
#include <KDebug>
#include <KIcon>

namespace Models {

Places::Places()
    : BaseModel(true)
{
    load();
}

Places::~Places()
{
}

void Places::load()
{
    // We don't want to use addUrl, because of the icons
    add(
        i18n("Home Folder"),
        qgetenv("HOME"),
        KIcon("user-home"),
        qgetenv("HOME")
    );

    add(
        i18n("Root Folder"),
        "/",
        KIcon("folder-red"),
        "/"
    );

    add(
        i18n("Network Folders"),
        "remote:/",
        KIcon("folder-remote"),
        "remote:/"
    );

}

} // namespace Models
