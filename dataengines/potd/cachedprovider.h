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

#ifndef CACHEDPROVIDER_H
#define CACHEDPROVIDER_H

#include <QImage>
#include <QRunnable>

#include "potdprovider.h"

/**
 * This class provides pictures from the local cache.
 */
class CachedProvider : public PotdProvider
{
    Q_OBJECT

    public:
        /**
         * Creates a new cached provider.
         *
         * @param identifier The identifier of the cached picture.
         * @param parent The parent object.
         */
        CachedProvider( const QString &identifier, QObject *parent );

        /**
         * Destroys the cached provider.
         */
        ~CachedProvider();

        /**
         * Returns the requested image.
         *
         * Note: This method returns only a valid image after the
         *       finished() signal has been emitted.
         */
        virtual QImage image() const;

        /**
         * Returns the identifier of the picture request (name + date).
         */
        virtual QString identifier() const;

        /**
         * Returns whether a picture with the given @p identifier is cached.
         */
        static bool isCached( const QString &identifier, bool ignoreAge = false );

        /**
         * Returns a path for the given identifier
         */
        static QString identifierToPath( const QString &identifier );

    private Q_SLOTS:
        void triggerFinished(const QImage &image);

    private:
        QString mIdentifier;
        QImage mImage;
};

class LoadImageThread : public QObject, public QRunnable
{
    Q_OBJECT

public:
    LoadImageThread(const QString &filePath);
    void run();

Q_SIGNALS:
    void done(const QImage &pixmap);

private:
    QString m_filePath;
};

class SaveImageThread : public QObject, public QRunnable
{
    Q_OBJECT

public:
    SaveImageThread(const QString &identifier, const QImage &image);
    void run();

Q_SIGNALS:
    void done( const QString &source, const QString &path, const QImage &img );

private:
    QImage m_image;
    QString m_identifier;
};

#endif
