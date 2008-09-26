/*
 *   Copyright (C) 2007 Marco Martin  <notmart@gmail.com>
 *   derived from DilbertProvider by
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

#ifndef OSNEWSPROVIDER_H
#define OSNEWSPROVIDER_H

#include "comicprovider.h"

#include <syndication/loader.h>
#include <syndication/feed.h>

/**
 * This class provides the comic strip image for osnews.com.
 */
class OsNewsProvider : public ComicProvider
{
    Q_OBJECT

    public:
        /**
         * Creates a new OsNews provider.
         *
         * @param parent The parent object.
         */
        OsNewsProvider( QObject *parent, const QVariantList &args );

        /**
         * Destroys the OsNews provider.
         */
        ~OsNewsProvider();

        /**
         * Returns the identifier type.
         */
        IdentifierType identifierType() const;

        /**
         * Returns the requested image.
         *
         * Note: This method returns only a valid image after the
         *       finished() signal has been emitted.
         */
        virtual QImage image() const;

        /**
         * Returns the identifier of the comic request.
         */
        virtual QString identifier() const;

        /**
         * Returns the website of the comic.
         */
        virtual KUrl websiteUrl() const;

        /**
         * Returns the identifier of the next comic.
         */
        virtual QString nextIdentifier() const;

        /**
         * Returns the identifier of the previous comic.
         */
        virtual QString previousIdentifier() const;

        /**
         * Returns title of this strip.
         */
        virtual QString stripTitle() const;

        /**
         * Returns additionalText of the comic (the tooltip of the image).
         */
        virtual QString additionalText() const;

    protected:
        virtual void pageRetrieved( int id, const QByteArray &data );
        virtual void pageError( int id, const QString &errorMessage );

    private:
        class Private;
        Private* const d;

        Q_PRIVATE_SLOT( d, void processRss( Syndication::Loader* loader,
                                           Syndication::FeedPtr feed,
                                           Syndication::ErrorCode error ) )
};

#endif
