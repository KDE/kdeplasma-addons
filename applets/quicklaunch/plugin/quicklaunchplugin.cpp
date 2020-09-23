/*
 *  SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "quicklaunchplugin.h"
#include "quicklaunch_p.h"

// Qt
#include <QQmlEngine>

void QuicklaunchPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.kde.plasma.private.quicklaunch"));

    qmlRegisterType<QuicklaunchPrivate>(uri, 1, 0, "Logic");
}
