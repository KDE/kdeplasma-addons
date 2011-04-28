/***************************************************************************
 *   Copyright (C) 2011 Matthias Fuchs <mat69@gmx.net>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef COMIC_ARCHIVE_JOB_H
#define COMIC_ARCHIVE_JOB_H

#include <KIO/Job>
#include <Plasma/DataEngine>

class KTemporaryFile;
class KZip;

class ComicArchiveJob : public KJob
{
    Q_OBJECT

    public:
        enum ArchiveType {
            ArchiveAll = 0,
            ArchiveStartTo,
            ArchiveEndTo,
            ArchiveFromTo
        };

        /**
         * Creates a comic archive job.
         * The engine has to be a working comic dataengine.
         * The archiveType defines what kind of input is given, e.g. if ArchiveAll is
         * used no other parameters need to be defined, while ArchiveFromTo needs
         * both toIdentifier and fromIdentifier (from <= to!), the other two types need only the toIdentifier.
         * You need to define the plugin name in any case, this is part of the identifier e.g.
         * "garfield:2010-03-04", here "garfield" is the plugin name
         * @see setToIdentifier, setFromIdentifier
         */
        ComicArchiveJob( const KUrl &dest, Plasma::DataEngine *engine, ArchiveType archiveType, const QString &pluginName, QObject *parent = 0 );
        ~ComicArchiveJob();

        /**
         * Checks if all the needed data has been set
         */
        bool isValid() const;

        /**
         * Sets the end to toIdentifier.
         * Keep in mind that depending on the ArchiveType this might be ignored
         */
        void setToIdentifier( const QString &toIdentifier );

        /**
         * Sets the beginning to toIdentifier.
         * Keep in mind that depending on the ArchiveType this might be ignored
         */
        void setFromIdentifier( const QString &fromIdentifier );

        virtual void start();

    public slots:
        void dataUpdated( const QString &source, const Plasma::DataEngine::Data& data );

    protected:
        virtual bool doKill();
        virtual bool doSuspend();
        virtual bool doResume();

    private:
        QString suffixToIdentifier( const QString &suffix ) const;
        void requestComic( QString identifier );
        bool addFileToZip( const QString &path );

        /**
         * If the ArchiveDirection is Backward, this will fill the zip
         * with mBackwardFiles (beginning from the back), and will call
         * copyZipFileToDestination afterwards
         */
        void createBackwardZip();
        void copyZipFileToDestination();

        void emitResultIfNeeded();

    private:
        enum ArchiveDirection {
            Undefined,
            Foward,
            Backward
        };

        ArchiveType mType;
        ArchiveDirection mDirection;
        bool mSuspend;
        bool mFindAmount;
        bool mHasVariants;
        bool mDone;
        int mComicNumber;
        int mProcessedFiles;
        Plasma::DataEngine *mEngine;
        KTemporaryFile *mZipFile;
        KZip *mZip;
        QString mPluginName;
        QString mToIdentifier;
        QString mFromIdentifier;
        QString mComicTitle;
        QString mRequest;
        const KUrl mDest;
        QStringList mAuthors;
        QList< KTemporaryFile* > mBackwardFiles;
};

#endif
