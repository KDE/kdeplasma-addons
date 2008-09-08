/*
 *   Copyright (C) 2008 Hugo Parente Lima <hugo.pl@gmail.com>
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

#ifndef MALVADOSPROVIDER_H
#define MALVADOSPROVIDER_H

#include <KUrl>
#include "comicprovider.h"

/**
 * This class provides the comic strip image for malvados.com.br
 */
class MalvadosProvider : public ComicProvider
{
    Q_OBJECT
    public:
        MalvadosProvider( QObject *parent, const QVariantList &args );
        ~MalvadosProvider();

        /**
         * Returns the type of identifier that is used by this
         * comic provider.
         */
        IdentifierType identifierType() const;

        /**
         * Returns the url of the website where the comic of that particular date resides.
         */
        KUrl websiteUrl() const;

        /**
         * Returns the requested image.
         *
         * Note: This method returns only a valid image after the
         *       finished() signal has been emitted.
         */
        QImage image() const;

        /**
         * Returns the identifier of the comic request.
         */
        QString identifier() const;

        /**
         * Returns the identifier of the next comic (default: date of next day).
         */
        QString nextIdentifier() const;

        /**
         * Returns the identifier of the previous comic (default: date of previous day).
         */
        QString previousIdentifier() const;

    protected:
        void pageRetrieved( int id, const QByteArray &rawData );
        void pageError( int id, const QString &message );

    private:
        class Private;
        Private* const d;
};

#endif
