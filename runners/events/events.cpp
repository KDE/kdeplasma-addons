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

#include "events.h"

#include <KDebug>
#include <KMimeType>
#include <KIconLoader>

#include <Akonadi/CollectionFetchJob>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/ItemModifyJob>
#include <Akonadi/Item>

#include <QEventLoop>

#include <kcal/event.h>
#include <kcal/todo.h>

#include <boost/shared_ptr.hpp>

// This is the command that links the applet to the .desktop file
K_EXPORT_PLASMA_RUNNER(events, EventsRunner)

// Mime types
static const QString eventMimeType( "application/x-vnd.akonadi.calendar.event" );
static const QString todoMimeType( "application/x-vnd.akonadi.calendar.todo" );

// Keywords
static const QString eventKeyword( i18nc( "Event creation keyword", "event" ) );
static const QString todoKeyword( i18nc( "Todo creation keyword", "todo" ) );
static const QString completeKeyword( i18nc( "Todo completion keyword", "complete" ) );
static const QString commentKeyword( i18nc( "Event comment keyword", "comment" ) );

using namespace Akonadi;

using Plasma::QueryMatch;
using Plasma::RunnerSyntax;

static KDateTime variantToDateTime( const QVariant & var ) {
    return var.type() == QVariant::Date ? KDateTime( var.toDate() ) : KDateTime( var.toDateTime() );
}

static QVariant dateTimeToVariant( const KDateTime & dt ) {
    return dt.isDateOnly() ? QVariant( dt.date() ) : QVariant( dt.dateTime() );
}

static QString dateTimeToString( const KDateTime & dt ) {
    return KGlobal::locale()->formatDateTime( dt );
}

EventsRunner::EventsRunner(QObject *parent, const QVariantList& args)
    : Plasma::AbstractRunner(parent, args), cachedItemsLoaded( false )
{
    Q_UNUSED(args);

    setObjectName("events_runner");

    icon = KIcon( KIconLoader().loadMimeTypeIcon( KMimeType::mimeType( "text/calendar" )->iconName(), KIconLoader::NoGroup ) );

    CollectionFetchJob *job = new CollectionFetchJob( Collection::root(), CollectionFetchJob::Recursive, this );

    connect( job, SIGNAL( collectionsReceived(Akonadi::Collection::List) ), this, SLOT( collectionsReceived(Akonadi::Collection::List) ) );

    describeSyntaxes();
}

EventsRunner::~EventsRunner() {
}

void EventsRunner::reloadConfiguration() {
}

void EventsRunner::collectionsReceived( const Collection::List & list ) {
    foreach ( const Collection & coll, list ) {
        if ( !eventsCollection.isValid() && coll.contentMimeTypes().contains( eventMimeType ) ) {
            eventsCollection = coll;
        }

        if ( !todoCollection.isValid() && coll.contentMimeTypes().contains( todoMimeType ) ) {
            todoCollection = coll;
        }
    }
}

Akonadi::Item::List EventsRunner::selectItems( const QString & query, const QStringList & mimeTypes ) {
    Item::List matchedItems;

    if ( query.length() < 3 )
        return matchedItems;

    QMutexLocker locker( &cachedItemsMutex ); // Lock cachedItems access

    if ( !cachedItemsLoaded ) {
        ItemFetchScope scope;
        scope.fetchFullPayload( true );

        ItemFetchJob job( todoCollection );
        job.setFetchScope( scope );

        QEventLoop loop;

        connect( &job, SIGNAL(finished( KJob * )), &loop, SLOT(quit()) );

        job.start();
        loop.exec();

        cachedItems = job.items();
    }

    foreach ( const Item & item, cachedItems ) {
        if ( !mimeTypes.contains( item.mimeType() ) )
            continue;

        if ( !item.hasPayload<KCal::Incidence::Ptr>() )
            continue;

        KCal::Incidence::Ptr incidence = item.payload<KCal::Incidence::Ptr>();

        if ( !incidence )
            continue;

        if ( incidence->summary().contains( query, Qt::CaseInsensitive ) )
            matchedItems.append( item );

        if ( matchedItems.size() >= 10 ) // Stop search when too many are found
            break;
    }

    return matchedItems;
}

