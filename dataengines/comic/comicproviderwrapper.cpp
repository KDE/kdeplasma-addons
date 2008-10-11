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
#include <KDebug>
#include <KStandardDirs>
#include <Plasma/Package>
#include <kross/core/action.h>
#include <kross/core/interpreter.h>
#include <kross/core/manager.h>

QStringList ComicProviderWrapper::m_extensions;

QImage ImageWrapper::image() const
{
    return QImage::fromData( mData );
}

QByteArray ImageWrapper::rawData() const
{
    return mData;
}

void ImageWrapper::setRawData( const QByteArray &rawData )
{
    mData = rawData;
}


ComicProviderWrapper::ComicProviderWrapper( ComicProviderKross *parent )
    : QObject( parent ),
      m_action( 0 ),
      m_provider( parent ),
      m_useDefaultImageHandler( true )
{
    QTimer::singleShot( 0, this, SLOT( init() ) );
}

ComicProviderWrapper::~ComicProviderWrapper()
{
}

void ComicProviderWrapper::init()
{
    const QString path = KStandardDirs::locate( "data", "plasma/comics/" + m_provider->pluginName() + "/" );
    Plasma::PackageStructure::Ptr structure = ComicProviderKross::packageStructure();
    structure->setPath( path );
    Plasma::Package *package = new Plasma::Package( path, structure );

    if ( package->isValid() ) {
        // package->filePath( "mainscript" ) returns empty if it does not exist
        // We want to test extensions supported by kross with mainscript
        const QString mainscript = package->path() + structure->contentsPrefix() +
                                   structure->path( "mainscript" );

        QFileInfo info( mainscript );
        for ( int i = 0; i < extensions().count() && !info.exists(); ++i ) {
            info.setFile( mainscript + extensions().value( i ) );
        }
        if ( info.exists() ) {
            m_action = new Kross::Action( parent(), m_provider->pluginName() );
            if ( m_action ) {
                m_action->addObject( this, "comic" );
                m_action->setFile( info.filePath() );
                m_action->trigger();
                m_functions = m_action->functionNames();

                callFunction( "init" );
            }
        }
    }
}

const QStringList& ComicProviderWrapper::extensions() const
{
    if ( m_extensions.isEmpty() ) {
        Kross::InterpreterInfo* info;
        QStringList list;
        QString wildcards;

        foreach( const QString &interpretername, Kross::Manager::self().interpreters() ) {
            info = Kross::Manager::self().interpreterInfo( interpretername );
            wildcards = info->wildcard();
            wildcards.replace( "*", "" );
            m_extensions << wildcards.split( " " );
        }
    }
    return m_extensions;
}

ComicProvider::IdentifierType ComicProviderWrapper::identifierType()
{
    return ( ComicProvider::IdentifierType )callFunction( "identifierType" ).toInt();
}

KUrl ComicProviderWrapper::websiteUrl()
{
    return KUrl( callFunction( "websiteUrl" ).toString() );
}

QImage ComicProviderWrapper::image()
{
    if ( m_useDefaultImageHandler ) {
        return m_image;
    }
    ImageWrapper* img = qobject_cast<ImageWrapper*>( callFunction( "image" ).value<QObject*>() );
    if ( img ) {
        return img->image();
    }
    return QImage();
}

QString ComicProviderWrapper::identifier()
{
    return callFunction( "identifier" ).toString();
}

QString ComicProviderWrapper::nextIdentifier()
{
    return callFunction( "nextIdentifier" ).toString();
}

QString ComicProviderWrapper::previousIdentifier()
{
    return callFunction( "previousIdentifier" ).toString();
}

QString ComicProviderWrapper::firstStripIdentifier()
{
    return callFunction( "firstStripIdentifier" ).toString();
}

QString ComicProviderWrapper::stripTitle()
{
    return callFunction( "stripTitle" ).toString();
}

QString ComicProviderWrapper::additionalText()
{
    return callFunction( "additionalText" ).toString();
}

void ComicProviderWrapper::pageRetrieved( int id, const QByteArray &data )
{
    if ( m_useDefaultImageHandler && id == imageEnum() ) {
        m_image = QImage::fromData( data );
        emit m_provider->finished( m_provider );
    } else if ( id == imageEnum() ) {
        m_krossImage.setRawData( data );
        callFunction( "pageRetrieved", QVariantList() << id <<
                      qVariantFromValue( qobject_cast<QObject*>( &m_krossImage ) ) );
    } else {
        callFunction( "pageRetrieved", QVariantList() << id << data );
    }
}

void ComicProviderWrapper::pageError( int id, const QString &message )
{
    callFunction( "pageError", QVariantList() << id << message );
    if ( !functionCalled() ) {
        emit m_provider->error( m_provider );
    }
}

QString ComicProviderWrapper::firstStripDate() const
{
    return m_provider->firstStripDate().toString( Qt::ISODate );
}

void ComicProviderWrapper::setFirstStripDate( const QString &date )
{
    m_provider->setFirstStripDate( QDate::fromString( date, Qt::ISODate ) );
}

int ComicProviderWrapper::firstStripNumber() const
{
    return m_provider->firstStripNumber();
}

void ComicProviderWrapper::setFirstStripNumber( int number )
{
    m_provider->setFirstStripNumber( number );
}

QString ComicProviderWrapper::comicAuthor() const
{
    return m_provider->comicAuthor();
}

void ComicProviderWrapper::setComicAuthor( const QString &author )
{
    m_provider->setComicAuthor( author );
}

void ComicProviderWrapper::finished() const
{
    emit m_provider->finished( m_provider );
}

void ComicProviderWrapper::error() const
{
    emit m_provider->error( m_provider );
}

QString ComicProviderWrapper::requestedDate() const
{
    return m_provider->requestedDate().toString( Qt::ISODate );
}

int ComicProviderWrapper::requestedNumber() const
{
    return m_provider->requestedNumber();
}

QString ComicProviderWrapper::requestedString() const
{
    return m_provider->requestedString();
}

void ComicProviderWrapper::requestPage( const QString &url, int id, const QVariantMap &infos )
{
    QMap<QString, QString> map;

    foreach ( const QString& key, infos.keys() ) {
        map[key] = infos[key].toString();
    }
    m_provider->requestPage( KUrl( url ), id, map );
}

bool ComicProviderWrapper::functionCalled() const
{
    return m_funcFound;
}

QVariant ComicProviderWrapper::callFunction( const QString &name, const QVariantList &args )
{
    if ( m_action ) {
        m_funcFound = m_functions.contains( name );
        if ( m_funcFound ) {
            return m_action->callFunction( name, args );
        }
    }
    return QVariant();
}

void ComicProviderWrapper::setUseDefaultImageHandler( bool useDefaultImageHandler )
{
    m_useDefaultImageHandler = useDefaultImageHandler;
}

bool ComicProviderWrapper::useDefaultImageHandler() const
{
    return m_useDefaultImageHandler;
}

#include "comicproviderwrapper.moc"
