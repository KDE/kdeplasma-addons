/*
 *   SPDX-FileCopyrightText: 2011 Matthias Fuchs <mat69@gmx.net>
 *   SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef COMIC_ARCHIVE_JOB_H
#define COMIC_ARCHIVE_JOB_H

#include "comicinfo.h"

#include <KIO/Job>
#include <Plasma/DataEngine>

class QTemporaryFile;
class KZip;

class ComicArchiveJob : public KJob
{
    Q_OBJECT

public:
    enum ArchiveType {
        ArchiveAll = 0,
        ArchiveStartTo,
        ArchiveEndTo,
        ArchiveFromTo,
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
    ComicArchiveJob(const QUrl &dest,
                    Plasma::DataEngine *engine,
                    ArchiveType archiveType,
                    IdentifierType identifierType,
                    const QString &pluginName,
                    QObject *parent = nullptr);
    ~ComicArchiveJob() override;

    /**
     * Checks if all the needed data has been set
     */
    bool isValid() const;

    /**
     * Sets the end to toIdentifier.
     * Keep in mind that depending on the ArchiveType this might be ignored
     */
    void setToIdentifier(const QString &toIdentifier);

    /**
     * Sets the beginning to toIdentifier.
     * Keep in mind that depending on the ArchiveType this might be ignored
     */
    void setFromIdentifier(const QString &fromIdentifier);

    void start() override;

public Q_SLOTS:
    void dataUpdated(const QString &source, const Plasma::DataEngine::Data &data);

protected:
    bool doKill() override;
    bool doSuspend() override;
    bool doResume() override;

private:
    /**
     * Sets the total number of comics to download.
     * @param currentSuffix if empty the from and to identifier suffix will be used.
     * If a currentSuffix is defined it will check if the total number is different
     * e.g. not a comic defined for every day etc.
     */
    void defineTotalNumber(const QString &currentSuffix = QString());

    /**
     * Sets mTotalFiles if that is -1 and it can calculate at total number
     * base on the from and to identifier suffix
     */
    void findTotalNumberFromTo();

    QString suffixToIdentifier(const QString &suffix) const;
    void requestComic(QString identifier);
    bool addFileToZip(const QString &path);

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
        Forward,
        Backward,
    };

    ArchiveType mType;
    ArchiveDirection mDirection;
    IdentifierType mIdentifierType;
    bool mSuspend;
    bool mFindAmount;
    bool mHasVariants;
    bool mDone;
    int mComicNumber;
    int mProcessedFiles;
    int mTotalFiles;
    Plasma::DataEngine *mEngine;
    QTemporaryFile *mZipFile;
    KZip *mZip;
    QString mPluginName;
    QString mToIdentifier;
    QString mToIdentifierSuffix;
    QString mFromIdentifier;
    QString mFromIdentifierSuffix;
    QString mComicTitle;
    QString mRequest;
    const QUrl mDest;
    QStringList mAuthors;
    QList<QTemporaryFile *> mBackwardFiles;
};

#endif
