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
#include <Akonadi/ItemCreateJob>
#include <Akonadi/Item>

#include <kcal/event.h>
#include <kcal/todo.h>

#include <boost/shared_ptr.hpp>

// This is the command that links your applet to the .desktop file
K_EXPORT_PLASMA_RUNNER(events_runner, EventsRunner)

// Mime types
QString eventMimeType( "text/calendar" );
QString todoMimeType( "text/calendar" );

QString eventKeyword( "event" );
QString todoKeyword( "todo" );

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
    return dt.toString( dt.isDateOnly() ? "%d.%m.%Y" : "%H:%M %d.%m.%Y" );
}

EventsRunner::EventsRunner(QObject *parent, const QVariantList& args)
    : Plasma::AbstractRunner(parent, args)
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

void EventsRunner::describeSyntaxes() {
    QList<RunnerSyntax> syntaxes;

    RunnerSyntax eventSyntax( QString("%1 :q:").arg( eventKeyword ), i18n("Creates event in calendar by its description in :q:, which consists of parts divided by semicolon. Two first obligatory parts are event summary and its start date, third, optional, is list of event categories, divided by comma.") );
    eventSyntax.setSearchTermDescription( i18n( "event description" ) );
    syntaxes.append(eventSyntax);

    RunnerSyntax todoSyntax( QString("%1 :q:").arg( todoKeyword ), i18n("Creates todo in calendar by its description in :q:, which consists of parts divided by semicolon. Two first obligatory parts are todo summary and its due date, third, optional, is list of todo categories, divided by comma.") );
    todoSyntax.setSearchTermDescription( i18n( "todo description" ) );
    syntaxes.append(todoSyntax);

    setSyntaxes(syntaxes);
}

QueryMatch EventsRunner::createQueryMatch( const QString & definition, EventsRunner::IncidentType type ) {
    QStringList args = definition.split( ";" );

    if ( args.size() < 2 || args[0].length() < 3 || args[1].length() < 3 )
        return QueryMatch( 0 ); // Return invalid match if not enough arguments

    for ( QStringList::Iterator it = args.begin(); it != args.end(); ++it ) {
        *it = (*it).trimmed(); // Trim all arguments
    }

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

    if ( type == Event ) {
        if ( range.isPoint() )
            match.setText( i18n( "Create event \"%1\" at %2", data["summary"].toString(), dateTimeToString( range.start ) ) );
        else
            match.setText( i18n( "Create event \"%1\" from %2 to %3", data["summary"].toString(), dateTimeToString( range.start ), dateTimeToString( range.finish ) ) );

        match.setId( eventKeyword + '|' + definition );
    } else if ( type == Todo ) {
        if ( range.isPoint() )
            match.setText( i18n( "Create todo \"%1\" due to %2", data["summary"].toString(), dateTimeToString( range.finish ) ) );
        else
            match.setText( i18n( "Create todo \"%1\" due to %3 starting at %2", data["summary"].toString(), dateTimeToString( range.start ), dateTimeToString( range.finish ) ) );

        match.setId( todoKeyword + '|' + definition );
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

void EventsRunner::match( Plasma::RunnerContext &context ) {
    const QString term = context.query();
    
    if ( term.length() < 8 )
        return;    
    
    if ( term.startsWith( eventKeyword ) ) {
        QueryMatch match = createQueryMatch( term.mid( eventKeyword.length() ), Event );
        
        if ( match.isValid() )
            context.addMatch( term, match );
     } else if ( term.startsWith( todoKeyword ) ) {
        QueryMatch match = createQueryMatch( term.mid( eventKeyword.length() ), Todo );
        
        if ( match.isValid() )
            context.addMatch( term, match );
     }
}

void EventsRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match) {    
    Q_UNUSED(context)
    
    QMap<QString,QVariant> data = match.data().toMap();
    
    if ( data["type"].toInt() == Event ) {
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
    } else if ( data["type"].toInt() == Todo ) {
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
    }
}

