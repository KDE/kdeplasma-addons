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
        CachedProvider( const QString &identifier, QObject *parent = 0 );

        /**
         * Destroys the cached provider.
         */
        ~CachedProvider();

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
         * Returns whether a comic with the given @p identifier is cached.
         */
        static bool isCached( const QString &identifier );

        /**
         * Stores the given @p comic with the given @p identifier in the cache.
         */
        static bool storeInCache( const QString &identifier, const QImage &comic );

    private Q_SLOTS:
        void triggerFinished();

    private:
        QString mIdentifier;
};

#endif