void EventsRunner::describeSyntaxes() {
    QList<RunnerSyntax> syntaxes;

    RunnerSyntax eventSyntax( QString("%1 :q:; summary; date [; categories]").arg( eventKeyword ), i18n("Creates event in calendar by its description in :q:, which consists of parts divided by semicolons. The first two parts (both obligatory) are the event summary and its start date. The third, optional, is list of event categories, divided by commas.") );
    eventSyntax.setSearchTermDescription( i18n( "event description" ) );
    syntaxes.append(eventSyntax);

    RunnerSyntax todoSyntax( QString("%1 :q:; summary; date [; categories]").arg( todoKeyword ), i18n("Creates todo in calendar by its description in :q:, which consists of parts divided by semicolons. The first two parts (both obligatory) are a summary of the todo, and its due date. The third, optional, is list of todo categories, divided by commas.") );
    todoSyntax.setSearchTermDescription( i18n( "todo description" ) );
    syntaxes.append(todoSyntax);

    RunnerSyntax completeSyntax( QString("%1 :q: [; <percent>]").arg( completeKeyword ), i18n("Selects todo from calendar by its summary in :q: and marks it as completed.") );
    completeSyntax.setSearchTermDescription( i18n( "complete todo description" ) );
    syntaxes.append(completeSyntax);

    RunnerSyntax commentSyntax( QString("%1 :q: <comment>").arg( commentKeyword ), i18n("Selects event from calendar by its summary in :q: and append <comment> to its body.") );
    commentSyntax.setSearchTermDescription( i18n( "comment todo description" ) );
    syntaxes.append(commentSyntax);

    setSyntaxes(syntaxes);
}

QStringList EventsRunner::splitArguments( const QString & str ) {
    QStringList args = str.split(';');

    for ( QStringList::Iterator it = args.begin(); it != args.end(); ++it ) {
        *it = (*it).trimmed(); // Trim all arguments
    }

    return args;
}

QueryMatch EventsRunner::createQueryMatch( const QString & definition, MatchType type ) {
    QStringList args = splitArguments( definition );

    if ( args.size() < 2 || args[0].length() < 3 || args[1].length() < 3 )
        return QueryMatch( 0 ); // Return invalid match if not enough arguments

    DateTimeRange range = dateTimeParser.parseRange( args[1].trimmed() );

    if ( !range.start.isValid() || !range.finish.isValid() )
        return QueryMatch( 0 ); // Return invalid match if date is invalid

    QMap<QString,QVariant> data; // Map for data

    data["type"] = type;
    data["summary"] = args[0];
    data["start"] = dateTimeToVariant( range.start );
    data["finish"] = dateTimeToVariant( range.finish );

    if ( args.length() > 2 && !args[2].isEmpty() ) // If categories info present
        data["categories"] = args[2];

    QueryMatch match( this );

    if ( type == CreateEvent ) {
        if ( range.isPoint() )
            match.setText( i18n( "Create event \"%1\" at %2", data["summary"].toString(), dateTimeToString( range.start ) ) );
        else
            match.setText( i18n( "Create event \"%1\" from %2 to %3", data["summary"].toString(), dateTimeToString( range.start ), dateTimeToString( range.finish ) ) );

        match.setId( eventKeyword + '|' + definition );
    } else if ( type == CreateTodo ) {
        if ( range.isPoint() )
            match.setText( i18n( "Create todo \"%1\" due to %2", data["summary"].toString(), dateTimeToString( range.finish ) ) );
        else
            match.setText( i18n( "Create todo \"%1\" due to %3 starting at %2", data["summary"].toString(), dateTimeToString( range.start ), dateTimeToString( range.finish ) ) );

        match.setId( todoKeyword + '|' + definition );
    } else {
        qDebug() << "Unknown match type: " << type;

        return QueryMatch( 0 );
    }

    QString subtext = "";

    if ( data.contains("categories") ) {
        subtext += i18n( "Categories: %1", data["categories"].toString() );
    }

    if ( !subtext.isEmpty() )
        match.setSubtext( subtext );

    match.setData( data );
    match.setRelevance( 0.8 );
    match.setIcon( icon );

    return match;
}

Plasma::QueryMatch EventsRunner::createUpdateMatch( const Item & item, MatchType type, const QStringList & args ) {
    QueryMatch match( this );

    QMap<QString,QVariant> data; // Map for data

    data["type"] = type;

    if ( type == CompleteTodo ) {
        KCal::Todo::Ptr todo = item.payload<KCal::Todo::Ptr>();

        match.setText( i18n( "Complete todo \"%1\"", todo->summary() ) );
        match.setSubtext( i18n( "Date: %1", dateTimeToString( todo->dtDue() ) ) );

        data["item"] = qVariantFromValue( item );
        data["percent"] = args.size() > 1 ? args[1].toInt() : 100; // Set percent complete to specified or 100 by default
    } else if ( type == CommentIncidence ) {
        if ( args.size() < 2 ) // There is no comment - skip match
            return QueryMatch( 0 );

        KCal::Incidence::Ptr incidence = item.payload<KCal::Incidence::Ptr>();

        match.setText( i18n( "Comment incidence \"%1\"", incidence->summary() ) );

        if ( KCal::Todo * todo = dynamic_cast<KCal::Todo *>( incidence.get() ) ) {
            match.setSubtext( i18n( "Date: %1", dateTimeToString( todo->dtDue() ) ) );
        } else if ( KCal::Event * event = dynamic_cast<KCal::Event *>( incidence.get() ) ) {
            match.setSubtext( i18n( "Date: %1", dateTimeToString( event->dtStart() ) ) );
        }

        data["item"] = qVariantFromValue( item );
        data["comment"] = args[1];
    } else {
        qDebug() << "Unknown match type: " << type;

        return QueryMatch( 0 );
    }

    match.setData( data );
    match.setRelevance( 0.8 );
    match.setIcon( icon );
    match.setId( QString("update-%1-%2").arg( item.id() ).arg( type )  );

    return match;
}

