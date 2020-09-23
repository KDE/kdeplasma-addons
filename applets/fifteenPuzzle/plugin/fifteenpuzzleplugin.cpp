/*
 * SPDX-FileCopyrightText: 2014 Jeremy Whiting <jpwhiting@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "fifteenpuzzleplugin.h"
#include "fifteenimageprovider.h"

// Qt
#include <QQmlEngine>
#include <QDebug>

void FifteenPuzzlePlugin::registerTypes(const char* uri)
{
    Q_UNUSED(uri);
    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.kde.plasma.private.fifteenpuzzle"));
    // Do some dummy registration, otherwise the plugin will be ignored at runtime
    qmlRegisterTypeNotAvailable(uri, 0, 1, "FifteenPuzzle", QStringLiteral("fifteenpuzzle"));
}

void FifteenPuzzlePlugin::initializeEngine(QQmlEngine * engine, const char * uri)
{
    qDebug() << "initializeEngine called, uri is " << uri;
    engine->addImageProvider(QStringLiteral("fifteenpuzzle"), new FifteenImageProvider());
}
