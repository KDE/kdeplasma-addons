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

class QImage;
class KUrl;

/**
 * This class is an interface for comic providers.
 */
class ComicProvider : public QObject
{
    Q_OBJECT

    public:
        /**
         * What is the meaning of the suffix of the identifier comicname:suffix
         * Usually it's a date, it can also be a simple integer or a string
         */
        enum SuffixType {
            DateSuffix = 0,
            IntSuffix,
            StringSuffix
        };

        /**
         * Creates a new comic provider.
         *
         * @param parent The parent object.
         */
        ComicProvider( QObject *parent = 0 );

        /**
         * Destroys the comic provider.
         */
        virtual ~ComicProvider();

        static SuffixType suffixType(const QString &name);

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
         * Returns the identifier suffix of the next comic
         */
        virtual QString nextIdentifierSuffix() const = 0;

        /**
         * Returns the identifier suffix of the previous comic
         */
        virtual QString previousIdentifierSuffix() const = 0;

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
    private:
        QDate m_date;
};

#endif
