/*
 *   SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef COMIC_ENGINE_TYPES
#define COMIC_ENGINE_TYPES

#include <QIcon>
#include <QString>
#include <QUrl>

struct ComicProviderInfo {
    QString pluginId;
    QString name;
    QString icon;
};

struct ComicMetaData {
    QString stripTitle;
    QUrl imageUrl;
    QImage image;
    QUrl websiteUrl;
    QUrl shopUrl;
    QString firstStripIdentifier;
    QString previousStripIdentifier;
    QString nextIdentifier;
    QString previousIdentifier;
    QString author;
    QString comicAuthor;
    QString additionalText;
    QString identifier;
    QString suffixType;
    bool isLeftToRight = false;
    bool isTopToBottom = false;
    QString lastCachedStripIdentifier;
    QString providerName;
    bool error = false;
    bool errorAutomaticallyFixable = false;
};

#endif