void EventsRunner::match( Plasma::RunnerContext &context ) {
    const QString term = context.query();

    if ( term.length() < 8 )
        return;

    if ( term.startsWith( eventKeyword ) ) {
        QueryMatch match = createQueryMatch( term.mid( eventKeyword.length() ), CreateEvent );

        if ( match.isValid() )
            context.addMatch( term, match );
    } else if ( term.startsWith( todoKeyword ) ) {
        QueryMatch match = createQueryMatch( term.mid( eventKeyword.length() ), CreateTodo );

        if ( match.isValid() )
            context.addMatch( term, match );
    } else if ( term.startsWith( completeKeyword ) ) {
        QStringList args = splitArguments( term.mid( completeKeyword.length() ) );
        Item::List items = selectItems( args[0], QStringList( todoMimeType ) );

        foreach ( const Item & item, items ) {
            QueryMatch match = createUpdateMatch( item, CompleteTodo, args );

            if ( match.isValid() )
                context.addMatch( term, match );
        }
    } else if ( term.startsWith( commentKeyword ) ) {
        QStringList args = splitArguments( term.mid( commentKeyword.length() ) );
        Item::List items = selectItems( args[0], QStringList( todoMimeType ) << eventMimeType );

        foreach ( const Item & item, items ) {
            QueryMatch match = createUpdateMatch( item, CommentIncidence, args );

            if ( match.isValid() )
                context.addMatch( term, match );
        }
    }
}

void EventsRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match) {
    Q_UNUSED(context)

    QMap<QString,QVariant> data = match.data().toMap();

    if ( data["type"].toInt() == CreateEvent ) {
        if ( !eventsCollection.isValid() ) {
            qDebug() << "No valid collection for events available";
            return;
        }

        KCal::Event::Ptr event( new KCal::Event() );
        event->setSummary( data["summary"].toString() );

        event->setDtStart( variantToDateTime( data["start"] ) );

        if ( data["start"] != data["finish"] ) { // Set end date if it differs from start date
            event->setDtEnd( variantToDateTime( data["finish"] ) );
        }

        if ( data.contains("categories") ) // Set categories if present
            event->setCategories( data["categories"].toString() );

        Item item( eventMimeType );
        item.setPayload<KCal::Event::Ptr>( event );

        new Akonadi::ItemCreateJob( item, eventsCollection, this );
    } else if ( data["type"].toInt() == CreateTodo ) {
        if ( !todoCollection.isValid() ) {
            qDebug() << "No valid collection for todos available";
            return;
        }

        KCal::Todo::Ptr todo( new KCal::Todo() );
        todo->setSummary( data["summary"].toString() );
        todo->setPercentComplete( 0 );

        todo->setDtDue( variantToDateTime( data["finish"] ) );
        todo->setHasDueDate( true );

        if ( data["start"] != data["finish"] ) { // Set start date if it differs from due date
            todo->setDtStart( variantToDateTime( data["start"] ) );
            todo->setHasStartDate( true );
        } else {
            todo->setHasStartDate( false );
        }

        if ( data.contains("categories") ) // Set categories if present
            todo->setCategories( data["categories"].toString() );

        Item item( todoMimeType );
        item.setPayload<KCal::Todo::Ptr>( todo );

        new Akonadi::ItemCreateJob( item, todoCollection, this );
    } else if ( data["type"].toInt() == CompleteTodo ) {
        Item item = data["item"].value<Item>(); // Retrieve item
        KCal::Todo::Ptr todo = item.payload<KCal::Todo::Ptr>(); // Retrieve item payload - todo

        todo->setPercentComplete( data["percent"].toInt() ); // Set item percent completed

        ItemModifyJob * job = new ItemModifyJob( item, this );

        job->setIgnorePayload( false ); // Update payload!!
    } else if ( data["type"].toInt() == CommentIncidence ) {
        Item item = data["item"].value<Item>(); // Retrieve item
        KCal::Incidence::Ptr incidence = item.payload<KCal::Incidence::Ptr>(); // Retrieve item payload - incidence

        if ( incidence->descriptionIsRich() ) {
            incidence->setDescription( incidence->richDescription() + "\n\n" + data["comment"].toString(), true);
        } else {
            incidence->setDescription( incidence->description() + "\n\n" + data["comment"].toString());
        }

        ItemModifyJob * job = new ItemModifyJob( item, this );

        job->setIgnorePayload( false ); // Update payload!!
    } else {
        qDebug() << "Unknown match type: " << data["type"];
    }
}

