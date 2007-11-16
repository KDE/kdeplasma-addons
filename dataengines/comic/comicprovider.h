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

class QDate;
class QImage;

/**
 * This class is an interface for comic providers.
 */
class ComicProvider : public QObject
{
    Q_OBJECT

    public:
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

        /**
         * Returns the requested image.
         *
         * Note: This method returns only a valid image after the
         *       finished() signal has been emitted.
         */
        virtual QImage image() const = 0;

        /**
         * Returns the identifier of the comic request (name + date).
         */
        virtual QString identifier() const = 0;

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
};

#endif
