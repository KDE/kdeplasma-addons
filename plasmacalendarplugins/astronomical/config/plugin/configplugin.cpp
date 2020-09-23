/*
    SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "configplugin.h"

#include "configstorage.h"

// Qt
#include <QQmlEngine>


void AstronomicalConfigPlugin::registerTypes(const char* uri)
{
    qmlRegisterType<ConfigStorage>(uri, 1, 0, "ConfigStorage");
}
