/*
 *   Copyright (C) 2007 Trever Fischer <wm161@wm161.net>
 *   Copyright (C) 2007 André Duffeck <duffeck@kde.org>
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

#include "twitterengine.h"

#include <QDateTime>
#include <QDomDocument>
#include <QDomNode>
#include <QDomNodeList>
#include <QHttp>
#include <QTimer>
#include <QImage>
#include <QPixmap>

#include <KDebug>
#include <KUrl>
#include <ksocketfactory.h>
#include <KCodecs>
#include <KUrl>

TwitterEngine::TwitterEngine(QObject* parent, const QVariantList& args)
    : Plasma::DataEngine(parent)
{
    Q_UNUSED(args);

    m_http = new QHttp("twitter.com");
    connect(m_http,SIGNAL(requestFinished(int,bool)), this, SLOT(requestFinished(int,bool)));
    m_unauthorizedHttp = new QHttp("twitter.com");
    connect(m_unauthorizedHttp,SIGNAL(requestFinished(int,bool)), this, SLOT(unauthorizedRequestFinished(int,bool)));

    setMinimumUpdateInterval(10 * 1000);
    setUpdateInterval(5 * 60 * 1000);
}

TwitterEngine::~TwitterEngine()
{
}

void TwitterEngine::setPassword(const QString &password)
{
    m_password = password;
    m_http->setUser(m_username,m_password);
}

void TwitterEngine::setUsername(const QString &username)
{
    m_username = username;
    m_http->setUser(m_username,m_password);
}

void TwitterEngine::setInterval(const QString &interval)
{
    setUpdateInterval( interval.toInt() );
}

void TwitterEngine::setStatus(const QString &status)
{
    kDebug();
    m_status = status;

    m_socket = KSocketFactory::connectToHost( "http", "twitter.com", 80 );
    connect( m_socket, SIGNAL(connected()), SLOT(slotConnected()) );
    connect( m_socket, SIGNAL(readyRead()), SLOT(slotRead()) );  
}

QString TwitterEngine::username() const
{
    return m_username;
}

QString TwitterEngine::password() const
{
    return m_password;
}

QString TwitterEngine::interval() const
{
    return m_interval;
}

QString TwitterEngine::status() const
{
    return m_status;
}

/*QStringList TwitterEngine::sources() const
{
    return QStringList() << "Timeline";
}*/

bool TwitterEngine::sourceRequested(const QString &name)
{
    kDebug() << name;
    m_activeSources.append( name );
    updateSource(name);
    return true;
}

void TwitterEngine::requestFinished(int id, bool error)
{
    kDebug() << id << error << m_http->lastResponse().statusCode();
    if( error ) {
        kDebug() << "An error occured: " << m_http->errorString();
        return;
    } else if( m_http->lastResponse().statusCode() == 401 ) {
        // Resend request, altough i'd guess that QHttps's job...
        foreach( QString source, m_activeSources ) {
            if( source.startsWith( "Timeline" ) )
               updateSource( source );
        }
        return;
    }
    UpdateType type = m_updates.take(id);
    QByteArray data = m_http->readAll();
    QDomDocument xml;
    xml.setContent(data);
    if (type==Timeline) {
        kDebug() << "Timeline";
        setData("Timeline",QVariant(parseStatuses(xml.elementsByTagName("status"))));
    } else if (type==UserTimeline) {
        QString user = m_timelines.value(id);
        kDebug() << QString("Timeline:%1").arg(user);
        setData(QString("Timeline:%1").arg(user),parseStatuses(xml.elementsByTagName("status")));
    } else if (type==UserTimelineWithFriends) {
        QString user = m_timelines.value(id);
        kDebug() << QString("TimelineWithFriends:%1").arg(user);
        setData(QString("TimelineWithFriends:%1").arg(user),parseStatuses(xml.elementsByTagName("status")));
    } else if (type==Status) {
        kDebug() << "Status";
        parseStatuses(xml.elementsByTagName("status"));
    } else if (type==UserImage) {
        kDebug() << "UserImage:" << m_timelines.value(id);
        QImage img;
        img.loadFromData( data );
        QPixmap pm = QPixmap::fromImage( img ).scaled( 48, 48 );
        QString user = m_timelines.value(id);
        setData(QString("UserInfo:%1").arg(user), "Image", pm );
    }
}

void TwitterEngine::unauthorizedRequestFinished(int id, bool error)
{
    kDebug() << id << error;
    if( error ) {
        kDebug() << "An error occured: " << m_unauthorizedHttp->errorString();
        return;
    }
    UpdateType type = m_unauthorizedUpdates.take(id);
    QByteArray data = m_unauthorizedHttp->readAll();
    QDomDocument xml;
    xml.setContent(data);

    if (type==UserImage) {
        kDebug() << "UserImage:" << m_timelines.value(id);
        QImage img;
        img.loadFromData( data );
        QPixmap pm = QPixmap::fromImage( img ).scaled( 48, 48 );
        QString user = m_timelines.value(id);
        setData(QString("UserInfo:%1").arg(user), "Image", pm );
    }
}


