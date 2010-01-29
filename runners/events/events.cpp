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

    KDateTime date = dateTimeParser.parse( args[1].trimmed() );

    if ( !date.isValid() )
        return QueryMatch( 0 ); // Return invalid match if date is invalid

    QMap<QString,QVariant> data; // Map for data

    data["type"] = type;
    data["summary"] = args[0];
    data["date"] = dateTimeToVariant( date );

    if ( args.length() > 2 && !args[2].isEmpty() ) // If categories info present
        data["categories"] = args[2];

    QueryMatch match( this );

    if ( type == Event ) {
        match.setText( i18n( "Create event \"%1\" at %2", data["summary"].toString(), date.toString( date.isDateOnly() ? "%d.%m.%Y" : "%H:%M %d.%m.%Y" ) ) );
        match.setId( eventKeyword + '|' + definition );
    } else if ( type == Todo ) {
        match.setText( i18n( "Create todo \"%1\" due to %2", data["summary"].toString(), date.toString( date.isDateOnly() ? "%d.%m.%Y" : "%H:%M %d.%m.%Y" ) ) );
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
        event->setDtStart( variantToDateTime( data["date"] ) );

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
        todo->setDtDue( variantToDateTime( data["date"] ) );
        todo->setPercentComplete( 0 );
        todo->setHasStartDate( false );
        todo->setHasDueDate( true );

        if ( data.contains("categories") ) // Set categories if present
            todo->setCategories( data["categories"].toString() );
            
        Item item( todoMimeType );
        item.setPayload<KCal::Todo::Ptr>( todo );
            
        new Akonadi::ItemCreateJob( item, todoCollection, this );
    }
}

