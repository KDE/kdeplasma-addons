/*
 *   Copyright (C) 2007 Tobias Koenig <tokoe@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify  
 *   it under the terms of the GNU General Public License as published by  
 *   the Free Software Foundation; either version 2 of the License, or     
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

#ifndef POTDPROVIDER_H
#define POTDPROVIDER_H

#include <QObject>
#include <QVariantList>

#include "plasma_potd_export.h"

class QImage;
class QDate;

/**
 * This class is an interface for PoTD providers.
 */
class PLASMA_POTD_EXPORT PotdProvider : public QObject
{
    Q_OBJECT

    public:
        /**
         * Creates a new PoTD provider.
         *
         * @param parent The parent object.
         * @param args The arguments.
         */
        explicit PotdProvider(QObject *parent, const QVariantList &args = QVariantList());

        /**
         * Destroys the PoTD provider.
         */
        ~PotdProvider() override;

        /**
         * Returns the requested image.
         *
         * Note: This method returns only a valid image after the
         *       finished() signal has been emitted.
         */
        virtual QImage image() const = 0;

        /**
         * Returns the identifier of the PoTD request (name + date).
         */
        virtual QString identifier() const;

        /**
         * @return the name of this provider (equiv to X-KDE-PlasmaPoTDProvider-Identifier)
         */
        QString name() const;

        /**
         * @return the date to load for this item, if any
         */
        QDate date() const;

        /**
         * @return if the date is fixed, or if it should always be "today"
         */
        bool isFixedDate() const;

    Q_SIGNALS:
        /**
         * This signal is emitted whenever a request has been finished
         * successfully.
         *
         * @param provider The provider which emitted the signal.
         */
        void finished( PotdProvider *provider );

        /**
         * This signal is emitted whenever an error has occurred.
         *
         * @param provider The provider which emitted the signal.
         */
        void error( PotdProvider *provider );

    private:
        const QScopedPointer<class PotdProviderPrivate> d;
};

#endif
