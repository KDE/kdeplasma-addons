/*
    SPDX-FileCopyrightText: 2015 Bernhard Friedreich <friesoft@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "timerplugin.h"
#include "timer.h"

// Qt
#include <QQmlEngine>

static QObject *timer_singletontype_provider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    return new Timer();
}

void TimerPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.kde.plasma.private.timer"));

    qmlRegisterSingletonType<Timer>(uri, 0, 1, "Timer", timer_singletontype_provider);
}
