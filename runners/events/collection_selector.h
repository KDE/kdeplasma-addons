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

#ifndef COLLECTION_SELECTOR_H
#define COLLECTION_SELECTOR_H

//Project-Includes

//KDE-Includes
#include <Akonadi/Collection>

//Qt

// Mime types
static const QString eventMimeType( "application/x-vnd.akonadi.calendar.event" );
static const QString todoMimeType( "application/x-vnd.akonadi.calendar.todo" );

class CollectionSelector : public QObject
{
    Q_OBJECT

public:
    explicit CollectionSelector( QObject* parent );

    void receiveCollections();

    Akonadi::Collection selectTodoCollection( Akonadi::Entity::Id id ) { return selectCollectionById( todoCollections, id ); }
    Akonadi::Collection selectEventCollection( Akonadi::Entity::Id id ) { return selectCollectionById( eventCollections, id ); }

signals:
    void collectionsReceived( CollectionSelector & selector );

public:
    Akonadi::Collection::List todoCollections;
    Akonadi::Collection::List eventCollections;

private:
    static Akonadi::Collection selectCollectionById( const Akonadi::Collection::List & collections, Akonadi::Entity::Id id );

private slots:
    void akonadiCollectionsReceived( const Akonadi::Collection::List & collections );
};

#endif
