/*
 * Copyright (C) 2015 Dominik Haumann <dhaumann@kde.org>
 * Copyright (C) 2017 David Faure <faure@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "dict_plugin.h"
#include "dict_object.h"
#include "dictionariesmodel.h"

// Qt
#include <QQmlEngine>

void DictPlugin::registerTypes (const char *uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.kde.plasma.private.dict"));
    qmlRegisterType<DictObject>(uri, 1, 0, "DictObject");
    qmlRegisterType<DictionariesModel>(uri, 1, 0, "DictionariesModel");
}
