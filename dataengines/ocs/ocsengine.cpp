/*
 *   Copyright (C) 2008 Dirk Mueller <mueller@kde.org>
 *   Copyright (C) 2009 Sebastian Kügler <sebas@kde.org>
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

#include <QTimer>
#include <QProcess>

#include <KLocale>

#include "ocsengine.h"
#include "activitylistjob.h"
#include "ocsapi.h"

#include <plasma/datacontainer.h>

using namespace Attica;

OcsEngine::OcsEngine(QObject* parent, const QVariantList& args)
    : Plasma::DataEngine(parent)
{
    // fairy random, should be set from the applet by calling
    m_maximumItems = 99;
    Q_UNUSED(args);
    setMinimumPollingInterval(500);
}

OcsEngine::~OcsEngine()
{
}

QStringList OcsEngine::sources() const
{
    return (QStringList() << "activity");
}

bool OcsEngine::sourceRequestEvent(const QString &name)
{
    kDebug() << "for name" << name;
    if (name == I18N_NOOP("activity")) {
        m_job = Attica::OcsApi::requestActivity();
        setData(name, DataEngine::Data());
        connect( m_job, SIGNAL( result( KJob * ) ), SLOT( slotActivityResult( KJob * ) ) );
        return m_job != 0;
    } else if (name.startsWith("Friends-")) {
        QString _id = QString(name).replace(QString("Friends-"), QString());
        kDebug() << "Searching friends for id" << _id;
        PersonListJob* _job = Attica::OcsApi::requestFriend(_id, 0, m_maximumItems);
        setData(name, DataEngine::Data());
        connect( _job, SIGNAL( result( KJob * ) ), SLOT( slotFriendsResult( KJob * ) ) );
        return _job != 0;
    } else if (name.startsWith("Person-")) {
        QString _id = QString(name).replace(QString("Person-"), QString());
        kDebug() << "Searching for Person id" << _id;
        PersonJob* _job = Attica::OcsApi::requestPerson(_id);
        setData(name, DataEngine::Data());
        connect( _job, SIGNAL( result( KJob * ) ), SLOT( slotPersonResult( KJob * ) ) );
        return _job != 0;
    } else if (name.startsWith("Near-")) {
        if (name.split(':').count() != 3) {
            kDebug() << "Don't understand your request." << name;
            kDebug() << "it should go in the format: lat:long:distance (all in degrees)";
            return false;
        }
        qreal lat = QString(name).replace(QString("Near-"), QString()).split(":")[0].toFloat();
        qreal lon = QString(name).replace(QString("Near-"), QString()).split(":")[1].toFloat();
        qreal dist = QString(name).replace(QString("Near-"), QString()).split(":")[2].toFloat();

        kDebug() << "Searching for people near" << lat << lon << "distance:" << dist << m_maximumItems;
        PersonListJob* _job = Attica::OcsApi::requestPersonSearchByLocation(lat, lon, dist, 0, m_maximumItems);
        setData(name, DataEngine::Data());
        connect( _job, SIGNAL( result( KJob * ) ), SLOT( slotNearPersonsResult( KJob * ) ) );
        return _job != 0;
        
    } else if (name.startsWith("PostLocation-")) {
        
        QStringList args = QString(name).replace(QString("PostLocation-"), QString()).split(":");
        if (args.size() != 4) {
            kDebug() << "Invalid location string:" << name;
            kDebug() << "it should go in the format: lat:long:country:city";
            return false;
        }
        qreal lat = args[0].toFloat();
        qreal lon = args[1].toFloat();
        QString country = args[2];
        QString city = args[3];
        
        kDebug() << "Posting location:" << lat << lon << country << city;
        PostJob* _job = Attica::OcsApi::postLocation(lat, lon, city, country);
        connect(_job, SIGNAL( result( KJob* ) ), SLOT( locationPosted( KJob* ) ));
        return _job != 0;

    } else if (name.startsWith("KnowledgeBase-")) {
        QString _id = QString(name).replace(QString("KnowledgeBase-"), QString());
        kDebug() << "Searching for KnowledgeBase id" << _id;
        KnowledgeBaseJob* _job = Attica::OcsApi::requestKnowledgeBase(_id);
        setData(name, DataEngine::Data());
        connect( _job, SIGNAL( result( KJob * ) ), SLOT( slotKnowledgeBaseResult( KJob * ) ) );
        return _job != 0;

    } else if (name.startsWith("KnowledgeBaseList-")) {
        const int numTokens = name.split(':').count();
        if (numTokens != 4 && numTokens != 5) {
            kDebug() << "Don't understand your request." << name;
            return false;
        }
        QString query = QString(name).replace(QString("KnowledgeBaseList-"), QString()).split(":")[0];
        QString sortModeString = QString(name).replace(QString("KnowledgeBaseList-"), QString()).split(":")[1];
        int page = QString(name).replace(QString("KnowledgeBaseList-"), QString()).split(":")[2].toInt();
        int pageSize = QString(name).replace(QString("KnowledgeBaseList-"), QString()).split(":")[3].toInt();
        int content = 0;
        if (numTokens == 5) {
            content = QString(name).replace(QString("KnowledgeBaseList-"), QString()).split(":")[4].toInt();
        }

        Attica::OcsApi::SortMode sortMode;

        if (sortModeString == "new") {
            sortMode = Attica::OcsApi::Newest;
        } else if (sortModeString == "alpha") {
            sortMode = Attica::OcsApi::Alphabetical;
        } else if (sortModeString == "high") {
            sortMode = Attica::OcsApi::Rating;
        } else {
            sortMode = Attica::OcsApi::Newest;
        }

        kDebug() << "Searching for" << query << "into knowledge base";
        KnowledgeBaseListJob* _job = Attica::OcsApi::requestKnowledgeBase(content, query, sortMode, page, pageSize);
        setData(name, DataEngine::Data());
        connect( _job, SIGNAL( result( KJob * ) ), SLOT( slotKnowledgeBaseListResult( KJob * ) ) );
        return _job != 0;

    } else if (name.startsWith("MaximumItems-")) {
        m_maximumItems = name.split("-")[1].toInt();
        kDebug() << "Changed maximum number of hits to" << m_maximumItems;
    }
    return false;
}

bool OcsEngine::updateSourceEvent(const QString &name)
{
    kDebug() << "for name" << name;
    if (name == I18N_NOOP("activity")) {
        foreach(const Attica::Activity &activity, m_activities ) {
            setData(name, I18N_NOOP("user"), activity.user());
            setData(name, I18N_NOOP("timestamp"), activity.timestamp());
            setData(name, I18N_NOOP("message"), activity.message());
        }
        return true;
    } else if (name.startsWith("Friends-")) {

    }

    return false;
}

void OcsEngine::slotActivityResult( KJob *j )
{
    m_job = 0;
    if (!j->error()) {
        Attica::ActivityListJob *job = static_cast<Attica::ActivityListJob *>( j );
        m_activities = job->ActivityList();
        updateSourceEvent(I18N_NOOP("activity"));
    }
}

void OcsEngine::locationPosted( KJob *j )
{
    if (!j->error()) {
        updateSourceEvent(I18N_NOOP("activity"));
    } else {
        kDebug() << "location posted returned an error:" << j->errorString();
    }
}

void OcsEngine::slotPersonResult( KJob *j )
{
    kDebug() << "============================= Person Full Data is in";
    if (!j->error()) {
        Attica::PersonJob *job = static_cast<Attica::PersonJob *>( j );
        Attica::Person p = job->person();
        setPersonData(QString("Person-%1").arg(p.id()), p);
    } else {
        kDebug() << "Fetching person failed:" << j->errorString();
    }
}

void OcsEngine::slotKnowledgeBaseResult( KJob *j )
{
    kDebug() << "============================= KnowledgeBase Full Data is in";
    if (!j->error()) {
        Attica::KnowledgeBaseJob *job = static_cast<Attica::KnowledgeBaseJob *>( j );
        Attica::KnowledgeBase k = job->knowledgeBase();
        setKnowledgeBaseData(QString("KnowledgeBase-%1").arg(k.id()), k);
    } else {
        kDebug() << "Fetching Knowledgebase failed:" << j->errorString();
    }
}

void OcsEngine::slotNearResult( KJob *j )
{
    if (!j->error()) {
        kDebug() << "============================= Near Person Full Data is in";
        Attica::PersonJob *job = static_cast<Attica::PersonJob *>( j );
        Attica::Person p = job->person();
        setPersonData(QString("Near-%1").arg(p.id()), p);
    } else {
        kDebug() << "Fetching person failed:" << j->errorString();
    }
}
        
        
void OcsEngine::setPersonData(const QString &source, Attica::Person &person)
{
    kDebug() << "Setting person data"<< source;
    setData(source, "Id", person.id());
    setData(source, "FirstName", person.firstName());
    setData(source, "LastName", person.lastName());
    QString n = QString("%1 %2").arg(person.firstName(), person.lastName());
    setData(source, "Name", n.trimmed());
    setData(source, "Birthday", person.birthday());
    setData(source, "City", person.city());
    setData(source, "Country", person.country());
    setData(source, "Latitude", person.latitude());
    setData(source, "Longitude", person.longitude());
    setData(source, "Avatar", person.avatar());
    setData(source, "AvatarUrl", person.avatarUrl());
    foreach(const QString &key, person.extendedAttributes().keys()) {
        setData(source, key, person.extendedAttributes()[key]);
    }
    scheduleSourcesUpdated();
}

void OcsEngine::setKnowledgeBaseData(const QString &source, Attica::KnowledgeBase &knowledgeBase)
{
    kDebug() << "Setting KnowledgeBase data"<< source;
    setData(source, "Id", knowledgeBase.id());
    setData(source, "ContentId", knowledgeBase.contentId());
    setData(source, "User", knowledgeBase.user());
    setData(source, "Status", knowledgeBase.status());
    setData(source, "Changed", knowledgeBase.changed());
    setData(source, "Name", knowledgeBase.name());
    setData(source, "Description", knowledgeBase.description());
    setData(source, "Answer", knowledgeBase.answer());
    setData(source, "Comments", knowledgeBase.comments());
    setData(source, "DetailPage", knowledgeBase.detailPage());

    foreach(const QString &key, knowledgeBase.extendedAttributes().keys()) {
        setData(source, key, knowledgeBase.extendedAttributes()[key]);
    }
    scheduleSourcesUpdated();
}

void OcsEngine::slotKnowledgeBaseListResult( KJob *j )
{
    m_job = 0;
    if (!j->error()) {
        Attica::KnowledgeBaseListJob *job = static_cast<Attica::KnowledgeBaseListJob *>( j );

        foreach (KnowledgeBase k, job->knowledgeBaseList()) {
            const QString source = QString("KnowledgeBase-%1").arg(k.id());

            setKnowledgeBaseData(source, k);
        }
        scheduleSourcesUpdated();
    } else {
        kDebug() << "Error:" << j->errorString();
    }
}

void OcsEngine::slotNearPersonsResult( KJob *j )
{
    m_job = 0;
    if (!j->error()) {
        Attica::PersonListJob *job = static_cast<Attica::PersonListJob *>( j );

        foreach (const Person &p, job->personList()) {
            const QString personId = QString("%1").arg(p.id());
            Attica::PersonJob* job = Attica::OcsApi::requestPerson(personId);
            connect(job, SIGNAL(result(KJob*)), this, SLOT(slotNearResult(KJob*)));
            QString _id = QString("Near-%1").arg(p.id());
            setData(_id, Plasma::DataEngine::Data());
            kDebug() << "New Near:" << _id << personId;
        }
        scheduleSourcesUpdated();
    } else {
        kDebug() << "Error:" << j->errorString();
    }
}


void OcsEngine::slotFriendsResult( KJob *j )
{
    m_job = 0;
    if (!j->error()) {
        Attica::PersonListJob *job = static_cast<Attica::PersonListJob *>( j );

        foreach (const Person &p, job->personList()) {
            const QString personId = QString("%1").arg(p.id());
            Attica::PersonJob* job = Attica::OcsApi::requestPerson(personId);
            connect(job, SIGNAL(result(KJob*)), this, SLOT(slotPersonResult(KJob*)));
            QString _id = QString("Person-%1").arg(p.id());
            setData(_id, Plasma::DataEngine::Data());
            kDebug() << "New Friend:" << _id << personId;
        }
        scheduleSourcesUpdated();
    } else {
        kDebug() << "Error:" << j->errorString();
    }
}

#include "ocsengine.moc"

