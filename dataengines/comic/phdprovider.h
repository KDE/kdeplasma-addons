/*
 *   Copyright (C) 2008 Stefan Majewsky <majewsky@gmx.net>
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

#ifndef PHDPROVIDER_H
#define PHDPROVIDER_H

#include "comicprovider.h"

/**
 * This class provides the comic strip image for www.phdcomics.com.
 */
class PhdProvider : public ComicProvider
{
    Q_OBJECT

    public:
        /**
         * Creates a new phdcomics provider.
         *
         * @param parent The parent object.
         */
        PhdProvider( QObject *parent, const QVariantList &args );

        /**
         * Destroys the phdcomics provider.
         */
        ~PhdProvider();

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
         * Returns the identifier of the comic request (name + date).
         */
        virtual QString identifier() const;

        virtual KUrl websiteUrl() const;

        virtual QString nextIdentifier() const;

        virtual QString previousIdentifier() const;

    private:
      class Private;
      Private* const d;

      Q_PRIVATE_SLOT( d, void pageRequestFinished( bool ) )
      Q_PRIVATE_SLOT( d, void imageRequestFinished( bool ) )
};

#endif
