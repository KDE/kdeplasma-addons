/*
 *   Copyright (C) 2007 Tobias Koenig <tokoe@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify  
 *   it under the terms of the GNU General Public License as published by  
 *   the Free Software Foundation; either version 2 of the License, or     
 *   (at your option) any later version.   
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

#include "potdprovider.h"

/**
 * This class provides pictures from the local cache.
 */
class CachedProvider : public PotdProvider
{
    Q_OBJECT

    public:
        /**
         * Creates a new cached provider.
         *
         * @param identifier The identifier of the cached picture.
         * @param parent The parent object.
         */
        CachedProvider( const QString &identifier, QObject *parent, const QVariantList &args );

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
         * Returns the identifier of the picture request (name + date).
         */
        virtual QString identifier() const;

        /**
         * Returns whether a picture with the given @p identifier is cached.
         */
        static bool isCached( const QString &identifier );

        /**
         * Stores the given @p potd with the given @p identifier in the cache.
         */
        static bool storeInCache( const QString &identifier, const QImage &potd );

    private Q_SLOTS:
        void triggerFinished();

    private:
        QString mIdentifier;
};

#endif
