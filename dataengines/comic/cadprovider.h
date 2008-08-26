/*
*   Copyright (C) 2008 Matthias Fuchs <mat69@gmx.net>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License as
*   published by the Free Software Foundation; either version 2, or
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

#ifndef CADPROVIDER_H
#define CADPROVIDER_H

#include "comicprovider.h"

/**
 * This class provides the comic strip image for http://cad-comic.com.
 */
class CadProvider : public ComicProvider
{
    Q_OBJECT

    public:
        /**
         * Creates a new ctrl+alt+del provider.
         *
         * @param parent The parent object.
         */
        CadProvider( QObject *parent, const QVariantList& );

        /**
         * Destroys the ctrl+alt+del provider.
         */
        ~CadProvider();

        /**
         * Sets the Http to the Website of the comic (either a concrete
         * date exluding the currentDate or a generic Url)
         */
        void setWebsiteHttp();

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

    protected:
        virtual void pageRetrieved( int id, const QByteArray &data );
        virtual void pageError( int id, const QString &errorMessage );

    private:
        class Private;
        Private* const d;
};

#endif
