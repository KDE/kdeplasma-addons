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

#include <KUrl>
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
        ImageWrapper( QObject *parent = 0, const QImage &image = QImage() );

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
        DateWrapper( QObject *parent = 0, const QDate &date = QDate() );

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
    public:
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
        Q_ENUMS( DateType )
        Q_PROPERTY( QVariant firstStripDate READ firstStripDate WRITE setFirstStripDate )
        Q_PROPERTY( int firstStripNumber READ firstStripNumber WRITE setFirstStripNumber )
        Q_PROPERTY( QString comicAuthor READ comicAuthor WRITE setComicAuthor )
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
        enum DateType {
            TextDate = Qt::TextDate,
            ISODate = Qt::ISODate,
            SystemLocaleShortDate = Qt::SystemLocaleShortDate,
            SystemLocaleLongDate = Qt::SystemLocaleLongDate,
            DefaultLocaleShortDate = Qt::DefaultLocaleShortDate,
            DefaultLocaleLongDate = Qt::DefaultLocaleLongDate
        };

        ComicProviderWrapper( ComicProviderKross *parent );
        ~ComicProviderWrapper();

        ComicProvider::IdentifierType identifierType();
        KUrl websiteUrl();
        QImage image();
        QString identifier();
        QString nextIdentifier();
        QString previousIdentifier();
        QString firstStripIdentifier();
        QString stripTitle();
        QString additionalText();
        void pageRetrieved( int id, const QByteArray &data );
        void pageError( int id, const QString &message );

        QVariant firstStripDate();
        void setFirstStripDate( const QVariant &date );
        int firstStripNumber() const;
        void setFirstStripNumber( int number );
        QString comicAuthor() const;
        void setComicAuthor( const QString &author );
        void setUseDefaultImageHandler( bool useDefaultImageHandler );
        bool useDefaultImageHandler() const;

        bool functionCalled() const;

    public slots:
        void finished() const;
        void error() const;

        QObject* requestedDate();
        int requestedNumber() const;
        QString requestedString() const;
        void requestPage( const QString &url, int id, const QVariantMap &infos = QVariantMap() );
        void addHeader( const QString &name, PositionType position = Top );

        void init();

    protected:
        QVariant callFunction( const QString &name, const QVariantList &args = QVariantList() );
        const QStringList& extensions() const;

    private:
        Kross::Action *mAction;
        ComicProviderKross *mProvider;
        QStringList mFunctions;
        bool mFuncFound;
        ImageWrapper mKrossImage;
        static QStringList mExtensions;
        Plasma::Package *mPackage;
};

#endif
