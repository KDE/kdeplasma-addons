/*
 *   Copyright (C) 2007 Trever Fischer <wm161@wm161.net>
 *   Copyright (C) 2007 André Duffeck <duffeck@kde.org>
 *   Copyright (C) 2007 Chani Armitage <chanika@gmail.com>
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

Q_DECLARE_METATYPE(Plasma::DataEngine::Data)

TwitterEngine::TwitterEngine(QObject* parent, const QVariantList& args)
    : Plasma::DataEngine(parent)
{
    Q_UNUSED(args);

    m_http = new QHttp("twitter.com");
    connect(m_http,SIGNAL(requestFinished(int,bool)), this, SLOT(requestFinished(int,bool)));
    m_anonHttp = new QHttp();
    connect(m_anonHttp,SIGNAL(requestFinished(int,bool)), this, SLOT(anonRequestFinished(int,bool)));

    setMinimumUpdateInterval(2 * 1000);
}

TwitterEngine::~TwitterEngine()
{
}

void TwitterEngine::setStatus(const QString &status)
{
    kDebug();
    int colon = status.indexOf(':');
    if (colon < 1) {
        return; // failed to get a name
    }
    QString user = status.left(colon);
    m_status = QUrl::toPercentEncoding(status.right(status.length() - (colon + 1)));
    QString statusurl = QString("source=kdetwitter&status=%1").arg(m_status);
    m_http->setUser(user, m_config.value(user).toString());
    int id = m_http->post("/statuses/update.xml", statusurl.toUtf8());
    m_pendingRequests.insert(id, Post);
}

QString TwitterEngine::status() const
{
    return m_status;
}

void TwitterEngine::setConfig(const Plasma::DataEngine::Data &config)
{
    m_config = config;
}

Plasma::DataEngine::Data TwitterEngine::config() const
{
    return m_config;
}

/*QStringList TwitterEngine::sources() const
{
    return QStringList() << "Timeline";
}*/

bool TwitterEngine::sourceRequested(const QString &name)
{
    kDebug() << name;
    setData(name, DataEngine::Data()); //need to have something because we're async.
    updateSource(name); //start a download
    return true; //TODO what if they requested a nonsense source like "foobar"?
}

//everything but image?
void TwitterEngine::requestFinished(int id, bool error)
{
    kDebug() << id;
    //we *always* want to remove these
    UpdateType type = m_pendingRequests.take(id);
    QString user = m_pendingNames.take(id);
    if (type == 0) { //we never inserted it, so we ignore it
        return;
    }
    if( error ) {
        kDebug() << "An error occured: " << m_http->errorString();
        //oh bugger, all our pending requests just went poof. FIXME
        //need to either dump them or reschedule them
        //problem: icons may never get downloaded if this interrupts them
        return;
    } else if( m_http->lastResponse().statusCode() == 401 ) {
        kDebug() << "Unauthorized";
        //TODO: tell the applet
        return;
    } else if (m_http->lastResponse().statusCode() != 200) {
        kDebug() << "not ok!" << m_http->lastResponse().statusCode(); //um, so, what do we do now? FIXME
    }
    QByteArray data = m_http->readAll();
    QDomDocument xml;
    xml.setContent(data);

    switch (type) {
    case Timeline: //TODO move to anon
        kDebug() << "Timeline";
        parseStatuses(xml.elementsByTagName("status"), "Timeline");
        break;
    case UserTimeline:
        kDebug() << QString("Timeline:%1").arg(user);
        parseStatuses(xml.elementsByTagName("status"), QString("Timeline:%1").arg(user));
        break;
    case UserTimelineWithFriends:
        kDebug() << QString("TimelineWithFriends:%1").arg(user);
        parseStatuses(xml.elementsByTagName("status"), QString("TimelineWithFriends:%1").arg(user));
        break;
    case Post:
        kDebug() << "Status upload succeeded.";
        //the data is a copy of the status update. could be useful someday.
        //update every bloody timeline we've got
        foreach( QString source, sources() ) {
            if( source.startsWith( "Timeline" ) )
                updateSource( source );
        }
        break;
    default:
        kDebug() << "unexpected type" << type;
    }
}

