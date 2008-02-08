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

#ifndef COMICPROVIDER_H
#define COMICPROVIDER_H

#include <QtCore/QObject>
#include <QtCore/QDate>

#include <kpluginfactory.h>

#include "plasma_comic_export.h"

class QImage;
class KUrl;

#define COMICPROVIDER_EXPORT_PLUGIN( classname, componentName, catalogName ) \
    K_PLUGIN_FACTORY( classname ## Factory, registerPlugin< classname >(); ) \
    K_EXPORT_PLUGIN( classname ## Factory( componentName, catalogName ) )

/**
 * This class is an interface for comic providers.
 */
class PLASMA_COMIC_EXPORT ComicProvider : public QObject
{
    Q_OBJECT

    public:
        enum IdentifierType {
            DateIdentifier = 0,
            NumberIdentifier,
            StringIdentifier
        };

        /**
         * Creates a new comic provider.
         *
         * @param parent The parent object.
         * @param args Arguments passed by the plugin loader.
         */
        ComicProvider( QObject *parent, const QVariantList &args );

        /**
         * Destroys the comic provider.
         */
        virtual ~ComicProvider();

        /**
         * Returns the type of identifier that is used by this
         * comic provider.
         */
        virtual IdentifierType identifierType() const = 0;

        /**
         * Returns the Url of the website where thee comic of that particular date resides
         */
        virtual KUrl websiteUrl() const = 0;

        /**
         * Returns the requested image.
         *
         * Note: This method returns only a valid image after the
         *       finished() signal has been emitted.
         */
        virtual QImage image() const = 0;

        /**
         * Returns the identifier of the comic request (usually name + date).
         */
        virtual QString identifier() const = 0;

        /**
         * Returns the identifier of the next comic.
         */
        virtual QString nextIdentifier() const;

        /**
         * Returns the identifier of the previous comic.
         */
        virtual QString previousIdentifier() const;

        /**
         * Set whether this request is for the current comic.
         */
        void setIsCurrent( bool value );

        /**
         * Returns whether this request is for the current comic.
         */
        bool isCurrent() const;

    Q_SIGNALS:
        /**
         * This signal is emitted whenever a request has been finished
         * successfully.
         *
         * @param provider The provider which emitted the signal.
         */
        void finished( ComicProvider *provider );

        /**
         * This signal is emitted whenever an error has occured.
         *
         * @param provider The provider which emitted the signal.
         */
        void error( ComicProvider *provider );

    protected:
        QDate requestedDate() const;
        int requestedNumber() const;
        QString requestedString() const;

    private:
        class Private;
        Private* const d;
};

#endif
