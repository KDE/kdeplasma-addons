/*
 *   Copyright (C) 2007 Tobias Koenig <tokoe@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify  
 *   it under the terms of the GNU General Public License as published by  
 *   the Free Software Foundation; either version 2 of the License, or     
 *   (at your option) any later version.   
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
#include <QtCore/QFileInfo>
#include <QtCore/QTimer>
#include <QtCore/QThreadPool>
#include <QtGui/QImage>

#include <KDebug>

#include <kstandarddirs.h>

LoadImageThread::LoadImageThread(const QString &filePath)
    : m_filePath(filePath)
{
}

void LoadImageThread::run()
{
    QImage image;
    image.load(m_filePath );
    emit done(image);
}

SaveImageThread::SaveImageThread(const QString &identifier, const QImage &image)
    : m_image(image),
      m_identifier(identifier)
{
}

void SaveImageThread::run()
{
    const QString path = CachedProvider::identifierToPath( m_identifier );
    m_image.save(path, "PNG");
    emit done( m_identifier, path, m_image );
}

QString CachedProvider::identifierToPath( const QString &identifier )
{
    const QString dataDir = KStandardDirs::locateLocal( "data", QLatin1String("plasma_engine_podt/") );
    return QString( dataDir + identifier );
}


CachedProvider::CachedProvider( const QString &identifier, QObject *parent )
    : PotdProvider( parent ), mIdentifier( identifier )
{
    LoadImageThread *thread = new LoadImageThread( identifierToPath( mIdentifier ) );
    connect(thread, SIGNAL(done(QImage)), this, SLOT(triggerFinished(QImage)));
    QThreadPool::globalInstance()->start(thread);
}

CachedProvider::~CachedProvider()
{
}

QImage CachedProvider::image() const
{
    return mImage;
}

QString CachedProvider::identifier() const
{
    return mIdentifier;
}

void CachedProvider::triggerFinished(const QImage &image)
{
    mImage = image;
    emit finished( this );
}

bool CachedProvider::isCached( const QString &identifier, bool ignoreAge )
{
    const QString path = identifierToPath( identifier );
    if (!QFile::exists( path ) ) {
        return false;
    }

    if (!ignoreAge && !identifier.contains( ':' ) ) {
        // no date in the identifier, so it's a daily; check to see ifthe modification time is today
        QFileInfo info( path );
        if ( info.lastModified().daysTo( QDateTime::currentDateTime() ) > 1 ) {
            return false;
        }
    }

    return true;
}

#include "cachedprovider.moc"

