/*
 * SPDX-FileCopyrightText: 2019 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "plugin.h"
#include "inhibitor.h"
#include "monitor.h"

#include <QQmlEngine>

void NightColorControlPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.kde.plasma.private.nightcolorcontrol"));
    qmlRegisterType<Inhibitor>(uri, 1, 0, "Inhibitor");
    qmlRegisterType<Monitor>(uri, 1, 0, "Monitor");
}
