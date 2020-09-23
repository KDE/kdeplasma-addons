/*
 * SPDX-FileCopyrightText: 2015 Dominik Haumann <dhaumann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "plugin.h"
#include "DiskQuota.h"
#include "QuotaListModel.h"

// Qt
#include <QQmlEngine>

void DiskQuotaPlugin::registerTypes (const char *uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.kde.plasma.private.diskquota"));
    qmlRegisterType<DiskQuota>(uri, 1, 0, "DiskQuota");
    qmlRegisterType<QuotaListModel>(uri, 1, 0, "QuotaListModel");
}
