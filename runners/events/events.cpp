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
#include "events_config.h"
#include "collection_selector.h"

#include <KDebug>
#include <KMimeType>

#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/ItemModifyJob>
#include <Akonadi/Item>

#include <QEventLoop>

#include <kcalcore/event.h>
#include <kcalcore/todo.h>

#include <boost/shared_ptr.hpp>

#include <iostream>

// This is the command that links the applet to the .desktop file
K_EXPORT_PLASMA_RUNNER(events, EventsRunner)

// Keywords
static const QString eventKeyword( i18nc( "Event creation keyword", "event" ) );
static const QString todoKeyword( i18nc( "Todo creation keyword", "todo" ) );
static const QString completeKeyword( i18nc( "Todo completion keyword", "complete" ) );
static const QString commentKeyword( i18nc( "Event comment keyword", "comment" ) );
static const QString eventsKeyword( i18nc( "Event list keyword", "events" ) );
static const QString todosKeyword( i18nc( "Todo list keyword", "todos" ) );

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

    setObjectName(RUNNER_NAME);
    setSpeed(SlowSpeed);

    icon = KIcon( QLatin1String( "text-calendar" ) );

    describeSyntaxes();
    reloadConfiguration();
}

EventsRunner::~EventsRunner() {
}

void EventsRunner::reloadConfiguration() {
    CollectionSelector * selector = new CollectionSelector( this );
    connect( selector, SIGNAL(collectionsReceived(CollectionSelector&)), this, SLOT(collectionsReceived(CollectionSelector&)) );
    selector->receiveCollections();
}

void EventsRunner::collectionsReceived( CollectionSelector & selector ) {
    KConfigGroup cfg = config();

    todoCollection = selector.selectTodoCollection( cfg.readEntry( CONFIG_TODO_COLLECTION, (Collection::Id)0 ) );
    eventCollection = selector.selectEventCollection( cfg.readEntry( CONFIG_TODO_COLLECTION, (Collection::Id)0 ) );

    selector.deleteLater(); // No need to store it in memory anymore
}

Akonadi::Item::List EventsRunner::listAllItems() {
    QMutexLocker locker( &cachedItemsMutex ); // Lock cachedItems access

    if ( !cachedItemsLoaded ) {
        ItemFetchScope scope;
        scope.fetchFullPayload( true );

        ItemFetchJob job( todoCollection );
        job.setFetchScope( scope );

        QEventLoop loop;

        connect( &job, SIGNAL(finished(KJob*)), &loop, SLOT(quit()) );

        job.start();
        loop.exec();

        cachedItemsLoaded = true;
        cachedItems = job.items();
    }

    return cachedItems;
}

Akonadi::Item::List EventsRunner::selectItems( const QString & query, const QStringList & mimeTypes ) {
    Item::List matchedItems;

    if ( query.length() < 3 )
        return matchedItems;

    foreach ( const Item & item, listAllItems() ) {
        if ( !mimeTypes.contains( item.mimeType() ) )
            continue;

        if ( !item.hasPayload<KCalCore::Incidence::Ptr>() )
            continue;

        KCalCore::Incidence::Ptr incidence = item.payload<KCalCore::Incidence::Ptr>();

        if ( !incidence )
            continue;

        if ( incidence->summary().contains( query, Qt::CaseInsensitive ) )
            matchedItems.append( item );

        if ( matchedItems.size() >= 10 ) // Stop search when too many are found
            break;
    }

    return matchedItems;
}

