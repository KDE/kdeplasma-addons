/*
 *   SPDX-FileCopyrightText: 2012 Matthias Fuchs <mat69@gmx.net>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef COMIC_DATA_H
#define COMIC_DATA_H

#include "engine/types.h"

#include <KConfigGroup>

// Qt
#include <QImage>
#include <QString>
#include <QUrl>
#include <qqmlregistration.h>

class ComicData
{
    Q_GADGET
    QML_VALUE_TYPE(comicData)

    Q_PROPERTY(Type type READ typeForQml)
    Q_PROPERTY(QString id READ id)
    Q_PROPERTY(QImage image READ image)
    Q_PROPERTY(QString prev READ prev)
    Q_PROPERTY(QString next READ next)
    Q_PROPERTY(QString additionalText READ additionalText)
    Q_PROPERTY(QUrl websiteUrl READ websiteUrl)
    Q_PROPERTY(QString websiteHost READ websiteHost)
    Q_PROPERTY(QUrl shopUrl READ shopUrl)
    Q_PROPERTY(QString first READ first)
    Q_PROPERTY(bool hasFirst READ hasFirst)
    Q_PROPERTY(QString stripTitle READ stripTitle)
    Q_PROPERTY(QString author READ author)
    Q_PROPERTY(QString title READ title)
    Q_PROPERTY(QString current READ current)
    Q_PROPERTY(QString currentReadable READ currentReadable)
    Q_PROPERTY(int firstStripNum READ firstStripNum)
    Q_PROPERTY(int maxStripNum READ maxStripNum)
    Q_PROPERTY(bool isLeftToRight READ isLeftToRight)
    Q_PROPERTY(bool isTopToBottom READ isTopToBottom)
    Q_PROPERTY(bool isError READ isError)
    Q_PROPERTY(bool storePosition READ storePosition)
    Q_PROPERTY(bool ready READ ready)

public:
    enum class Type { // redefined for QML access, keep in sync with engine/types.h
        DateIdentifier,
        NumberIdentifier,
        StringIdentifier,
    };
    Q_ENUM(Type)

    ComicData();

    void init(const QString &id, const KConfigGroup &config);

    void setData(const ComicMetaData &data);

    Q_INVOKABLE bool saveImage(const QUrl &fileUrl);
    Q_INVOKABLE void launchShop();
    Q_INVOKABLE void launchWebsite();

    IdentifierType type() const
    {
        return mComicMetaData.identifierType;
    }

    Type typeForQml() const
    {
        return static_cast<Type>(mComicMetaData.identifierType);
    }

    /**
     * The identifier of the comic, e.g. "garfield"
     */
    QString id() const
    {
        return mId;
    }

    /**
     * The stored comic e.g. "2007-12-21" for a comic of the Date type
     */
    QString stored() const
    {
        return mStored;
    }

    void storePosition(bool store);

    /**
     * The previous comic e.g. "2007-12-21" for a comic of the Date type
     */
    QString prev() const
    {
        return mComicMetaData.previousIdentifier;
    }

    /**
     * The current comic e.g. "2007-12-21" for a comic of the Date type
     */
    QString current() const
    {
        return mCurrent;
    }

    /**
     * The next comic e.g. "2007-12-21" for a comic of the Date type
     */
    QString next() const
    {
        return mComicMetaData.nextIdentifier;
    }

    QString currentReadable() const
    {
        return mCurrentReadable;
    }

    /**
     * The first comic e.g. "2007-12-21" for a comic of the Date type
     */
    QString first() const
    {
        return mComicMetaData.firstStripIdentifier;
    }

    bool hasNext() const
    {
        return !mComicMetaData.nextIdentifier.isEmpty();
    }

    bool hasPrev() const
    {
        return !mComicMetaData.previousIdentifier.isEmpty();
    }

    bool hasFirst() const
    {
        return !mComicMetaData.firstStripIdentifier.isEmpty();
    }

    bool hasStored() const
    {
        return !mStored.isEmpty();
    }

    bool hasImage() const
    {
        return !mComicMetaData.image.isNull();
    }

    QString additionalText() const
    {
        return mComicMetaData.additionalText;
    }

    QString title() const
    {
        return mComicMetaData.providerName;
    }

    QString stripTitle() const
    {
        return mComicMetaData.stripTitle;
    }

    QUrl websiteUrl() const
    {
        return mComicMetaData.websiteUrl;
    }

    QString websiteHost() const
    {
        return mComicMetaData.websiteUrl.host();
    }

    QUrl imageUrl() const
    {
        return mComicMetaData.imageUrl;
    }

    QUrl shopUrl() const
    {
        return mComicMetaData.shopUrl;
    }

    QString author() const
    {
        return mComicMetaData.comicAuthor;
    }

    QImage image() const
    {
        return mComicMetaData.image;
    }

    bool showActualSize() const
    {
        return mShowActualSize;
    }

    bool isLeftToRight() const
    {
        return mComicMetaData.isLeftToRight;
    }

    bool isTopToBottom() const
    {
        return mComicMetaData.isTopToBottom;
    }

    bool storePosition() const
    {
        return !mStored.isEmpty();
    }

    void setShowActualSize(bool show);

    int firstStripNum() const
    {
        return mFirstStripNum;
    }

    int maxStripNum() const
    {
        return mMaxStripNum;
    }

    bool ready() const
    {
        return mReady;
    }

    bool isError() const
    {
        return mComicMetaData.error;
    }

    void save();

private:
    void load();

private:
    ComicMetaData mComicMetaData;
    QString mId;
    QString mLast;
    QString mCurrent;
    QString mStored;
    QString mCurrentReadable;

    // only applicable if the comic is of type Number
    int mFirstStripNum = 0;
    int mMaxStripNum = 0;

    bool mShowActualSize = false;
    bool mReady = false;

    KConfigGroup mCfg;
};

#endif
