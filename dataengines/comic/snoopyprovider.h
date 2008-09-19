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

#ifndef SNOOPYPROVIDER_H
#define SNOOPYPROVIDER_H

#include "comicprovider.h"

/**
* This class provides the comic strip image for snoopy.com.
*/
class SnoopyProvider : public ComicProvider
{
    Q_OBJECT

    public:
        /**
        * Creates a new snoopy provider.
        *
        * @param parent The parent object.
        */
        SnoopyProvider( QObject *parent, const QVariantList &args );

        /**
        * Destroys the snoopy provider.
        */
        ~SnoopyProvider();

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

    protected:
        virtual void pageRetrieved( int id, const QByteArray &data );
        virtual void pageError( int id, const QString &errorMessage );

    private:
        class Private;
        Private* const d;
};

#endif
