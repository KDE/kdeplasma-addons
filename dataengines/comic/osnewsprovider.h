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
 * This class provides the comic strip image for penny-arcade.com.
 */
class OsNewsProvider : public ComicProvider
{
    Q_OBJECT

    public:
        /**
         * Creates a new OsNews provider.
         *
         * @param date The date for which the image shall be fetched.
         * @param parent The parent object.
         */
        OsNewsProvider( const QDate &date, QObject *parent = 0 );

        /**
         * Destroys the OsNews provider.
         */
        ~OsNewsProvider();

        //Reimplementations
        virtual QImage image() const;

        virtual QString identifier() const;

        virtual KUrl websiteUrl() const;

        virtual QString nextIdentifierSuffix() const;

        virtual QString previousIdentifierSuffix() const;

    private:
      class Private;
      Private* const d;

      Q_PRIVATE_SLOT( d, void processRss(Syndication::Loader* loader,
                           Syndication::FeedPtr feed,
                           Syndication::ErrorCode error) )
      Q_PRIVATE_SLOT( d, void pageRequestFinished( bool ) )
      Q_PRIVATE_SLOT( d, void imageRequestFinished( bool ) )
};

#endif
