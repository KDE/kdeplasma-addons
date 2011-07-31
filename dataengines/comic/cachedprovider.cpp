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

#include "cachedprovider.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QSettings>
#include <QtCore/QTimer>
#include <QtGui/QImage>

#include <KDebug>
#include <kstandarddirs.h>
#include <KUrl>

const int CachedProvider::CACHE_DEFAULT = 20;

static QString identifierToPath( const QString &identifier )
{
    const QString dataDir = KStandardDirs::locateLocal( "data", QLatin1String( "plasma_engine_comic/" ));

    return QString( dataDir + QString::fromAscii( QUrl::toPercentEncoding( identifier ) ) );
}


CachedProvider::CachedProvider( QObject *parent, const QVariantList &args )
    : ComicProvider( parent, args )
{
    QTimer::singleShot( 0, this, SLOT(triggerFinished()) );
}

CachedProvider::~CachedProvider()
{
}

ComicProvider::IdentifierType CachedProvider::identifierType() const
{
    return StringIdentifier;
}

QImage CachedProvider::image() const
{
    if ( !QFile::exists( identifierToPath( requestedString() ) ) )
        return QImage();

    QImage img;
    img.load( identifierToPath( requestedString() ), "PNG" );

    return img;
}

QString CachedProvider::identifier() const
{
    return requestedString();
}

QString CachedProvider::nextIdentifier() const
{
    QSettings settings( identifierToPath( requestedString() ) + QLatin1String( ".conf" ), QSettings::IniFormat );
    return settings.value( QLatin1String( "nextIdentifier" ), QString() ).toString();
}

QString CachedProvider::previousIdentifier() const
{
    QSettings settings( identifierToPath( requestedString() ) + QLatin1String( ".conf" ), QSettings::IniFormat );
    return settings.value( QLatin1String( "previousIdentifier" ), QString() ).toString();
}

QString CachedProvider::firstStripIdentifier() const
{
    QSettings settings( identifierToPath( requestedComicName() ) + QLatin1String( ".conf" ), QSettings::IniFormat );
    return settings.value( QLatin1String( "firstStripIdentifier" ), QString() ).toString();
}

QString CachedProvider::lastCachedStripIdentifier() const
{
    QSettings settings( identifierToPath( requestedComicName() ) + QLatin1String( ".conf" ), QSettings::IniFormat );
    return settings.value( QLatin1String( "lastCachedStripIdentifier" ), QString() ).toString();
}

QString CachedProvider::comicAuthor() const
{
    QSettings settings( identifierToPath( requestedString() ) + QLatin1String( ".conf" ), QSettings::IniFormat );
    return settings.value( QLatin1String( "comicAuthor" ), QString() ).toString();
}

QString CachedProvider::stripTitle() const
{
    QSettings settings( identifierToPath ( requestedString() ) + QLatin1String( ".conf" ), QSettings::IniFormat );
    return settings.value( QLatin1String( "stripTitle" ), QString() ).toString();
}

QString CachedProvider::additionalText() const
{
    QSettings settings( identifierToPath ( requestedString() ) + QLatin1String( ".conf" ), QSettings::IniFormat );
    return settings.value( QLatin1String( "additionalText" ), QString() ).toString();
}

QString CachedProvider::suffixType() const
{
    QSettings settings( identifierToPath ( requestedComicName() ) + QLatin1String( ".conf" ), QSettings::IniFormat );
    return settings.value( QLatin1String( "suffixType" ), QString() ).toString();
}

QString CachedProvider::name() const
{
    QSettings settings( identifierToPath ( requestedComicName() ) + QLatin1String( ".conf" ), QSettings::IniFormat );
    return settings.value( QLatin1String( "title" ), QString() ).toString();
}

void CachedProvider::triggerFinished()
{
    emit finished( this );
}

bool CachedProvider::isCached( const QString &identifier )
{
    return QFile::exists( identifierToPath( identifier ) );
}

