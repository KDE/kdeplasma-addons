/*
 *   Copyright (C) 2007 Tobias Koenig <tokoe@kde.org>
 *   Copyright  2008 by Anne-Marie Mahfouf <annma@kde.org>
 *   Copyright  2008 by Georges Toth <gtoth@trypill.org>
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

#include "flickrprovider.h"

#include <QtCore/QDate>
#include <QtCore/QRegExp>
#include <QtGui/QImage>
#include <QXmlStreamReader>

#include <KDebug>
#include <kio/job.h>

POTDPROVIDER_EXPORT_PLUGIN( FlickrProvider, "FlickrProvider", "" )

class FlickrProvider::Private
{
  public:
    Private( FlickrProvider *parent )
      : mParent( parent )
    {
	qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
    }

    void pageRequestFinished( KJob* );
    void imageRequestFinished( KJob* );
    void parsePage();

    FlickrProvider *mParent;
    QByteArray mPage;
    QDate mDate;
    QImage mImage;

    QXmlStreamReader xml;

  private:
    QStringList m_photoList;
};

void FlickrProvider::Private::pageRequestFinished( KJob *_job )
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>( _job );
    if ( job->error() ) {
	emit mParent->error( mParent );
	kDebug() << "pageRequestFinished error";
	return;
    }

    const QString data = QString::fromUtf8( job->data() );

    // Clear the list
    m_photoList.clear();

    xml.clear();
    xml.addData(data);

    while ( !xml.atEnd() )
    {
	xml.readNext();

	if ( xml.isStartElement() )
	{
	    if ( xml.name() == "rsp" )
	    {
		/* no pictures available for the specified parameters */
		if (xml.attributes().value ( "stat" ).toString() == "fail")
		{
		    /* To be sure, decrement the date to two days earlier... @TODO */
		    QDate tempDate = mDate.addDays(-2);

                    KUrl url( "http://api.flickr.com/services/rest/?api_key=a902f4e74cf1e7bce231742d8ffb46b4&method=flickr.interestingness.getList&date=" + tempDate.toString( Qt::ISODate) );
                    KIO::StoredTransferJob *imageJob = KIO::storedGet( url );
                    mParent->connect( imageJob, SIGNAL( finished( KJob* ) ), SLOT( pageRequestFinished( KJob* ) ) );
		    return;
		}
	    }
	    else if ( xml.name() == "photo" )
	    {
	    	if (xml.attributes().value ( "ispublic" ).toString() != "1")
		    continue;

	    	QString fileUrl = QString("http://farm" + xml.attributes().value ( "farm" ).toString() + ".static.flickr.com/"
		    + xml.attributes().value ( "server" ).toString() + '/' + xml.attributes().value ( "id" ).toString()
		    + '_' + xml.attributes().value ( "secret" ).toString() + ".jpg");

		m_photoList.append(fileUrl);
	    }
	}
    }

    if ( xml.error() && xml.error() != QXmlStreamReader::PrematureEndOfDocumentError )
    {
	qWarning() << "XML ERROR:" << xml.lineNumber() << ": " << xml.errorString();
    }

    if (m_photoList.begin() != m_photoList.end())
    {
	KUrl url( m_photoList.at(qrand() % m_photoList.size()) );
        KIO::StoredTransferJob *imageJob = KIO::storedGet( url );
        mParent->connect( imageJob, SIGNAL( finished( KJob* ) ), SLOT( imageRequestFinished( KJob* ) ) );
    }else{
	kDebug() << "empty list";
    }
}

void FlickrProvider::Private::imageRequestFinished( KJob *_job )
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>( _job );
    if ( job->error() ) {
	emit mParent->error( mParent );
	return;
    }

    mImage = QImage::fromData( job->data() );
    emit mParent->finished( mParent );
}

FlickrProvider::FlickrProvider( QObject *parent, const QVariantList &args )
    : PotdProvider( parent, args ), d( new Private( this ) )
{
    const QString type = args[ 0 ].toString();
    if ( type == "Date" )
        d->mDate = args[ 1 ].toDate();
    else
	Q_ASSERT( false && "Invalid type passed to potd provider" );

    KUrl url("http://api.flickr.com/services/rest/?api_key=a902f4e74cf1e7bce231742d8ffb46b4&method=flickr.interestingness.getList&date=" + d->mDate.toString( Qt::ISODate ) );
    KIO::StoredTransferJob *job = KIO::storedGet( url );
    connect( job, SIGNAL( finished( KJob* ) ), SLOT( pageRequestFinished( KJob* ) ) );
}

FlickrProvider::~FlickrProvider()
{
    delete d;
}

QImage FlickrProvider::image() const
{
    return d->mImage;
}

QString FlickrProvider::identifier() const
{
    return QString( "flickr:%1" ).arg( d->mDate.toString( Qt::ISODate ));
}

#include "flickrprovider.moc"
