/*
 *  SPDX-FileCopyrightText: 2015 Lars Pontoppidan <dev.larpon@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "mediaframeplugin.h"
#include "mediaframe.h"

// Qt

void MediaFramePlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.kde.plasma.private.mediaframe"));

    qmlRegisterType<MediaFrame>(uri, 2, 0, "MediaFrame");
}
