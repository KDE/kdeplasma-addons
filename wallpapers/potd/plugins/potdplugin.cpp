/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "potdplugin.h"
#include "potdprovidermodel.h"
#include <QQmlContext>

void PotdPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QByteArrayLiteral("org.kde.plasma.wallpapers.potd"));

    qmlRegisterType<PotdProviderModel>(uri, 1, 0, "PotdProviderModel");
    qmlRegisterUncreatableType<PotdProviderModel>(uri, 1, 0, "Global", QStringLiteral("Error: only enums"));
}
