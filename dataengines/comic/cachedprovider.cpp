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

#include <QtCore/QFile>
#include <QtCore/QSettings>
#include <QtCore/QTimer>
#include <QtGui/QImage>

#include <kstandarddirs.h>
#include <KUrl>


static QString identifierToPath( const QString &identifier )
{
    const QString dataDir = KStandardDirs::locateLocal( "data", QLatin1String( "plasma_engine_comic/" ));

    return QString( dataDir + QString::fromAscii( QUrl::toPercentEncoding( identifier ) ) );
}


CachedProvider::CachedProvider( QObject *parent, const QVariantList &args )
    : ComicProvider( parent, args )
{
    QTimer::singleShot( 0, this, SLOT( triggerFinished() ) );
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
    const QString pathMain = identifierToPath( identifier.mid( 0, index ) );

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
    }

    return comic.save( path, "PNG" );
}

KUrl CachedProvider::websiteUrl() const
{
    QSettings settings( identifierToPath( requestedString() ) + QLatin1String( ".conf" ), QSettings::IniFormat );
    return KUrl( settings.value( QLatin1String( "websiteUrl" ), QString() ).toString() );
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

#include "cachedprovider.moc"
