/*
 *   SPDX-FileCopyrightText: 2011 Matthias Fuchs <mat69@gmx.net>
 *   SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *   SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "comicarchivejob.h"

#include <KLocalizedString>
#include <KZip>
#include <QDebug>
#include <QTemporaryFile>

#include <QImage>

ComicArchiveJob::ComicArchiveJob(const QUrl &dest,
                                 ComicEngine *engine,
                                 ComicArchiveJob::ArchiveType archiveType,
                                 IdentifierType identifierType,
                                 const QString &pluginName,
                                 QObject *parent)
    : KJob(parent)
    , mType(archiveType)
    , mDirection(Undefined)
    , mIdentifierType(identifierType)
    , mSuspend(false)
    , mFindAmount(true)
    , mHasVariants(false)
    , mDone(false)
    , mComicNumber(0)
    , mProcessedFiles(0)
    , mTotalFiles(-1)
    , mEngine(engine)
    , mZipFile(new QTemporaryFile)
    , mZip(nullptr)
    , mPluginName(pluginName)
    , mDest(dest)
{
    if (mZipFile->open()) {
        mZip = new KZip(mZipFile->fileName());
        mZip->open(QIODevice::ReadWrite);
        mZip->setCompression(KZip::NoCompression);
        setCapabilities(Killable | Suspendable);
    } else {
        qWarning() << "Could not create a temporary file for the zip file.";
    }
}

ComicArchiveJob::~ComicArchiveJob()
{
    emitResultIfNeeded();
    delete mZip;
    delete mZipFile;
    qDeleteAll(mBackwardFiles);
}

bool ComicArchiveJob::isValid() const
{
    if (mPluginName.isEmpty()) {
        qWarning() << "No plugin name specified.";
        return false;
    }

    switch (mType) {
    case ArchiveFromTo:
        if (mToIdentifier.isEmpty() || mFromIdentifier.isEmpty()) {
            qWarning() << "Not enough data provided to archive a range.";
            return false;
        }
        break;
    case ArchiveStartTo:
    case ArchiveEndTo:
        if (mToIdentifier.isEmpty()) {
            qWarning() << "Not enough data provided to archive StartTo/EndTo.";
            return false;
        }
        break;
    default:
        break;
    }

    return mZip && mZip->isOpen();
}

void ComicArchiveJob::setToIdentifier(const QString &toIdentifier)
{
    mToIdentifier = toIdentifier;
    mToIdentifierSuffix = mToIdentifier;
    mToIdentifierSuffix.remove(mPluginName + QLatin1Char(':'));
}

void ComicArchiveJob::setFromIdentifier(const QString &fromIdentifier)
{
    mFromIdentifier = fromIdentifier;
    mFromIdentifierSuffix = mFromIdentifier;
    mFromIdentifierSuffix.remove(mPluginName + QLatin1Char(':'));
}

void ComicArchiveJob::start()
{
    switch (mType) {
    case ArchiveAll:
        requestComic(suffixToIdentifier(QString()));
        break;
    case ArchiveStartTo:
        requestComic(mToIdentifier);
        break;
    case ArchiveEndTo: {
        setFromIdentifier(mToIdentifier);
        mToIdentifier.clear();
        mToIdentifierSuffix.clear();
        requestComic(suffixToIdentifier(QString()));
        break;
    }
    case ArchiveFromTo:
        mDirection = Forward;
        defineTotalNumber();
        requestComic(mFromIdentifier);
        break;
    }
}

void ComicArchiveJob::dataUpdated(const QString &source, const ComicMetaData &data)
{
    if (!mZip) {
        qWarning() << "No zip file, aborting.";
        setErrorText(i18n("No zip file is existing, aborting."));
        setError(KilledJobError);
        emitResultIfNeeded();
        return;
    }

    const QString currentIdentifier = data.identifier;
    QString currentIdentifierSuffix = currentIdentifier;
    currentIdentifierSuffix.remove(mPluginName + QLatin1Char(':'));

    const QImage image = data.image;
    const bool hasError = data.error || image.isNull();
    const QString previousIdentifierSuffix = data.previousIdentifier;
    const QString nextIdentifierSuffix = data.nextIdentifier;
    const QString firstIdentifierSuffix = data.firstStripIdentifier;

    mAuthors << data.comicAuthor.split(QLatin1Char(','), Qt::SkipEmptyParts);
    mAuthors.removeDuplicates();

    if (mComicTitle.isEmpty()) {
        mComicTitle = data.stripTitle;
    }

    if (hasError) {
        qWarning() << "An error occurred at" << source << "stopping.";
        setErrorText(i18n("An error happened for identifier %1.", source));
        setError(KilledJobError);
        copyZipFileToDestination();
        return;
    }

    if (mDirection == Undefined) {
        if ((mType == ArchiveAll) || (mType == ArchiveStartTo)) {
            if (!firstIdentifierSuffix.isEmpty()) {
                setFromIdentifier(suffixToIdentifier(firstIdentifierSuffix));
            }
            if (mType == ArchiveAll) {
                setToIdentifier(currentIdentifier);
            }
            mDirection = (firstIdentifierSuffix.isEmpty() ? Backward : Forward);
            if (mDirection == Forward) {
                requestComic(suffixToIdentifier(firstIdentifierSuffix));
                return;
            } else {
                // backward, i.e. the to identifier is unknown
                mToIdentifier.clear();
                mToIdentifierSuffix.clear();
            }
        } else if (mType == ArchiveEndTo) {
            mDirection = Forward;
            setToIdentifier(currentIdentifier);
            requestComic(mFromIdentifier);
            return;
        }
    }

    bool worked = false;
    ++mProcessedFiles;
    if (mDirection == Forward) {
        QTemporaryFile tempFile;
        worked = tempFile.open();
        worked = worked && tempFile.flush();
        worked = (worked ? image.save(tempFile.fileName(), "PNG") : worked);
        worked = (worked ? addFileToZip(tempFile.fileName()) : worked);

        if (worked) {
            if ((currentIdentifier == mToIdentifier) || (currentIdentifierSuffix == nextIdentifierSuffix) || nextIdentifierSuffix.isEmpty()) {
                qDebug() << "Done downloading at:" << source;
                copyZipFileToDestination();
            } else {
                requestComic(suffixToIdentifier(nextIdentifierSuffix));
            }
        }
    } else if (mDirection == Backward) {
        QTemporaryFile *tempFile = new QTemporaryFile;
        mBackwardFiles << tempFile;
        worked = tempFile->open();
        worked = worked && tempFile->flush();
        worked = (worked ? image.save(tempFile->fileName(), "PNG") : worked);

        if (worked) {
            if ((currentIdentifier == mToIdentifier) || (currentIdentifierSuffix == previousIdentifierSuffix) || previousIdentifierSuffix.isEmpty()) {
                qDebug() << "Done downloading at:" << source;
                createBackwardZip();
            } else {
                requestComic(suffixToIdentifier(previousIdentifierSuffix));
            }
        }
    }

    defineTotalNumber(currentIdentifierSuffix);
    setProcessedAmount(Files, mProcessedFiles);
    if (mTotalFiles != -1) {
        setPercent((100 * mProcessedFiles) / mTotalFiles);
    }

    if (!worked) {
        qWarning() << "Could not write the file, identifier:" << source;
        setErrorText(i18n("Failed creating the file with identifier %1.", source));
        setError(KilledJobError);
        emitResultIfNeeded();
    }
}

bool ComicArchiveJob::doKill()
{
    mSuspend = true;
    return KJob::doKill();
}

bool ComicArchiveJob::doSuspend()
{
    mSuspend = true;
    return true;
}

bool ComicArchiveJob::doResume()
{
    mSuspend = false;
    if (!mRequest.isEmpty()) {
        requestComic(mRequest);
    }
    return true;
}

void ComicArchiveJob::defineTotalNumber(const QString &currentSuffix)
{
    findTotalNumberFromTo();
    if (mTotalFiles == -1) {
        qDebug() << "Unable to find the total number for" << mPluginName;
        return;
    }

    // calculate a new value for total files, can be different from the previous one,
    // if there are no strips for certain days/numbers
    if (!currentSuffix.isEmpty()) {
        if (mIdentifierType == Date) {
            const QDate current = QDate::fromString(currentSuffix, QStringLiteral("yyyy-MM-dd"));
            const QDate to = QDate::fromString(mToIdentifierSuffix, QStringLiteral("yyyy-MM-dd"));
            if (current.isValid() && to.isValid()) {
                // processed files + files still to download
                mTotalFiles = mProcessedFiles + qAbs(current.daysTo(to));
            }
        } else if (mIdentifierType == Number) {
            bool result = true;
            bool ok;
            const int current = currentSuffix.toInt(&ok);
            result = (result && ok);
            const int to = mToIdentifierSuffix.toInt(&ok);
            result = (result && ok);
            if (result) {
                // processed files + files still to download
                mTotalFiles = mProcessedFiles + qAbs(to - current);
            }
        }
    }

    if (mTotalFiles != -1) {
        setTotalAmount(Files, mTotalFiles);
    }
}

void ComicArchiveJob::findTotalNumberFromTo()
{
    if (mTotalFiles != -1) {
        return;
    }

    if (mIdentifierType == Date) {
        const QDate from = QDate::fromString(mFromIdentifierSuffix, QStringLiteral("yyyy-MM-dd"));
        const QDate to = QDate::fromString(mToIdentifierSuffix, QStringLiteral("yyyy-MM-dd"));
        if (from.isValid() && to.isValid()) {
            mTotalFiles = qAbs(from.daysTo(to)) + 1;
        }
    } else if (mIdentifierType == Number) {
        bool result = true;
        bool ok;
        const int from = mFromIdentifierSuffix.toInt(&ok);
        result = (result && ok);
        const int to = mToIdentifierSuffix.toInt(&ok);
        result = (result && ok);
        if (result) {
            mTotalFiles = qAbs(to - from) + 1;
        }
    }
}

QString ComicArchiveJob::suffixToIdentifier(const QString &suffix) const
{
    return mPluginName + QLatin1Char(':') + suffix;
}

void ComicArchiveJob::requestComic(QString identifier) // krazy:exclude=passbyvalue
{
    mRequest.clear();
    if (mSuspend) {
        mRequest = identifier;
        return;
    }

    Q_EMIT description(this,
                       i18n("Creating Comic Book Archive"),
                       qMakePair(QStringLiteral("source"), identifier),
                       qMakePair(QStringLiteral("destination"), mDest.toString()));

    mEngine->requestSource(identifier, [this, identifier](const auto &data) {
        dataUpdated(identifier, data);
    });
}

bool ComicArchiveJob::addFileToZip(const QString &path)
{
    // We use 6 signs, e.g. number 1 --> 000001.png, 123 --> 000123.png
    // this way the comics should always be correctly sorted (otherwise evince e.g. has problems)
    static const int numSigns = 6;
    static const QString zero = QLatin1String("0");
    QString number = QString::number(++mComicNumber);
    const int length = number.length();
    if (length < numSigns) {
        number = zero.repeated(numSigns - length) + number;
    }

    return mZip->addLocalFile(path, number + QLatin1String(".png"));
}

void ComicArchiveJob::createBackwardZip()
{
    for (int i = mBackwardFiles.count() - 1; i >= 0; --i) {
        if (!addFileToZip(mBackwardFiles[i]->fileName())) {
            qWarning() << "Failed adding a file to the archive.";
            setErrorText(i18n("Failed adding a file to the archive."));
            setError(KilledJobError);
            emitResultIfNeeded();
            return;
        }
    }

    copyZipFileToDestination();
}

void ComicArchiveJob::copyZipFileToDestination()
{
    mZip->close();

    KIO::FileCopyJob *job = KIO::file_copy(QUrl::fromLocalFile(mZipFile->fileName()), mDest);

    const bool worked = job->exec();

    if (!worked) {
        qWarning() << "Could not copy the zip file to the specified destination:" << mDest;
        setErrorText(i18n("Could not create the archive at the specified location."));
        setError(KilledJobError);
        emitResultIfNeeded();
        return;
    }

    emitResultIfNeeded();
}

void ComicArchiveJob::emitResultIfNeeded()
{
    if (!mDone) {
        mDone = true;
        emitResult();
    }
}
