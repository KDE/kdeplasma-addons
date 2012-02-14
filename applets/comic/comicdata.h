/***************************************************************************
 *   Copyright (C) 2012 Matthias Fuchs <mat69@gmx.net>                     *
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

#ifndef COMIC_DATA_H
#define COMIC_DATA_H

#include "comicinfo.h"

#include <KUrl>
#include <Plasma/DataEngine>

#include <QtCore/QString>

class ComicData
{
    public:
        ComicData();

        void init(const QString &id, const KConfigGroup &config);

        void setData(const Plasma::DataEngine::Data &data);

        IdentifierType type() const { return mType; }

        /**
         * The identifier of the comic, e.g. "garfield"
         */
        QString id() const { return mId; }

        /**
         * The stored comic e.g. "2007-12-21" for a comic of the Date type
         */
        QString stored() const { return mStored; }

        void storePosition(bool store);

        /**
         * The previous comic e.g. "2007-12-21" for a comic of the Date type
         */
        QString prev() const { return mPrev; }

        /**
         * The current comic e.g. "2007-12-21" for a comic of the Date type
         */
        QString current() const { return mCurrent; }

        /**
         * The next comic e.g. "2007-12-21" for a comic of the Date type
         */
        QString next() const { return mNext; }

        QString currentReadable() const { return mCurrentReadable; }

        /**
         * The first comic e.g. "2007-12-21" for a comic of the Date type
         */
        QString first() const { return mFirst; }

        bool hasNext() const { return !mNext.isEmpty(); }

        bool hasPrev() const { return !mPrev.isEmpty(); }

        bool hasFirst() const { return !mFirst.isEmpty(); }

        bool hasStored() const { return !mStored.isEmpty(); }

        bool hasImage() const { return !mImage.isNull(); }

        QString additionalText() const { return mAdditionalText; }

        QString title() const { return mTitle; }
        void setTitle(const QString &title) { mTitle = title; }


        QString stripTitle() const { return mStripTitle; }

        KUrl websiteUrl() const { return mWebsiteUrl; }

        KUrl imageUrl() const { return mImageUrl; }

        KUrl shopUrl() const { return mShopUrl; }

        QString author() const { return mAuthor; }

        QImage image() const { return mImage; }

        bool scaleComic() const { return mScaleComic; }
        bool isLeftToRight() const { return mIsLeftToRight; }
        bool isTopToBottom() const { return mIsTopToBottom; }
        bool storePosition() const { return !mStored.isEmpty(); }

        void setScaleComic(bool scale);


        QString errorStrip() const { return mErrorStrip; }

        int firstStripNum() const { return mFirstStripNum; }
        int maxStripNum() const { return mMaxStripNum; }

        void save();

    private:
        void load();
        void createErrorPicture(const Plasma::DataEngine::Data &data);

    private:

        IdentifierType mType;
        QString mId;
        QString mFirst;
        QString mLast;
        QString mCurrent;
        QString mNext;
        QString mPrev;
        QString mStored;
        QString mCurrentReadable;

        QString mErrorStrip;

        QString mAuthor;
        QString mTitle;
        QString mStripTitle;
        QString mAdditionalText;
        KUrl mWebsiteUrl;
        KUrl mImageUrl;
        KUrl mShopUrl;

        QImage mImage;

        // only applicable if the comic is of type Number
        int mFirstStripNum;
        int mMaxStripNum;

        bool mScaleComic;
        bool mIsLeftToRight;
        bool mIsTopToBottom;

        KConfigGroup mCfg;
};

#endif
