/*
 *   SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "cachedprovider.h"

#include <QFile>
#include <QFileInfo>
#include <QTimer>
#include <QThreadPool>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>
#include <QRegularExpression>

#include <QDebug>

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
    m_image.save(path, "JPEG");
    emit done( m_identifier, path, m_image );
}

QString CachedProvider::identifierToPath( const QString &identifier )
{
    const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + QLatin1String("/plasma_engine_potd/");
    QDir d;
    d.mkpath(dataDir);
    return dataDir + identifier;
}


CachedProvider::CachedProvider( const QString &identifier, QObject *parent )
    : PotdProvider( parent ), mIdentifier( identifier )
{
    LoadImageThread *thread = new LoadImageThread( identifierToPath( mIdentifier ) );
    connect(thread, &LoadImageThread::done, this, &CachedProvider::triggerFinished);
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

    QRegularExpression re(QLatin1String(":\\d{4}-\\d{2}-\\d{2}"));

    if (!ignoreAge && !re.match(identifier).hasMatch()) {
        // no date in the identifier, so it's a daily; check to see ifthe modification time is today
        QFileInfo info( path );
        if ( info.lastModified().daysTo( QDateTime::currentDateTime() ) >= 1 ) {
            return false;
        }
    }

    return true;
}


