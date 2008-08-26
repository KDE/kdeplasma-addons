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

#include <QtCore/QDate>
#include <QtCore/QObject>

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
        /**
         * Describes the type of how this comic provider
         * references the previous or next comic strip.
         */
        enum IdentifierType {
            DateIdentifier = 0,   ///< References by date
            NumberIdentifier,     ///< References by numerical identifier
            StringIdentifier      ///< References by arbitrary string
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
         * Returns the url of the website where the comic of that particular date resides.
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
         * Returns the identifier of the comic request.
         */
        virtual QString identifier() const = 0;

        /**
         * Returns the identifier of the next comic (default: date of next day).
         */
        virtual QString nextIdentifier() const;

        /**
         * Returns the identifier of the previous comic (default: date of previous day).
         */
        virtual QString previousIdentifier() const;

        /**
         * Set whether this request is for the current comic (only used internally).
         */
        void setIsCurrent( bool value );

        /**
         * Returns whether this request is for the current comic (only used internally).
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

        typedef QMap<QString, QString> MetaInfos;

        /**
         * This method should be used by all comic providers to request
         * websites or images from the web. It encapsulates the HTTP
         * handling and calls pageRetrieved() or pageError() on success or error.
         *
         * @param url The url to access.
         * @param id A unique id that identifies this request.
         * @param infos A list of meta informations passed to http.
         */
        void requestPage( const KUrl &url, int id, const MetaInfos &infos = MetaInfos() );

        /**
         * This method is called whenever a request done by requestPage() was successfull.
         *
         * @param id The unique identifier of that request.
         * @param data The data of the fetched object.
         */
        virtual void pageRetrieved( int id, const QByteArray &data );

        /**
         * This method is called whenever a request done by requestPage() has failed.
         *
         * @param id The unique identifier of that request.
         * @param message The error message.
         */
        virtual void pageError( int id, const QString &message );

    private:
        class Private;
        Private* const d;

        Q_PRIVATE_SLOT( d, void jobDone( KJob* ) );
};

#endif
