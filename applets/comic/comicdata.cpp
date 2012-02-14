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

#include "comicdata.h"

#include <Plasma/Theme>

#include <QtGui/QPainter>

ComicData::ComicData()
{
}

void ComicData::init(const QString &id, const KConfigGroup &config)
{
    mId = id;
    mCfg = config;
    mFirstStripNum = 0;
    mMaxStripNum = 0;
    mScaleComic = false;
    mIsLeftToRight = true;
    mIsTopToBottom = true;

    load();
}

void ComicData::load()
{
    mScaleComic = mCfg.readEntry("scaleToContent_" + mId, false);
    mMaxStripNum = mCfg.readEntry("maxStripNum_" + mId, 0);
    mStored = mCfg.readEntry("storedPosition_" + mId, QString());
}

void ComicData::save()
{
    mCfg.writeEntry("scaleToContent_" + mId,  mScaleComic);
    mCfg.writeEntry("maxStripNum_" + mId, mMaxStripNum);
    mCfg.writeEntry("storedPosition_" + id(), mStored);

    // no next, thus the most recent strip
    if (!hasNext()) {
        mCfg.writeEntry("lastStripVisited_" + mId, true);
        mCfg.writeEntry("lastStrip_" + mId, mLast);
    }
}

void ComicData::setScaleComic(bool scale)
{
    mScaleComic = scale;
    save();
}

void ComicData::storePosition(bool store)
{
    mStored = (store ? mCurrent : QString());
    save();
}

void ComicData::setData(const Plasma::DataEngine::Data &data)
{
    const bool hasError = data[ "Error" ].toBool();
    if (!hasError) {
        mImage = data["Image"].value<QImage>();
        mPrev = data["Previous identifier suffix"].toString();
        mNext = data["Next identifier suffix"].toString();
        mAdditionalText = data["Additional text"].toString();
    }

    mWebsiteUrl = data[ "Website Url" ].value<KUrl>();
    mImageUrl = data["Image Url"].value<KUrl>();
    mShopUrl = data[ "Shop Url" ].value<KUrl>();
    mFirst = data[ "First strip identifier suffix" ].toString();
    mStripTitle = data[ "Strip title" ].toString();
    mAuthor = data[ "Comic Author" ].toString();
    mTitle = data[ "Title" ].toString();

    const QString suffixType = data[ "SuffixType" ].toString();
    if ( suffixType == "Date" ) {
        mType = Date;
    } else if ( suffixType == "Number" ) {
        mType = Number;
    } else {
        mType = String;
    }

    QString temp = data["Identifier"].toString();
    mCurrent = temp.remove(mId + ':');

    //found a new last identifier
    if (!hasNext()) {
        mLast = mCurrent;
    }

    mCurrentReadable = "";
    if ( mType == Number ) {
        mCurrentReadable = i18nc("an abbreviation for Number", "# %1", mCurrent);
        int tempNum = mCurrent.toInt();
        if ( mMaxStripNum < tempNum ) {
            mMaxStripNum = tempNum;
        }

        temp = mFirst.remove(mId + ':');
        mFirstStripNum = temp.toInt();
    } else if ( mType == Date && QDate::fromString( temp, "yyyy-MM-dd" ).isValid() ) {
        mCurrentReadable = mCurrent;
    } else if ( mType == String ) {
        mCurrentReadable = mCurrent;
    }

    mIsLeftToRight = data["isLeftToRight"].toBool();
    mIsTopToBottom = data["isTopToBottom"].toBool();

    save();
}

void ComicData::createErrorPicture(const Plasma::DataEngine::Data &data)
{
    QPixmap errorPic( 500, 400 );
    errorPic.fill();
    QPainter p( &errorPic );
    QFont font = Plasma::Theme::defaultTheme()->font( Plasma::Theme::DefaultFont );
    font.setPointSize( 24 );
    p.setPen( QColor( 0, 0, 0 ) );
    p.setFont( font );
    QString title = i18n( "Getting comic strip failed:" );
    p.drawText( QRect( 10, 10 , 480, 100 ), Qt::TextWordWrap | Qt::AlignHCenter | Qt::AlignVCenter, title );
    QString text = i18n( "Maybe there is no Internet connection.\nMaybe the comic plugin is broken.\nAnother reason might be that there is no comic for this day/number/string, so choosing a different one might work." );

    mPrev = data["Previous identifier suffix"].toString();
    if (hasPrev()) {
        if (!data["Identifier"].toString().isEmpty() ) {
            mErrorStrip = data["Identifier"].toString();
        }
        text.append( i18n( "\n\nChoose the previous strip to go to the last cached strip." ) );
    }

    font.setPointSize( 16 );
    p.setFont( font );
    p.drawText( QRect( 10, 120 , 480, 270 ), Qt::TextWordWrap | Qt::AlignLeft, text );

    mImage = errorPic.toImage();
    mAdditionalText = title + text;
}
