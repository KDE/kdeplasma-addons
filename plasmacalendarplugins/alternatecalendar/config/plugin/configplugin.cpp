/*
    SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QQmlEngine>

#include "../../calendarsystem.h"
#include "configstorage.h"

#include <QQmlExtensionPlugin>

class AlternateCalendarConfigPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri) override
    {
        qmlRegisterType<ConfigStorage>(uri, 1, 0, "ConfigStorage");
        qmlRegisterUncreatableType<CalendarSystem>(uri, 1, 0, "CalendarSystem", QStringLiteral("Error: only enums"));
    }
};

#include "configplugin.moc"