Akonadi::Item::List EventsRunner::selectItems( const DateTimeRange & query, const QStringList & mimeTypes ) {
    Item::List matchedItems;

    foreach ( const Item & item, listAllItems() ) {
        if ( !mimeTypes.contains( item.mimeType() ) )
            continue;

        if ( !item.hasPayload<KCalCore::Incidence::Ptr>() )
            continue;

        KCalCore::Incidence::Ptr incidence = item.payload<KCalCore::Incidence::Ptr>();

        if ( !incidence )
            continue;

        if ( KCalCore::Todo * todo = dynamic_cast<KCalCore::Todo *>( incidence.data() ) ) {
            if ( todo->hasStartDate() && todo->hasDueDate() && !query.intersects( todo->dtStart(), todo->dtDue() ) )
                continue;
            else if ( todo->hasStartDate() && !query.includes( todo->dtStart() ) )
                continue;
            else if ( todo->hasDueDate() && !query.includes( todo->dtDue() ) )
                continue;
            else if ( !todo->hasDueDate() && !todo->hasStartDate() )
                continue;
        } else if ( KCalCore::Event * event = dynamic_cast<KCalCore::Event *>( incidence.data() ) ) {
            if ( event->recurs() ) {
                if ( event->recurrence()->timesInInterval( query.start, query.finish ).empty() )
                    continue;
            } else {
                if ( event->hasEndDate() && !query.intersects( event->dtStart(), event->dtEnd() ) )
                    continue;
                else if ( !query.includes( event->dtStart() ) )
                    continue;
            }
        } else {
            KDateTime end = incidence->duration().end( incidence->dtStart() );
            if ( !query.intersects( incidence->dtStart(), end ) )
                continue;
        }

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
    completeSyntax.setSearchTermDescription( i18nc( "The command syntax description for complete", "complete todo description" ) );
    syntaxes.append(completeSyntax);

    RunnerSyntax commentSyntax( QString("%1 :q: <comment>").arg( commentKeyword ), i18n("Selects event from calendar by its summary in :q: and append <comment> to its body.") );
    commentSyntax.setSearchTermDescription( i18nc( "The command syntax description for comment", "comment todo description" ) );
    syntaxes.append(commentSyntax);

    RunnerSyntax eventsSyntax( QString("%1 :q:").arg( eventsKeyword ), i18n("Shows events from calendar by its date in :q:.") );
    eventsSyntax.setSearchTermDescription( i18nc( "The command syntax description for event", "event date/time" ) );
    syntaxes.append(eventsSyntax);

    RunnerSyntax todosSyntax( QString("%1 :q:").arg( eventsKeyword ), i18n("Shows todos from calendar by its date in :q:.") );
    todosSyntax.setSearchTermDescription( i18nc( "The command syntax description for todo", "todo date/time" ) );
    syntaxes.append(todosSyntax);

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
    const QStringList args = splitArguments( definition );

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
            match.setText( i18nc( "%1 is the summary of event, %2 is the start time of event", "Create event \"%1\" at %2", data["summary"].toString(), dateTimeToString( range.start ) ) );
        else
            match.setText( i18nc( "%1 is the summary of event, %2 is the start time of event, %3 is the end time of event", "Create event \"%1\" from %2 to %3", data["summary"].toString(), dateTimeToString( range.start ), dateTimeToString( range.finish ) ) );

        match.setId( eventKeyword + '|' + definition );
    } else if ( type == CreateTodo ) {
        if ( range.isPoint() )
            match.setText( i18nc( "%1 is the summary of todo, %2 is the due time of todo", "Create todo \"%1\" due to %2", data["summary"].toString(), dateTimeToString( range.finish ) ) );
        else
            match.setText( i18nc( "%1 is the summary of todo, %2 is the start time of todo, %3 is the due time of todo", "Create todo \"%1\" due to %3 starting at %2", data["summary"].toString(), dateTimeToString( range.start ), dateTimeToString( range.finish ) ) );

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
        KCalCore::Todo::Ptr todo = item.payload<KCalCore::Todo::Ptr>();

        match.setText( i18nc( "The todo is complete", "Complete todo \"%1\"", todo->summary() ) );
        match.setSubtext( i18n( "Date: %1", dateTimeToString( todo->dtDue() ) ) );

        data["item"] = qVariantFromValue( item );
        data["percent"] = args.size() > 1 ? args[1].toInt() : 100; // Set percent complete to specified or 100 by default
    } else if ( type == CommentIncidence ) {
        if ( args.size() < 2 ) // There is no comment - skip match
            return QueryMatch( 0 );

        KCalCore::Incidence::Ptr incidence = item.payload<KCalCore::Incidence::Ptr>();

        match.setText( i18n( "Comment incidence \"%1\"", incidence->summary() ) );

        if ( KCalCore::Todo * todo = dynamic_cast<KCalCore::Todo *>( incidence.data() ) ) {
            match.setSubtext( i18n( "Date: %1", dateTimeToString( todo->dtDue() ) ) );
        } else if ( KCalCore::Event * event = dynamic_cast<KCalCore::Event *>( incidence.data() ) ) {
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

Plasma::QueryMatch EventsRunner::createShowMatch( const Item & item, MatchType type, const DateTimeRange & range ) {
    QueryMatch match( this );

    QMap<QString,QVariant> data; // Map for data

    data["type"] = type;

    if ( type == ShowIncidence ) {
        KCalCore::Incidence::Ptr incidence = item.payload<KCalCore::Incidence::Ptr>();

        match.setText( incidence->summary() );

        if ( KCalCore::Todo * todo = dynamic_cast<KCalCore::Todo *>( incidence.data() ) ) {
            match.setSubtext( i18n( "Date: %1", dateTimeToString( todo->dtDue() ) ) );
        } else if ( KCalCore::Event * event = dynamic_cast<KCalCore::Event *>( incidence.data() ) ) {
            if ( event->recurs() ) {
                QString dates = "";

                foreach ( const KDateTime & dt, event->recurrence()->timesInInterval( range.start, range.finish ) ) {
                    if ( !dates.isEmpty() )
                        dates += ", ";

                    dates += dateTimeToString( dt );
                }

                match.setSubtext( i18n( "Date: %1", dates ) );
            } else {
                match.setSubtext( i18n( "Date: %1", dateTimeToString( event->dtStart() ) ) );
            }
        }

        data["item"] = qVariantFromValue( item );
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

    if ( term.startsWith( eventsKeyword ) ) {
        const QStringList args = splitArguments( term.mid( eventsKeyword.length() ) );
        DateTimeRange range = dateTimeParser.parseRange( args[0].trimmed() );

        if ( range.isValid() ) {
            Item::List items = selectItems( range, QStringList( eventMimeType ) );

            foreach ( const Item & item, items ) {
                QueryMatch match = createShowMatch( item, ShowIncidence, range );

                if ( match.isValid() )
                    context.addMatch( term, match );
            }
        }
    } else if ( term.startsWith( todosKeyword ) ) {
        const QStringList args = splitArguments( term.mid( todosKeyword.length() ) );
        DateTimeRange range = dateTimeParser.parseRange( args[0].trimmed() );

        if ( range.isValid() ) {
            Item::List items = selectItems( range, QStringList( todoMimeType ) );

            foreach ( const Item & item, items ) {
                QueryMatch match = createShowMatch( item, ShowIncidence, range );

                if ( match.isValid() )
                    context.addMatch( term, match );
            }
        }
    } else if ( term.startsWith( eventKeyword ) ) {
        QueryMatch match = createQueryMatch( term.mid( eventKeyword.length() ), CreateEvent );

        if ( match.isValid() )
            context.addMatch( term, match );
    } else if ( term.startsWith( todoKeyword ) ) {
        QueryMatch match = createQueryMatch( term.mid( eventKeyword.length() ), CreateTodo );

        if ( match.isValid() )
            context.addMatch( term, match );
    } else if ( term.startsWith( completeKeyword ) ) {
        const QStringList args = splitArguments( term.mid( completeKeyword.length() ) );
        Item::List items = selectItems( args[0], QStringList( todoMimeType ) );

        foreach ( const Item & item, items ) {
            QueryMatch match = createUpdateMatch( item, CompleteTodo, args );

            if ( match.isValid() )
                context.addMatch( term, match );
        }
    } else if ( term.startsWith( commentKeyword ) ) {
        const QStringList args = splitArguments( term.mid( commentKeyword.length() ) );
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

    const QMap<QString,QVariant> data = match.data().toMap();

    if ( data["type"].toInt() == CreateEvent ) {
        if ( !eventCollection.isValid() ) {
            qDebug() << "No valid collection for events available";
            return;
        }

        KCalCore::Event::Ptr event( new KCalCore::Event() );
        event->setSummary( data["summary"].toString() );

        event->setDtStart( variantToDateTime( data["start"] ) );

        if ( data["start"] != data["finish"] ) { // Set end date if it differs from start date
            event->setDtEnd( variantToDateTime( data["finish"] ) );
        }

        if ( data.contains("categories") ) // Set categories if present
            event->setCategories( data["categories"].toString() );

        Item item( eventMimeType );
        item.setPayload<KCalCore::Event::Ptr>( event );

        new Akonadi::ItemCreateJob( item, eventCollection, this );
    } else if ( data["type"].toInt() == CreateTodo ) {
        if ( !todoCollection.isValid() ) {
            qDebug() << "No valid collection for todos available";
            return;
        }

        KCalCore::Todo::Ptr todo( new KCalCore::Todo() );
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
        item.setPayload<KCalCore::Todo::Ptr>( todo );

        new Akonadi::ItemCreateJob( item, todoCollection, this );
    } else if ( data["type"].toInt() == CompleteTodo ) {
        Item item = data["item"].value<Item>(); // Retrieve item
        KCalCore::Todo::Ptr todo = item.payload<KCalCore::Todo::Ptr>(); // Retrieve item payload - todo

        todo->setPercentComplete( data["percent"].toInt() ); // Set item percent completed

        ItemModifyJob * job = new ItemModifyJob( item, this );

        job->setIgnorePayload( false ); // Update payload!!
    } else if ( data["type"].toInt() == CommentIncidence ) {
        Item item = data["item"].value<Item>(); // Retrieve item
        KCalCore::Incidence::Ptr incidence = item.payload<KCalCore::Incidence::Ptr>(); // Retrieve item payload - incidence

        if ( incidence->descriptionIsRich() ) {
            incidence->setDescription( incidence->richDescription() + "\n\n" + data["comment"].toString(), true);
        } else {
            incidence->setDescription( incidence->description() + "\n\n" + data["comment"].toString());
        }

        ItemModifyJob * job = new ItemModifyJob( item, this );

        job->setIgnorePayload( false ); // Update payload!!
    } else if ( data["type"].toInt() == ShowIncidence ) {
        // Do nothing yet
    } else {
        qDebug() << "Unknown match type: " << data["type"];
    }
}

