/*
 *   Copyright (C) 2008 Petri Damstén <damu@iki.fi>
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

#include "comicproviderwrapper.h"
#include "comicproviderkross.h"
#include "comicprovider.h"

#include <QTimer>
#include <QBuffer>
#include <QPainter>
#include <KUrl>
#include <KDebug>
#include <KStandardDirs>
#include <Plasma/Package>
#include <kross/core/action.h>
#include <kross/core/interpreter.h>
#include <kross/core/manager.h>

QStringList ComicProviderWrapper::mExtensions;

ImageWrapper::ImageWrapper( QObject *parent, const QImage &image )
: QObject( parent )
, mImage( image )
{
}

QImage ImageWrapper::image() const
{
    return mImage;
}

void ImageWrapper::setImage( const QImage &image )
{
    mImage = image;
}

QByteArray ImageWrapper::rawData() const
{
    QByteArray data;
    QBuffer buffer( &data );
    mImage.save( &buffer );
    return data;
}

void ImageWrapper::setRawData( const QByteArray &rawData )
{
    mImage = QImage::fromData( rawData );
}

DateWrapper::DateWrapper( QObject *parent, const QDate &date )
: QObject( parent )
, mDate( date )
{
}

QDate DateWrapper::date() const
{
    return mDate;
}

void DateWrapper::setDate( const QDate &date )
{
    mDate = date;
}

QDate DateWrapper::fromVariant( const QVariant &variant )
{
    if ( variant.type() == QVariant::Date || variant.type() == QVariant::DateTime ) {
        return variant.toDate();
    } else if ( variant.type() == QVariant::String ) {
        return QDate::fromString( variant.toString(), Qt::ISODate );
    } else {
        DateWrapper* dw = qobject_cast<DateWrapper*>( variant.value<QObject*>() );
        if (dw) {
            return dw->date();
        }
    }
    return QDate();
}

QObject* DateWrapper::addDays( int ndays )
{
    return new DateWrapper( this, mDate.addDays( ndays ) );
}

QObject* DateWrapper::addMonths( int nmonths )
{
    return new DateWrapper( this, mDate.addMonths( nmonths ) );
}

QObject* DateWrapper::addYears( int nyears )
{
    return new DateWrapper( this, mDate.addYears( nyears ) );
}

int DateWrapper::day() const
{
    return mDate.day();
}

int DateWrapper::dayOfWeek() const
{
    return mDate.dayOfWeek();
}

int DateWrapper::dayOfYear() const
{
    return mDate.dayOfYear();
}

int DateWrapper::daysInMonth() const
{
    return mDate.daysInMonth();
}

int DateWrapper::daysInYear() const
{
    return mDate.daysInYear();
}

int DateWrapper::daysTo( const QVariant d ) const
{
    return mDate.daysTo( fromVariant( d ) );
}

bool DateWrapper::isNull() const
{
    return mDate.isNull();
}

bool DateWrapper::isValid() const
{
    return mDate.isValid();
}

int DateWrapper::month() const
{
    return mDate.month();
}

bool DateWrapper::setDate( int year, int month, int day )
{
    return mDate.setDate( year, month, day );
}

int DateWrapper::toJulianDay() const
{
    return mDate.toJulianDay();
}

QString DateWrapper::toString( const QString &format ) const
{
    return mDate.toString( format );
}

QString DateWrapper::toString( int format ) const
{
    return mDate.toString( ( Qt::DateFormat )format );
}

int DateWrapper::weekNumber() const
{
    return mDate.weekNumber();
}

int DateWrapper::year() const
{
    return mDate.year();
}

StaticDateWrapper::StaticDateWrapper( QObject *parent )
: QObject( parent )
{
}

QObject* StaticDateWrapper::currentDate()
{
    return new DateWrapper( this, QDate::currentDate() );
}

QObject* StaticDateWrapper::fromJulianDay( int jd )
{
    return new DateWrapper( this, QDate::fromJulianDay( jd ) );
}

QObject* StaticDateWrapper::fromString( const QString &string, int format )
{
    return new DateWrapper( this, QDate::fromString( string, ( Qt::DateFormat )format ) );
}

QObject* StaticDateWrapper::fromString( const QString &string, const QString &format )
{
    return new DateWrapper( this, QDate::fromString( string, format ) );
}

bool StaticDateWrapper::isLeapYear ( int year )
{
    return QDate::isLeapYear( year );
}

bool StaticDateWrapper::isValid ( int year, int month, int day )
{
    return QDate::isValid( year, month, day );
}

QString StaticDateWrapper::longDayName ( int weekday )
{
    return QDate::longDayName( weekday );
}

QString StaticDateWrapper::longMonthName ( int month )
{
    return QDate::longMonthName( month );
}

QString StaticDateWrapper::shortDayName ( int weekday )
{
    return QDate::shortDayName( weekday );
}

QString StaticDateWrapper::shortMonthName ( int month )
{
    return QDate::shortMonthName( month );
}

ComicProviderWrapper::ComicProviderWrapper( ComicProviderKross *parent )
    : QObject( parent ),
      mAction( 0 ),
      mProvider( parent ),
      mPackage( 0 )
{
    QTimer::singleShot( 0, this, SLOT( init() ) );
}

ComicProviderWrapper::~ComicProviderWrapper()
{
    delete mPackage;
}

void ComicProviderWrapper::init()
{
    const QString path = KStandardDirs::locate( "data", "plasma/comics/" + mProvider->pluginName() + "/" );
    if (!path.isEmpty()) {
        Plasma::PackageStructure::Ptr structure = ComicProviderKross::packageStructure();
        structure->setPath( path );
        mPackage = new Plasma::Package( path, structure );

        if ( mPackage->isValid() ) {
            // package->filePath( "mainscript" ) returns empty if it does not exist
            // We want to test extensions supported by kross with mainscript
            const QString mainscript = mPackage->path() + structure->contentsPrefix() +
                                       structure->path( "mainscript" );

            QFileInfo info( mainscript );
            for ( int i = 0; i < extensions().count() && !info.exists(); ++i ) {
                info.setFile( mainscript + extensions().value( i ) );
            }
            if ( info.exists() ) {
                mAction = new Kross::Action( parent(), mProvider->pluginName() );
                if ( mAction ) {
                    mAction->addObject( this, "comic" );
                    mAction->addObject( new StaticDateWrapper( this ), "date" );
                    mAction->setFile( info.filePath() );
                    mAction->trigger();
                    mFunctions = mAction->functionNames();

                    callFunction( "init" );
                }
            }
        }
    }
}

const QStringList& ComicProviderWrapper::extensions() const
{
    if ( mExtensions.isEmpty() ) {
        Kross::InterpreterInfo* info;
        QStringList list;
        QString wildcards;

        foreach( const QString &interpretername, Kross::Manager::self().interpreters() ) {
            info = Kross::Manager::self().interpreterInfo( interpretername );
            wildcards = info->wildcard();
            wildcards.replace( "*", "" );
            mExtensions << wildcards.split( " " );
        }
    }
    return mExtensions;
}

ComicProvider::IdentifierType ComicProviderWrapper::identifierType() const
{
    ComicProvider::IdentifierType result = ComicProvider::StringIdentifier;
    const QString type = mProvider->description().property( "X-KDE-PlasmaComicProvider-SuffixType" ).toString();
    if ( type == "Date" ) {
        result = ComicProvider::DateIdentifier;
    } else if ( type == "Number" ) {
        result = ComicProvider::NumberIdentifier;
    } else if ( type == "String" ) {
        result = ComicProvider::StringIdentifier;
    }
    return result;
}

QImage ComicProviderWrapper::image()
{
    ImageWrapper* img = qobject_cast<ImageWrapper*>( callFunction( "image" ).value<QObject*>() );
    if ( functionCalled() && img ) {
        return img->image();
    }
    return mKrossImage.image();
}

QString ComicProviderWrapper::identifierToString(const QVariant &identifier) const
{
    if ( identifierType() == ComicProvider::DateIdentifier ) {
        return identifier.toDate().toString(Qt::ISODate);
    }
    return identifier.toString();
}

QVariant ComicProviderWrapper::identifierToScript(const QVariant &identifier)
{
    if ( identifierType() == ComicProvider::DateIdentifier) {
        return QVariant::fromValue( qobject_cast<QObject*>( new DateWrapper( this, identifier.toDate() ) ) );
    }
    return identifier;
}

QVariant ComicProviderWrapper::identifierFromScript( const QVariant &identifier ) const
{
    QVariant result;
    switch ( identifierType() ) {
    case DateIdentifier:
        result = DateWrapper::fromVariant( identifier );
        break;
    case NumberIdentifier:
        result = identifier.toInt();
        break;
    case StringIdentifier:
        result = identifier.toString();
        break;
    }
    return result;
}

QVariant ComicProviderWrapper::identifierWithDefault() const
{
    if ( mIdentifier.isNull() ) {
        switch ( identifierType() ) {
        case DateIdentifier:
            return mProvider->requestedDate();
        case NumberIdentifier:
            return mProvider->requestedNumber();
        case StringIdentifier:
            return mProvider->requestedString();
        }
    }
    return mIdentifier;
}

QString ComicProviderWrapper::comicAuthor() const
{
    return mProvider->comicAuthor();
}

void ComicProviderWrapper::setComicAuthor( const QString &author )
{
    mProvider->setComicAuthor( author );
}

QString ComicProviderWrapper::websiteUrl()
{
    return mWebsiteUrl;
}

void ComicProviderWrapper::setWebsiteUrl( const QString &websiteUrl )
{
    mWebsiteUrl = websiteUrl;
}

QString ComicProviderWrapper::title()
{
    return mTitle;
}

void ComicProviderWrapper::setTitle( const QString &title )
{
    mTitle = title;
}

QString ComicProviderWrapper::additionalText()
{
    return mAdditionalText;
}

void ComicProviderWrapper::setAdditionalText( const QString &additionalText )
{
    mAdditionalText = additionalText;
}

QVariant ComicProviderWrapper::identifier()
{
    return identifierToScript( identifierWithDefault() );
}

void ComicProviderWrapper::setIdentifier( const QVariant &identifier )
{
    mIdentifier = identifierFromScript( identifier );
}

QVariant ComicProviderWrapper::nextIdentifier()
{
    return identifierToScript( mNextIdentifier );
}

void ComicProviderWrapper::setNextIdentifier( const QVariant &nextIdentifier )
{
    mNextIdentifier = identifierFromScript( nextIdentifier );
}

QVariant ComicProviderWrapper::previousIdentifier()
{
    return identifierToScript( mPreviousIdentifier );
}

void ComicProviderWrapper::setPreviousIdentifier( const QVariant &previousIdentifier )
{
    mPreviousIdentifier = identifierFromScript( previousIdentifier );
}

QVariant ComicProviderWrapper::firstIdentifier()
{
    return identifierToScript( mFirstIdentifier );
}

void ComicProviderWrapper::setFirstIdentifier( const QVariant &firstIdentifier )
{
    switch ( identifierType() ) {
    case DateIdentifier:
        mProvider->setFirstStripDate( DateWrapper::fromVariant(firstIdentifier) );
        break;
    case NumberIdentifier:
        mProvider->setFirstStripNumber( firstIdentifier.toInt() );
        break;
    case StringIdentifier:
        break;
    }
    mFirstIdentifier = identifierFromScript( firstIdentifier );
}

QString ComicProviderWrapper::identifierString() const
{
    return mProvider->pluginName() + ':' + identifierToString( identifierWithDefault() );
}

QString ComicProviderWrapper::firstIdentifierString() const
{
    return identifierToString( mFirstIdentifier );
}

QString ComicProviderWrapper::nextIdentifierString() const
{
    return identifierToString( mNextIdentifier );
}

QString ComicProviderWrapper::previousIdentifierString() const
{
    return identifierToString( mPreviousIdentifier );
}

void ComicProviderWrapper::pageRetrieved( int id, const QByteArray &data )
{
    if ( id == Image ) {
        mKrossImage.setRawData( data );
        callFunction( "pageRetrieved", QVariantList() << id <<
                      qVariantFromValue( qobject_cast<QObject*>( &mKrossImage ) ) );
        finished();
    } else {
        callFunction( "pageRetrieved", QVariantList() << id << data );
    }
}

void ComicProviderWrapper::pageError( int id, const QString &message )
{
    callFunction( "pageError", QVariantList() << id << message );
    if ( !functionCalled() ) {
        emit mProvider->error( mProvider );
    }
}

void ComicProviderWrapper::finished() const
{
    kDebug() << QString( "Author" ).leftJustified( 22, '.' ) << comicAuthor();
    kDebug() << QString( "Website URL" ).leftJustified( 22, '.' ) << mWebsiteUrl;
    kDebug() << QString( "Title" ).leftJustified( 22, '.' ) << mTitle;
    kDebug() << QString( "Additional Text" ).leftJustified( 22, '.' ) << mAdditionalText;
    kDebug() << QString( "Identifier" ).leftJustified( 22, '.' ) << identifierString();
    kDebug() << QString( "First Identifier" ).leftJustified( 22, '.' ) << firstIdentifierString();
    kDebug() << QString( "Next Identifier" ).leftJustified( 22, '.' ) << nextIdentifierString();
    kDebug() << QString( "Previous Identifier" ).leftJustified( 22, '.' ) << previousIdentifierString();
    emit mProvider->finished( mProvider );
}

void ComicProviderWrapper::error() const
{
    emit mProvider->error( mProvider );
}

void ComicProviderWrapper::requestPage( const QString &url, int id, const QVariantMap &infos )
{
    QMap<QString, QString> map;

    foreach ( const QString& key, infos.keys() ) {
        map[key] = infos[key].toString();
    }
    mProvider->requestPage( KUrl( url ), id, map );
}

bool ComicProviderWrapper::functionCalled() const
{
    return mFuncFound;
}

QVariant ComicProviderWrapper::callFunction( const QString &name, const QVariantList &args )
{
    if ( mAction ) {
        mFuncFound = mFunctions.contains( name );
        if ( mFuncFound ) {
            return mAction->callFunction( name, args );
        }
    }
    return QVariant();
}

void ComicProviderWrapper::addHeader( const QString &name, PositionType position )
{
    const QString headerRelLoc( mPackage->filePath( "images", name ) );
    const QImage header( headerRelLoc );
    const QImage comic = mKrossImage.image();
    int height = 0;
    int width = 0;

    switch (position) {
    case Top:
    case Bottom:
        height = header.height() + comic.height();
        width = ( header.width() >= comic.width() ) ? header.width() : comic.width();
        break;
    case Left:
    case Right:
        height = ( header.height() >= comic.height() ) ? header.height() : comic.height();
        width = header.width() + comic.width();
        break;
    }

    QImage image = QImage( QSize( width, height ), QImage::Format_RGB32 );
    image.fill( header.pixel( QPoint( 0, 0 ) ) );

    QPainter painter( &image );

    // center and draw the Images
    QPoint headerPos;
    QPoint comicPos;

    switch (position) {
    case Top:
        headerPos = QPoint( ( ( width - header.width() ) / 2 ), 0 );
        comicPos = QPoint( ( ( width - comic.width() ) / 2 ), header.height() );
        break;
    case Bottom:
        headerPos = QPoint( ( ( width - header.width() ) / 2 ), comic.height() );
        comicPos = QPoint( ( ( width - comic.width() ) / 2 ), 0 );
        break;
    case Left:
        headerPos = QPoint( 0, ( ( height - header.height() ) / 2 ) );
        comicPos = QPoint( header.width(), ( ( height - comic.height() ) / 2 ) );
        break;
    case Right:
        headerPos = QPoint( comic.width(), ( ( height - header.height() ) / 2 ) );
        comicPos = QPoint( 0, ( ( height - comic.height() ) / 2 ) );
        break;
    }
    painter.drawImage( headerPos, header );
    painter.drawImage( comicPos, comic );
    mKrossImage.setImage( image );
}

#include "comicproviderwrapper.moc"