//only used for images so far
void TwitterEngine::anonRequestFinished(int id, bool error)
{
    kDebug() << id;
    //we *always* want to remove these
    UpdateType type = m_pendingAnonRequests.take(id);
    QString user = m_pendingNames.take(id);
    if (type == 0) { //we never inserted it, so we ignore it
        return;
    }
    if( error ) {
        kDebug() << "An error occured: " << m_anonHttp->errorString();
        return;
    } else if (m_anonHttp->lastResponse().statusCode() != 200) {
        kDebug() << "not ok!" << m_anonHttp->lastResponse().statusCode(); //um, so, what do we do now? FIXME
    }
    QByteArray data = m_anonHttp->readAll();

    if (type==UserImage) {
        kDebug() << "UserImage:" << user;
        QImage img;
        img.loadFromData( data );
        QPixmap pm = QPixmap::fromImage( img ).scaled( 48, 48 ); //FIXME do we really want to do this?
        setData("UserImages", user, pm); //FIXME clear out old images someday
        clearData("LatestImage");
        setData("LatestImage", user, pm);
        //FIXME user's own image needs an explicit request somehow. is there a userinfo xml?
    }
}

//called when it's time to update a source
//also called by twitter from sourceRequested
//and when it thinks an update would be useful
//always returns false becaues everything is async
bool TwitterEngine::updateSource(const QString &source)
{
    kDebug() << source;
    if (source=="Timeline") {
        updateTimeline();
    }
    QStringList tokens = source.split(':');
    if (tokens.at(0)=="Timeline") {
        updateUser(tokens.at(1));
    } else if (tokens.at(0)=="TimelineWithFriends") {
        updateUserWithFriends(tokens.at(1));
    }
    return false;
}

//this is *the* twitter everyone-timeline
//currently unused
//with this we would need to be sure we cleaned out old icons
//source Timeline
//TODO we never need auth for this
void TwitterEngine::updateTimeline()
{
    m_pendingRequests.insert(m_http->get("/statuses/public_timeline.xml"),Timeline);
}

//source Timeline:user
//the tweets of one single user.
//we don't always *need* auth for this
//in fact, how can we be sure of whose auth we should use?
//TODO default to same-user, offer option for different/no auth
void TwitterEngine::updateUser(const QString &who)
{
    m_http->setUser(who, m_config.value(who).toString());
    int id=m_http->get(QString("/statuses/user_timeline/%1.xml").arg(who));
    m_pendingRequests.insert(id,UserTimeline);
    m_pendingNames.insert(id,who);
}

//source TimelineWithFriends:user
//this is the normal one
//http auth is important here
void TwitterEngine::updateUserWithFriends(const QString &who)
{
    m_http->setUser(who, m_config.value(who).toString());
    int id=m_http->get(QString("/statuses/friends_timeline.xml"));
    m_pendingRequests.insert(id,UserTimelineWithFriends);
    m_pendingNames.insert(id,who);
}

void TwitterEngine::getUserImage( const QString &who, const KUrl &url )
{
    kDebug() << who << "has image" << url.url();
    m_anonHttp->setHost( url.host() ); //it's not twitter.com
    int id = m_anonHttp->get( url.path() );
    m_pendingAnonRequests.insert( id, UserImage );
    m_pendingNames.insert( id, who ); //FIXME is it safe to share with the other http object?
}

//parses the returned xml for a timeline
//sets the data for the source
//and fetches new images if needed
void TwitterEngine::parseStatuses(QDomNodeList updates, const QString& source)
{
    kDebug() << source;
    clearData(source); //get rid of the old ones
    for (uint i=0;i<updates.length();i++) {
        QDomNode n = updates.at( i );

        //extract useful data
        QString text = n.firstChildElement( "text" ).text();
        QDomNode usernode = n.firstChildElement( "user" );
        QString user = usernode.firstChildElement( "screen_name" ).text();
        QString imageUrl = usernode.firstChildElement( "profile_image_url" ).text();
        QString url = usernode.firstChildElement( "url" ).text();
        QString tsource = n.firstChildElement( "source" ).text();
        QString id = n.firstChildElement( "id" ).text();
        //there's lots more data in there, but we have enough for now

        //get the timestamp in a useful form
        QString created = n.firstChildElement( "created_at" ).text();
        created = created.right( created.length() - 4 );
        created.replace( QRegExp( "[+]\\d\\d\\d\\d" ), "" );
        QDateTime time = QDateTime::fromString( created, "MMM dd hh:mm:ss  yyyy" );
        time.setTimeSpec( Qt::UTC );

        //bundle up each tweet
        DataEngine::Data tweet;
        tweet["Date"]=QVariant(time);
        tweet["Status"]=QVariant(text);
        tweet["User"]=QVariant(user);
        tweet["Source"]=QVariant(tsource);
        tweet["url"]=QVariant(url);
        QVariant v;
        v.setValue(tweet);
        setData(source, id, v);

        //update the image if necessary
        //TODO check whether anyone cares, first
        KUrl imgKurl( imageUrl );
        if( !m_userImages.contains( user ) || 
            m_userImages[user] != imgKurl ) {
            m_userImages[user] = imgKurl; //FIXME if the download fails it'll never retry
            getUserImage( user, imgKurl );
        }
    }
}

#include "twitterengine.moc"
