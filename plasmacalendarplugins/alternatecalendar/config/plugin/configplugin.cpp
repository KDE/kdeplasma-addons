/*
    SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "configplugin.h"

#include <QQmlEngine>

#include "../../calendarsystem.h"
#include "configstorage.h"

void AlternateCalendarConfigPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QByteArray("org.kde.plasmacalendar.alternatecalendarconfig"));

    qmlRegisterType<ConfigStorage>(uri, 1, 0, "ConfigStorage");
    qmlRegisterUncreatableType<CalendarSystem>(uri, 1, 0, "CalendarSystem", QStringLiteral("Error: only enums"));
}
