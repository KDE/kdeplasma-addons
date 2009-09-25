/*
    This file is part of KDE.

    Copyright (c) 2008 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#include "provider.h"

#include <KDebug>
#include <KUrl>

#include "activity.h"
#include "content.h"
#include "contentjob.h"
#include "eventjob.h"
#include "folder.h"
#include "knowledgebaseentryjob.h"
#include "knowledgebaseentrylistjob.h"
#include "message.h"
#include "personjob.h"
#include "postjob.h"
#include "providerinitjob.h"
#include "listjob.h"

using namespace Attica;


class Provider::Private : public QSharedData {
  public:
    KUrl m_baseUrl;
    QString m_id;
    QString m_name;
    Private(const Private& other)
      : QSharedData(other), m_baseUrl(other.m_baseUrl), m_id(other.m_id), m_name(other.m_name)
    {
    }
    Private(const QString& id, const KUrl& baseUrl, const QString name)
      : m_baseUrl(baseUrl), m_id(id), m_name(name)
    {
    }
};


ProviderInitJob* Provider::byId(const QString& id)
{
  ProviderInitJob* job = new ProviderInitJob(id);
  job->start();
  return job;
}


Provider::Provider()
  : d(new Private(QString(), KUrl(), QString()))
{
}

Provider::Provider(const Provider& other)
  : d(other.d)
{
}

Provider::Provider(const QString& id, const KUrl& baseUrl, const QString& name)
  : d(new Private(id, baseUrl, name))
{
}

Provider& Provider::operator=(const Attica::Provider & other)
{
    d = other.d;
    return *this;
}

Provider::~Provider()
{
}


QString Provider::id() const
{
    return d->m_id;
}


QString Provider::name() const
{
    return d->m_name;
}


PersonJob* Provider::requestPerson(const QString& id)
{
  KUrl url = createUrl( "person/data/" + id );
  return doRequestPerson( url );
}

PersonJob* Provider::requestPersonSelf()
{
  KUrl url = createUrl( "person/self" );
  return doRequestPerson( url );
}

ListJob<Person>* Provider::requestPersonSearchByName(const QString& name)
{
  KUrl url = createUrl( "person/data");
  url.addQueryItem("name", name);
  return doRequestPersonList( url );
}

ListJob<Person>* Provider::requestPersonSearchByLocation(qreal latitude, qreal longitude, qreal distance, int page, int pageSize)
{
  KUrl url = createUrl( "person/data" );
  url.addQueryItem("latitude", QString::number(latitude));
  url.addQueryItem("longitude", QString::number(longitude));
  url.addQueryItem("distance", QString::number(distance));
  url.addQueryItem("page", QString::number(page));
  url.addQueryItem("pagesize", QString::number(pageSize));
  
  qDebug() << "Location-based search:" << latitude << longitude << distance;
  qDebug() << "URL:" << url;
  return doRequestPersonList( url );
}

ListJob<Person>* Provider::requestFriends(const QString& id, int page, int pageSize)
{
  KUrl url = createUrl( "friend/data/" + id );
  url.addQueryItem("page", QString::number(page));
  url.addQueryItem("pagesize", QString::number(pageSize));
  kDebug() << "URL:" << url;
  return doRequestPersonList( url );
}

ListJob<Activity>* Provider::requestActivities()
{
  KUrl url = createUrl( "activity" );
  return doRequestActivityList( url );
}

PostJob* Provider::postActivity(const QString& message)
{
  PostJob *job = new PostJob();

  KUrl url = createUrl( "activity" );
  job->setUrl( url );
  job->setData( "message", message );
  
  job->start();
  return job;
}

PostJob* Provider::postFriendInvitation(const QString& to, const QString& message)
{
  PostJob *job = new PostJob();

  KUrl url = createUrl( "friend/outbox/" + to );
  job->setUrl( url );
  job->setData( "message", message );

  job->start();
  return job;  
}

PostJob* Provider::postLocation(qreal latitude, qreal longitude, const QString& city, const QString& country)
{
  PostJob *job = new PostJob();
  
  KUrl url = createUrl( "person/self" );
  
  job->setUrl( url ); 

  job->setData( "latitude", QString("%1").arg(latitude) );
  job->setData( "longitude", QString("%1").arg(longitude) );
  job->setData( "city", city );
  job->setData( "country", country );
  
  job->start();
  return job;
}


ListJob<Folder>* Provider::requestFolders()
{
  return doRequestFolderList( createUrl( "message" ) );
}

ListJob<Message>* Provider::requestMessages(const Folder& folder)
{
  return doRequestMessageList( createUrl( "message/" + folder.id() ) );
}

PostJob* Provider::postMessage( const Message &message )
{
  PostJob *job = new PostJob();
  
  KUrl url = createUrl( "message/2" );
  job->setUrl( url );
  job->setData( "message", message.body() );
  job->setData( "subject", message.subject() );
  job->setData( "to", message.to() );
  
  job->start();
  return job;
}

ListJob<Category>* Provider::requestCategories()
{
  ListJob<Category> *job = new ListJob<Category>();
  
  KUrl url = createUrl( "content/categories" );
  job->setUrl( url );
  
  job->start();
  return job;
}

ListJob<Content>* Provider::searchContents(const Category::List& categories, const QString& search, SortMode sortMode)
{
  ListJob<Content> *job = new ListJob<Content>();
  
  KUrl url = createUrl( "content/data" );

  QStringList categoryIds;
  foreach( const Category &category, categories ) {
    categoryIds.append( category.id() );
  }
  url.addQueryItem( "categories", categoryIds.join( "x" ) );
  
  url.addQueryItem( "search", search );
  QString sortModeString;
  switch ( sortMode ) {
    case Newest:
      sortModeString = "new";
      break;
    case Alphabetical:
      sortModeString = "alpha";
      break;
    case Rating:
      sortModeString = "high";
      break;
    case Downloads:
      sortModeString = "down";
      break;
  }
  if ( !sortModeString.isEmpty() ) {
    url.addQueryItem( "sortmode", sortModeString );
  }

  job->setUrl( url );
  
  job->start();
  return job;
}

ContentJob* Provider::requestContent(const QString& id)
{
  ContentJob *job = new ContentJob();
  
  KUrl url = createUrl( "content/data/" + id );
  job->setUrl( url );
  
  job->start();
  return job;
}

KnowledgeBaseEntryJob* Provider::requestKnowledgeBaseEntry(const QString& id)
{
  KnowledgeBaseEntryJob *job = new KnowledgeBaseEntryJob();

  KUrl url = createUrl( "knowledgebase/data/" + id );
  job->setUrl( url );

  job->start();
  return job;
}

KnowledgeBaseListJob* Provider::searchKnowledgeBase(const Content& content, const QString& search, Provider::SortMode sortMode, int page, int pageSize)
{
  KnowledgeBaseListJob *job = new KnowledgeBaseListJob();

  KUrl url = createUrl( "knowledgebase/data" );

  if (content.isValid()) {
      url.addQueryItem("content", content.id());
  }

  url.addQueryItem( "search", search );
  QString sortModeString;
  switch ( sortMode ) {
    case Newest:
      sortModeString = "new";
      break;
    case Alphabetical:
      sortModeString = "alpha";
      break;
    case Rating:
      sortModeString = "high";
      break;
    //FIXME: knowledge base doesn't have downloads
    case Downloads:
      sortModeString = "new";
      break;
  }
  if ( !sortModeString.isEmpty() ) {
    url.addQueryItem( "sortmode", sortModeString );
  }

  url.addQueryItem( "page", QString::number(page) );
  url.addQueryItem( "pagesize", QString::number(pageSize) );

  job->setUrl( url );

  job->start();
  return job;
}

EventJob* Provider::requestEvent(const QString& id)
{
  EventJob* job = new EventJob();

  job->setUrl(createUrl("event/data/" + id));

  job->start();
  return job;
}

ListJob<Event>* Provider::requestEvent(const QString& country, const QString& search, const QDate& startAt, Provider::SortMode mode, int page, int pageSize)
{
  ListJob<Event>* job = new ListJob<Event>();

  KUrl url = createUrl("event/data");

  if (!search.isEmpty()) {
      url.addQueryItem("search", search);
  }

  QString sortModeString;
  switch (mode) {
    case Newest:
      sortModeString = "new";
      break;
    case Alphabetical:
      sortModeString = "alpha";
      break;
    default:
        break;
  }
  if (!sortModeString.isEmpty()) {
    url.addQueryItem("sortmode", sortModeString);
  }
  
  if (!country.isEmpty()) {
    url.addQueryItem("country", country);
  }
  
  url.addQueryItem("startat", startAt.toString(Qt::ISODate));

  url.addQueryItem("page", QString::number(page));
  url.addQueryItem("pagesize", QString::number(pageSize));

  job->setUrl(url);

  job->start();
  return job;

}

KUrl Provider::createUrl(const QString& path)
{
  KUrl url(d->m_baseUrl);
  url.addPath( path );
  return url;
}

PersonJob* Provider::doRequestPerson(const KUrl& url)
{
  PersonJob *job = new PersonJob();

  job->setUrl( url );

  job->start();
  return job;
}

ListJob<Person>* Provider::doRequestPersonList(const KUrl& url)
{
  ListJob<Person> *job = new ListJob<Person>();

  job->setUrl( url );

  job->start();
  return job;
}

ListJob<Activity>* Provider::doRequestActivityList(const KUrl& url)
{
  ListJob<Activity> *job = new ListJob<Activity>();

  job->setUrl( url );

  job->start();
  return job;
}

ListJob<Folder>* Provider::doRequestFolderList(const KUrl& url)
{
  ListJob<Folder> *job = new ListJob<Folder>();
  
  job->setUrl( url );
  job->start();
  return job;
}

ListJob<Message>* Provider::doRequestMessageList(const KUrl& url)
{
  ListJob<Message> *job = new ListJob<Message>();
  
  job->setUrl( url );
  job->start();
  return job;
}
