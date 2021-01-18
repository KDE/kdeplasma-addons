/*
 * SPDX-FileCopyrightText: 2015 Dominik Haumann <dhaumann@kde.org>
 * SPDX-FileCopyrightText: 2017 David Faure <faure@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "dict_plugin.h"
#include "dict_object.h"
#include "dictionariesmodel.h"

// Qt

void DictPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.kde.plasma.private.dict"));
    qmlRegisterType<DictObject>(uri, 1, 0, "DictObject");
    qmlRegisterType<DictionariesModel>(uri, 1, 0, "DictionariesModel");
}
