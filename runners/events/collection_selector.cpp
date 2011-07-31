/*
 *   Copyright (C) 2010 Alexey Noskov <alexey.noskov@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2 of
 *   the License or (at your option) version 3 or any later version
 *   accepted by the membership of KDE e.V. (or its successor approved
 *   by the membership of KDE e.V.), which shall act as a proxy
 *   defined in Section 14 of version 3 of the license.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

//Project-Includes
#include "collection_selector.h"

//KDE-Includes
#include <Akonadi/CollectionFetchJob>

//Qt-Includes
#include <QStringList>

using Akonadi::Collection;
using Akonadi::CollectionFetchJob;

CollectionSelector::CollectionSelector( QObject* parent ): QObject( parent ) {
}

void CollectionSelector::receiveCollections() {
    CollectionFetchJob *job = new CollectionFetchJob( Collection::root(), CollectionFetchJob::Recursive, this );

    connect( job, SIGNAL(collectionsReceived(Akonadi::Collection::List)), this, SLOT(akonadiCollectionsReceived(Akonadi::Collection::List)) );
}

void CollectionSelector::akonadiCollectionsReceived( const Collection::List& collections ) {
    foreach ( const Collection & collection, collections ) {
        if ( collection.contentMimeTypes().contains( eventMimeType ) )
            eventCollections.append( collection );

        if ( collection.contentMimeTypes().contains( todoMimeType ) )
            todoCollections.append( collection );
    }

    emit collectionsReceived( *this );
}

Collection CollectionSelector::selectCollectionById( const Collection::List& collections, Akonadi::Entity::Id id ) {
    foreach ( const Collection & collection, collections )
        if ( collection.id() == id )
            return collection;

    if ( !collections.isEmpty() )
        return collections.first();

    return Collection();
}
