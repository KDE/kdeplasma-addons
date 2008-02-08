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

#include <QtCore/QFile>
#include <QtCore/QTimer>
#include <QtGui/QImage>

#include <kstandarddirs.h>
#include <KUrl>

#include "cachedprovider.h"

static QString identifierToPath( const QString &identifier )
{
    const QString dataDir = KStandardDirs::locateLocal( "data", "plasma_engine_comic/" );

    return QString( dataDir + identifier );
}


CachedProvider::CachedProvider( const QString &identifier, QObject *parent )
    : ComicProvider( parent ), mIdentifier( identifier )
{
    mSettings = new QSettings( identifierToPath( mIdentifier )+".conf", QSettings::IniFormat );

    QTimer::singleShot( 0, this, SLOT( triggerFinished() ) );
}

CachedProvider::~CachedProvider()
{
   delete mSettings;
}

QImage CachedProvider::image() const
{
    if ( !QFile::exists( identifierToPath( mIdentifier ) ) )
        return QImage();

    QImage img;
    img.load( identifierToPath( mIdentifier ), "PNG" );

    return img;
}

QString CachedProvider::identifier() const
{
    return mIdentifier;
}

QString CachedProvider::nextIdentifierSuffix() const
{
   return mSettings->value("nextIdentifierSuffix", QString()).toString();
}

QString CachedProvider::previousIdentifierSuffix() const
{
   return mSettings->value("previousIdentifierSuffix", QString()).toString();
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

    if (!info.isEmpty()) {
        QSettings settings( path+".conf", QSettings::IniFormat );

        for(Settings::const_iterator i = info.constBegin();
            i != info.constEnd(); ++i) {
            settings.setValue(i.key(), i.value());
        }
    }

    return comic.save( path, "PNG" );
}

KUrl CachedProvider::websiteUrl() const
{
    return KUrl(mSettings->value("websiteUrl", QString()).toString());
}
