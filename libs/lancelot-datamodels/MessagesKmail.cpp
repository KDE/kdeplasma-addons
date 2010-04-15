/*
 *   Copyright (C) 2007, 2008, 2009, 2010 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser/Library General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser/Library General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser/Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "MessagesKmail.h"
#include <KIcon>

#include <KRun>
#include <KStandardDirs>

#include "Logger.h"
//#include "config-lancelot-datamodels.h"

namespace Lancelot {
namespace Models {

#ifndef LANCELOT_DATAMODELS_HAS_PIMLIBS

// We don't have akonadi and pimlibs, thus, the model is
// showing only kmail icon

MessagesKmail::MessagesKmail()
    : d(NULL)
{
    setSelfTitle(i18n("Unread messages"));
    setSelfIcon(KIcon("kmail"));

    load();
}

MessagesKmail::~MessagesKmail()
{
}

void MessagesKmail::activate(int index)
{
    Q_UNUSED(index)
}

void MessagesKmail::load()
{
    if (!addService("kontact") && !addService("kmail")) {
        add(i18n("Unable to find Kontact"), "",
                KIcon("application-x-executable"), QVariant("http://kontact.kde.org"));
    }
}

#else

#error "Not implemented yet"


#endif

} // namespace Models
} // namespace Lancelot
