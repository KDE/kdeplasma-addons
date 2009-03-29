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
    const QString dataDir = KStandardDirs::locateLocal( "data", "plasma_engine_comic/" );

    return QString( dataDir + QUrl::toPercentEncoding( identifier ) );
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
    QSettings settings( identifierToPath( requestedString() ) + ".conf", QSettings::IniFormat );
    return settings.value( "nextIdentifier", QString() ).toString();
}

QString CachedProvider::previousIdentifier() const
{
    QSettings settings( identifierToPath( requestedString() ) + ".conf", QSettings::IniFormat );
    return settings.value( "previousIdentifier", QString() ).toString();
}

QString CachedProvider::firstStripIdentifier() const
{
    QSettings settings( identifierToPath( requestedComicName() ) + ".conf", QSettings::IniFormat );
    return settings.value( "firstStripIdentifier", QString() ).toString();
}

QString CachedProvider::lastCachedStripIdentifier() const
{
    QSettings settings( identifierToPath( requestedComicName() ) + ".conf", QSettings::IniFormat );
    return settings.value( "lastCachedStripIdentifier", QString() ).toString();
}

QString CachedProvider::comicAuthor() const
{
    QSettings settings( identifierToPath( requestedString() ) + ".conf", QSettings::IniFormat );
    return settings.value( "comicAuthor", QString() ).toString();
}

QString CachedProvider::stripTitle() const
{
    QSettings settings( identifierToPath ( requestedString() ) + ".conf", QSettings::IniFormat );
    return settings.value( "stripTitle", QString() ).toString();
}

QString CachedProvider::additionalText() const
{
    QSettings settings( identifierToPath ( requestedString() ) + ".conf", QSettings::IniFormat );
    return settings.value( "additionalText", QString() ).toString();
}

QString CachedProvider::suffixType() const
{
    QSettings settings( identifierToPath ( requestedComicName() ) + ".conf", QSettings::IniFormat );
    return settings.value( "suffixType", QString() ).toString();
}

QString CachedProvider::name() const
{
    QSettings settings( identifierToPath ( requestedComicName() ) + ".conf", QSettings::IniFormat );
    return settings.value( "title", QString() ).toString();
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

    int index = identifier.indexOf( ':' );
    const QString pathMain = identifierToPath( identifier.mid( 0, index ) );

    if ( !info.isEmpty() ) {
        QSettings settings( path + ".conf", QSettings::IniFormat );
        QSettings settingsMain( pathMain + ".conf", QSettings::IniFormat );

        for ( Settings::const_iterator i = info.constBegin(); i != info.constEnd(); ++i ) {
                if ( ( i.key() == "firstStripIdentifier" ) || ( i.key() == "title" ) ||
                     ( i.key() == "lastCachedStripIdentifier" ) || ( i.key() == "suffixType" ) ||
                     ( i.key() == "shopUrl" ) || ( i.key() == "isLeftToRight" ) ||
                     ( i.key() == "isTopToBottom" ) ) {
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
    QSettings settings( identifierToPath( requestedString() ) + ".conf", QSettings::IniFormat );
    return KUrl( settings.value( "websiteUrl", QString() ).toString() );
}

KUrl CachedProvider::shopUrl() const
{
    QSettings settings( identifierToPath( requestedComicName() ) + ".conf", QSettings::IniFormat );
    return KUrl( settings.value( "shopUrl", QString() ).toString() );
}

bool CachedProvider::isLeftToRight() const
{
    QSettings settings( identifierToPath( requestedComicName() ) + ".conf", QSettings::IniFormat );
    return settings.value( "isLeftToRight", true ).toBool();
}

bool CachedProvider::isTopToBottom() const
{
    QSettings settings( identifierToPath( requestedComicName() ) + ".conf", QSettings::IniFormat );
    return settings.value( "isTopToBottom", true ).toBool();
}

#include "cachedprovider.moc"
