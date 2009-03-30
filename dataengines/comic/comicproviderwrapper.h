/*
 *   Copyright (C) 2008 Petri Damstén <damu@iki.fi>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#ifndef COMICPROVIDERWRAPPER_H
#define COMICPROVIDERWRAPPER_H

#include "comicprovider.h"

#include <QImage>
#include <QByteArray>

namespace Kross {
    class Action;
}
namespace Plasma {
    class Package;
}
class ComicProviderKross;

class ImageWrapper : public QObject
{
        Q_OBJECT
        Q_PROPERTY( QImage image READ image WRITE setImage )
        Q_PROPERTY( QByteArray rawData READ rawData WRITE setRawData )
    public:
        explicit ImageWrapper( QObject *parent = 0, const QImage &image = QImage() );

        QImage image() const;
        void setImage( const QImage &image );
        QByteArray rawData() const;
        void setRawData( const QByteArray &rawData );

    private:
        QImage mImage;
};

class DateWrapper : public QObject
{
        Q_OBJECT
        Q_PROPERTY( QDate date READ date WRITE setDate )
    public:
        explicit DateWrapper( QObject *parent = 0, const QDate &date = QDate() );

        QDate date() const;
        void setDate( const QDate &date );
        static QDate fromVariant( const QVariant &variant );

    public slots:
        QObject* addDays( int ndays );
        QObject* addMonths( int nmonths );
        QObject* addYears( int nyears );
        int day() const;
        int dayOfWeek() const;
        int dayOfYear() const;
        int daysInMonth() const;
        int daysInYear() const;
        int daysTo( const QVariant d ) const;
        bool isNull() const;
        bool isValid() const;
        int month() const;
        bool setDate( int year, int month, int day );
        int toJulianDay() const;
        QString toString( const QString &format ) const;
        QString toString( int format = 0 ) const;
        int weekNumber() const;
        int year() const;

    private:
        QDate mDate;
};

class StaticDateWrapper : public QObject
{
        Q_OBJECT
        Q_ENUMS( DateType )
    public:
        enum DateType {
            TextDate = Qt::TextDate,
            ISODate = Qt::ISODate,
            SystemLocaleShortDate = Qt::SystemLocaleShortDate,
            SystemLocaleLongDate = Qt::SystemLocaleLongDate,
            DefaultLocaleShortDate = Qt::DefaultLocaleShortDate,
            DefaultLocaleLongDate = Qt::DefaultLocaleLongDate
        };

        StaticDateWrapper( QObject *parent = 0 );

    public slots:
        QObject* currentDate();
        QObject* fromJulianDay( int jd );
        QObject* fromString( const QString & string, int format = Qt::TextDate );
        QObject* fromString( const QString & string, const QString & format );
        bool isLeapYear ( int year );
        bool isValid ( int year, int month, int day );
        QString longDayName ( int weekday );
        QString longMonthName ( int month );
        QString shortDayName ( int weekday );
        QString shortMonthName ( int month );
};

class ComicProviderWrapper : public QObject
{
        Q_OBJECT
        Q_ENUMS( IdentifierType )
        Q_ENUMS( RequestType )
        Q_ENUMS( PositionType )
        Q_PROPERTY( bool identifierSpecified READ identifierSpecified )
        Q_PROPERTY( QString textCodec READ textCodec WRITE setTextCodec )
        Q_PROPERTY( QString comicAuthor READ comicAuthor WRITE setComicAuthor )
        Q_PROPERTY( QString websiteUrl READ websiteUrl WRITE setWebsiteUrl )
        Q_PROPERTY( QString shopUrl READ shopUrl WRITE setShopUrl )
        Q_PROPERTY( QString title READ title WRITE setTitle )
        Q_PROPERTY( QString additionalText READ additionalText WRITE setAdditionalText )
        Q_PROPERTY( QVariant identifier READ identifier WRITE setIdentifier )
        Q_PROPERTY( QVariant nextIdentifier READ nextIdentifier WRITE setNextIdentifier )
        Q_PROPERTY( QVariant previousIdentifier READ previousIdentifier WRITE setPreviousIdentifier )
        Q_PROPERTY( QVariant firstIdentifier READ firstIdentifier WRITE setFirstIdentifier )
        Q_PROPERTY( QVariant lastIdentifier READ lastIdentifier WRITE setLastIdentifier )
        Q_PROPERTY( bool isLeftToRight READ isLeftToRight WRITE setLeftToRight )
        Q_PROPERTY( bool isTopToBottom READ isTopToBottom WRITE setTopToBottom )
    public:
        enum PositionType {
            Left = 0,
            Top,
            Right,
            Bottom
        };
        enum RequestType {
            Page = 0,
            Image,
            User
        };
        enum IdentifierType {
            DateIdentifier   = ComicProvider::DateIdentifier,
            NumberIdentifier = ComicProvider::NumberIdentifier,
            StringIdentifier = ComicProvider::StringIdentifier
        };

        ComicProviderWrapper( ComicProviderKross *parent );
        ~ComicProviderWrapper();

        ComicProvider::IdentifierType identifierType() const;
        QImage comicImage();
        void pageRetrieved( int id, const QByteArray &data );
        void pageError( int id, const QString &message );

        bool identifierSpecified() const;
        QString textCodec() const;
        void setTextCodec( const QString &textCodec );
        QString comicAuthor() const;
        void setComicAuthor( const QString &author );
        QString websiteUrl() const;
        void setWebsiteUrl( const QString &websiteUrl );
        QString shopUrl() const;
        void setShopUrl( const QString &shopUrl );
        QString title() const;
        void setTitle( const QString &title );
        QString additionalText() const;
        void setAdditionalText( const QString &additionalText );
        QVariant identifier();
        void setIdentifier( const QVariant &identifier );
        QVariant nextIdentifier();
        void setNextIdentifier( const QVariant &nextIdentifier );
        QVariant previousIdentifier();
        void setPreviousIdentifier( const QVariant &previousIdentifier );
        QVariant firstIdentifier();
        void setFirstIdentifier( const QVariant &firstIdentifier );
        QVariant lastIdentifier();
        void setLastIdentifier( const QVariant &lastIdentifier );
        bool isLeftToRight() const;
        void setLeftToRight( bool ltr );
        bool isTopToBottom() const;
        void setTopToBottom( bool ttb );

        QVariant identifierVariant() const;
        QVariant firstIdentifierVariant() const;
        QVariant lastIdentifierVariant() const;
        QVariant nextIdentifierVariant() const;
        QVariant previousIdentifierVariant() const;

    public slots:
        void finished() const;
        void error() const;

        void requestPage( const QString &url, int id, const QVariantMap &infos = QVariantMap() );
        void combine( const QVariant &image, PositionType position = Top );
        QObject* image();

        void init();

    protected:
        QVariant callFunction( const QString &name, const QVariantList &args = QVariantList() );
        const QStringList& extensions() const;
        bool functionCalled() const;
        QVariant identifierToScript( const QVariant &identifier );
        QVariant identifierFromScript( const QVariant &identifier ) const;
        void setIdentifierToDefault();
        void checkIdentifier( QVariant *identifier );

    private:
        Kross::Action *mAction;
        ComicProviderKross *mProvider;
        QStringList mFunctions;
        bool mFuncFound;
        ImageWrapper *mKrossImage;
        static QStringList mExtensions;
        Plasma::Package *mPackage;

        QByteArray mTextCodec;
        QString mWebsiteUrl;
        QString mShopUrl;
        QString mTitle;
        QString mAdditionalText;
        QVariant mIdentifier;
        QVariant mNextIdentifier;
        QVariant mPreviousIdentifier;
        QVariant mFirstIdentifier;
        QVariant mLastIdentifier;
        int mRequests;
        bool mIdentifierSpecified;
        bool mIsLeftToRight;
        bool mIsTopToBottom;
};

#endif
