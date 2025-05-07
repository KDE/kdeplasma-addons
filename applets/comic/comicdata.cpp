/*
 *   SPDX-FileCopyrightText: 2012 Matthias Fuchs <mat69@gmx.net>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "comicdata.h"

#include <KIO/OpenUrlJob>
#include <KLocalizedString>

ComicData::ComicData()
{
}

void ComicData::init(const QString &id, const KConfigGroup &config)
{
    mId = id;
    mCfg = config;
    load();
}

void ComicData::load()
{
    mShowActualSize = mCfg.readEntry(QLatin1String("scaleToContent_") + mId, false);
    mMaxStripNum = mCfg.readEntry(QLatin1String("maxStripNum_") + mId, 0);
    mStored = mCfg.readEntry(QLatin1String("storedPosition_") + mId, QString());
}

void ComicData::save()
{
    mCfg.writeEntry(QLatin1String("scaleToContent_") + mId, mShowActualSize);
    mCfg.writeEntry(QLatin1String("maxStripNum_") + mId, mMaxStripNum);
    mCfg.writeEntry(QLatin1String("storedPosition_") + id(), mStored);

    // no next, thus the most recent strip
    if (!hasNext()) {
        mCfg.writeEntry(QLatin1String("lastStripVisited_") + mId, true);
        mCfg.writeEntry(QLatin1String("lastStrip_") + mId, mLast);
    }
}

void ComicData::setShowActualSize(bool show)
{
    mShowActualSize = show;
    save();
}

void ComicData::storePosition(bool store)
{
    mStored = (store ? mCurrent : QString());
    save();
}

void ComicData::setData(const ComicMetaData &data)
{
    mComicMetaData = data;

    if (data.error) {
        mComicMetaData.image = QImage();
        mComicMetaData.additionalText = QString();
    } else {
        mReady = true;
    }

    QString temp = data.identifier;
    mCurrent = temp.remove(mId + QLatin1Char(':'));

    // found a new last identifier
    if (!hasNext()) {
        mLast = mCurrent;
    }

    mCurrentReadable.clear();
    if (mComicMetaData.identifierType == IdentifierType::NumberIdentifier) {
        mCurrentReadable = i18nc("an abbreviation for Number", "# %1", mCurrent);
        int tempNum = mCurrent.toInt();
        if (mMaxStripNum < tempNum) {
            mMaxStripNum = tempNum;
        }
        temp = mComicMetaData.firstStripIdentifier.remove(mId + QLatin1Char(':'));
        mFirstStripNum = temp.toInt();
    } else if (mComicMetaData.identifierType == IdentifierType::DateIdentifier && QDate::fromString(temp, QStringLiteral("yyyy-MM-dd")).isValid()) {
        mCurrentReadable = mCurrent;
    } else if (mComicMetaData.identifierType == IdentifierType::StringIdentifier) {
        mCurrentReadable = mCurrent;
    }

    save();
}

bool ComicData::saveImage(const QUrl &fileUrl)
{
    if (!fileUrl.isValid()) {
        return false;
    }

    return mComicMetaData.image.save(fileUrl.toLocalFile(), "PNG");
}

void ComicData::launchWebsite()
{
    auto *job = new KIO::OpenUrlJob(mComicMetaData.websiteUrl);
    job->start();
}

void ComicData::launchShop()
{
    auto *job = new KIO::OpenUrlJob(mComicMetaData.shopUrl);
    job->start();
}