bool CachedProvider::storeInCache( const QString &identifier, const QImage &comic, const Settings &info )
{
    const QString path = identifierToPath( identifier );

    int index = identifier.indexOf( QLatin1Char( ':' ) );
    const QString comicName = identifier.mid( 0, index );
    const QString pathMain = identifierToPath( comicName );
    const QString dirPath = KStandardDirs::locateLocal( "data", QLatin1String( "plasma_engine_comic/" ) );

    if ( !info.isEmpty() ) {
        QSettings settings( path + QLatin1String( ".conf" ), QSettings::IniFormat );
        QSettings settingsMain( pathMain + QLatin1String( ".conf" ), QSettings::IniFormat );

        for ( Settings::const_iterator i = info.constBegin(); i != info.constEnd(); ++i ) {
                if ( ( i.key() == QLatin1String( "firstStripIdentifier" ) ) || ( i.key() == QLatin1String( "title" ) ) ||
                     ( i.key() == QLatin1String( "lastCachedStripIdentifier" ) ) || ( i.key() == QLatin1String( "suffixType" ) ) ||
                     ( i.key() == QLatin1String( "shopUrl" ) ) || ( i.key() == QLatin1String( "isLeftToRight" ) ) ||
                     ( i.key() == QLatin1String( "isTopToBottom" ) ) ) {
                    settingsMain.setValue( i.key(), i.value() );
                } else {
                    settings.setValue( i.key(), i.value() );
                }
        }

        QStringList comics;
        if ( settingsMain.contains( QLatin1String( "comics" ) ) ) {
            comics = settingsMain.value( QLatin1String( "comics" ), QStringList() ).toStringList();
        } else {
            //existing strips haven't been stored in the conf-file yet, do that now, oldest first, newest last
            QDir dir( dirPath );
            comics = dir.entryList( QStringList() << QUrl::toPercentEncoding( comicName + ':' ) + '*', QDir::Files, QDir::Time | QDir::Reversed );
            QStringList::iterator it = comics.begin();
            while ( it != comics.end() ) {
                //only count images, not the conf files
                if ( (*it).endsWith( QLatin1String( ".conf" ) ) ) {
                    it = comics.erase(it);
                } else {
                    ++it;
                }
            }
        }
        comics.append( QUrl::toPercentEncoding( identifier ) );

        const int limit = CachedProvider::maxComicLimit();
        //limit is on
        if ( limit > 0 ) {
            kDebug() << QLatin1String( "MaxComicLimit on." );
            int comicsToRemove = comics.count() - limit;
            QStringList::iterator it = comics.begin();
            while ( comicsToRemove > 0 && it != comics.end() ) {
                kDebug() << QLatin1String( "Remove file" ) << (dirPath + (*it));
                QFile::remove( dirPath + (*it) );
                QFile::remove( dirPath + (*it) + QLatin1String( ".conf" ) );
                it = comics.erase(it);
                -- comicsToRemove;
            }
        }
        settingsMain.setValue( QLatin1String( "comics" ), comics );
    }

    return comic.save( path, "PNG" );
}

KUrl CachedProvider::websiteUrl() const
{
    QSettings settings( identifierToPath( requestedString() ) + QLatin1String( ".conf" ), QSettings::IniFormat );
    return KUrl( settings.value( QLatin1String( "websiteUrl" ), QString() ).toString() );
}

KUrl CachedProvider::imageUrl() const
{
    QSettings settings( identifierToPath( requestedString() ) + QLatin1String( ".conf" ), QSettings::IniFormat );
    return KUrl( settings.value( QLatin1String( "imageUrl" ), QString() ).toString() );
}

KUrl CachedProvider::shopUrl() const
{
    QSettings settings( identifierToPath( requestedComicName() ) + QLatin1String( ".conf" ), QSettings::IniFormat );
    return KUrl( settings.value( QLatin1String( "shopUrl" ), QString() ).toString() );
}

bool CachedProvider::isLeftToRight() const
{
    QSettings settings( identifierToPath( requestedComicName() ) + QLatin1String( ".conf" ), QSettings::IniFormat );
    return settings.value( QLatin1String( "isLeftToRight" ), true ).toBool();
}

bool CachedProvider::isTopToBottom() const
{
    QSettings settings( identifierToPath( requestedComicName() ) + QLatin1String( ".conf" ), QSettings::IniFormat );
    return settings.value( QLatin1String( "isTopToBottom" ), true ).toBool();
}

int CachedProvider::maxComicLimit()
{
    QSettings settings( identifierToPath( QLatin1String( "comic_settings.conf" ) ), QSettings::IniFormat );
    return  qMax( settings.value( QLatin1String( "maxComics" ), CACHE_DEFAULT ).toInt(), 0 );//old value was -1, thus use qMax
}

void CachedProvider::setMaxComicLimit( int limit )
{
    if ( limit < 0 ) {
        kDebug() << "Wrong limit, setting to default.";
        limit = CACHE_DEFAULT;
    }
    QSettings settings( identifierToPath( QLatin1String( "comic_settings.conf" ) ), QSettings::IniFormat );
    settings.setValue( QLatin1String( "maxComics" ), limit );
}

#include "cachedprovider.moc"
