/*
 *   Copyright (C) 2007 Tobias Koenig <tokoe@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef CACHEDPROVIDER_H
#define CACHEDPROVIDER_H

#include "comicprovider.h"

#include <QtCore/QHash>

/**
 * This class provides comics from the local cache.
 */
class CachedProvider : public ComicProvider
{
    Q_OBJECT

    public:
        /**
         * Creates a new cached provider.
         *
         * @param identifier The identifier of the cached comic.
         * @param parent The parent object.
         */
        explicit CachedProvider( QObject *parent, const QVariantList &args = QVariantList() );

        /**
         * Destroys the cached provider.
         */
        ~CachedProvider();

        /**
         * Returns the identifier type.
         *
         * Is always StringIdentifier here.
         */
        IdentifierType identifierType() const;

        /**
         * Returns the type of identifier that is used by this
         * comic provider.
         */
        virtual QString suffixType() const;

        /**
         * Returns the requested image.
         *
         * Note: This method returns only a valid image after the
         *       finished() signal has been emitted.
         */
        virtual QImage image() const;

        /**
         * Returns the identifier of the comic request (name + date).
         */
        virtual QString identifier() const;

        /**
         * Returns the identifier suffix of the next comic.
         */
        virtual QString nextIdentifier() const;

        /**
         * Returns the identifier suffix of the previous comic.
         */
        virtual QString previousIdentifier() const;

        /**
         * Returns the identifier of the first strip.
         */
        virtual QString firstStripIdentifier() const;

        /**
         * Returns the identifier of the last cached strip.
         */
        QString lastCachedStripIdentifier() const;

        /**
         * Returns the title of the strip.
         */
        virtual QString stripTitle() const;

        /**
         * Returns the author of the comic.
         */
        virtual QString comicAuthor() const;

        /**
         * Returns additionalText of the comic.
         */
        virtual QString additionalText() const;

        /**
         * Returns the name for the comic
         */
        virtual QString name() const;

        /**
        * Returns wether the comic is leftToRight or not
        */
        virtual bool isLeftToRight() const;

        /**
        * Returns wether the comic is topToBottom or not
        */
        virtual bool isTopToBottom() const;

        /**
         * Returns whether a comic with the given @p identifier is cached.
         */
        static bool isCached( const QString &identifier );

        /**
         * Map of keys and values to store in the config file for an individual identifier
         */
        typedef QHash<QString, QString> Settings;

        /**
         * Stores the given @p comic with the given @p identifier in the cache.
         */
        static bool storeInCache( const QString &identifier, const QImage &comic, const Settings &info = Settings() );

        /**
         * Returns the website of the comic.
         */
        virtual KUrl websiteUrl() const;

        virtual KUrl imageUrl() const;

        /**
         * Returns the shop website of the comic.
         */
        virtual KUrl shopUrl() const;

        /**
          * Returns the maximum number of cached strips per comic, -1 means that there is no limit
          * @note defaulte is -1
          */
        static int maxComicLimit();

        /**
          * Sets the maximum number of cached strips per comic, -1 means that there is no limit
          */
        static void setMaxComicLimit( int limit );

    private Q_SLOTS:
        void triggerFinished();

    private:
        static const int CACHE_DEFAULT;
};

#endif