bool TwitterEngine::updateSource(const QString &source)
{
    if (source=="Timeline") {
        updateTimeline();
    }
    QStringList tokens = source.split(':');
    if (tokens.at(0)=="Update") {
        if( !sourceDict()[source] )
            getTweet(tokens.at(1).toInt());
    }
    if (tokens.at(0)=="Timeline") {
        updateUser(tokens.at(1));
    }
    if (tokens.at(0)=="TimelineWithFriends") {
        updateUserWithFriends(tokens.at(1));
    }
    return false;
}

void TwitterEngine::updateTimeline()
{
    kDebug() ;
    m_http->setHost( "twitter.com" );
    m_updates.insert(m_http->get("/statuses/public_timeline.xml"),Timeline);
}

void TwitterEngine::updateUser(const QString &who)
{
    kDebug() ;
    m_http->setHost( "twitter.com" );
    int id=m_http->get(QString("/statuses/user_timeline/%1.xml").arg(who));
    m_updates.insert(id,UserTimeline);
    m_timelines.insert(id,who);
}

void TwitterEngine::updateUserWithFriends(const QString &who)
{
    kDebug() ;
    m_http->setHost( "twitter.com" );
    int id=m_http->get(QString("/statuses/friends_timeline.xml"));
    m_updates.insert(id,UserTimelineWithFriends);
    m_timelines.insert(id,who);
}

void TwitterEngine::getTweet(const int &ID)
{
    kDebug() ;
    m_http->setHost( "twitter.com" );
    m_updates.insert(m_http->get(QString("/statuses/show/%1.xml").arg(ID)),Status);
}

void TwitterEngine::getUserImage( const QString &who, const KUrl &url )
{
    kDebug() << who << " " << url.url();
    m_unauthorizedHttp->setHost( url.host() );
    int id = m_unauthorizedHttp->get( url.path() );
    m_unauthorizedUpdates.insert( id, UserImage );
    m_timelines.insert( id, who );
}

QList<QVariant> TwitterEngine::parseStatuses(QDomNodeList updates)
{
    kDebug() ;
    QList<QVariant> timeline;
    for (uint i=0;i<updates.length();i++) {
        QDomNode n = updates.at( i );

        QString text = n.firstChildElement( "text" ).text();
        QString user = n.firstChildElement( "user" ).firstChildElement( "screen_name" ).text();
        QString imageUrl = n.firstChildElement( "user" ).firstChildElement( "profile_image_url" ).text();
        QString url = n.firstChildElement( "user" ).firstChildElement( "url" ).text();
        QString source = n.firstChildElement( "source" ).text();
        uint id = n.firstChildElement( "id" ).text().toUInt();

        QString created = n.firstChildElement( "created_at" ).text();
        created = created.right( created.length() - 4 );
        created.replace( QRegExp( "[+]\\d\\d\\d\\d" ), "" );
        QDateTime time = QDateTime::fromString( created, "MMM dd hh:mm:ss  yyyy" );
        time.setTimeSpec( Qt::UTC );

        QString updateEntry = QString("Update:%1").arg(id);
        setData(updateEntry,"ID",QVariant(id));
        setData(updateEntry,"Date",QVariant(time));
        setData(updateEntry,"Status",QVariant(text));
        setData(updateEntry,"User",QVariant(user));
        setData(updateEntry,"Source",QVariant(source));
        setData(updateEntry,"ImageUrl",QVariant(imageUrl));
        setData(updateEntry,"url",QVariant(url));

        KUrl imgKurl( imageUrl );
        if( !m_userImages.contains( user ) || 
            m_userImages[user] != imgKurl ) {
            m_userImages[user] = imgKurl;
            getUserImage( user, imgKurl );
        }

        timeline.append(id);
    }
    return timeline;
}

void TwitterEngine::slotConnected()
{
    kDebug() ;
    QString auth = QString( "%1:%2" ).arg( m_username, m_password );
    auth = QString( "Basic " ) + KCodecs::base64Encode( auth.toAscii() );
    QString data;
    QString status = QString( "source=kdetwitter&status=%1" ).arg( m_status );
    data = QString("POST /statuses/update.xml HTTP/1.1\r\n"
        "Authorization: %1\r\n"
        "User-Agent: Mozilla/5.0\r\n"
        "Host: twitter.com\r\n"
        "Accept: */*\r\n"
        "Content-Length: %2\r\n"
        "Content-Type: application/x-www-form-urlencoded\r\n\r\n"
        "%3" )
        .arg( auth, QString::number(status.toUtf8().length()), status );

    m_header = QHttpResponseHeader();
    m_data.truncate( 0 );
    m_socket->write( data.toUtf8(), data.toUtf8().length() );
}

void TwitterEngine::slotRead()
{
    kDebug() ;
    QString read = m_socket->readAll();

    QString data;
    if( !m_header.isValid() ) {
        m_header = read.section( "\r\n\r\n", 0, 0 );
        m_data = read.section( "\r\n\r\n", 1, 1 );
    } else {
        m_data.append( read );
    }

    if( m_header.statusCode() == 401 ) {
        kDebug() << "Status upload succeeded.";
        return;
    }

    kDebug() << "Status upload succeeded.";
    foreach( QString source, m_activeSources ) {
        if( source.startsWith( "Timeline" ) )
            updateSource( source );
    }
}


#include "twitterengine.moc"
