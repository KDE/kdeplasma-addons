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

    connect(Solid::Networking::notifier(), SIGNAL(statusChanged(Solid::Networking::Status)),
            this, SLOT(networkStatusChanged(Solid::Networking::Status)));
}

OcsEngine::~OcsEngine()
{
}

QStringList OcsEngine::sources() const
{
    return (QStringList() << "activity");
}


Plasma::Service* OcsEngine::serviceForSource(const QString& source)
{
    if (source.startsWith("Person-")) {
        QString id = QString(source).remove(0, 7);
        if (!m_personServices.contains(id)) {
            m_personServices[id] = new PersonService(id, this);
        }
        return m_personServices[id];
    }
    return Plasma::DataEngine::serviceForSource(source);
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
        QString _id = QString(name).remove(0, 8); // Removes prefix Friends-
        kDebug() << "Searching friends for id" << _id;
        PersonListJob* _job = Attica::OcsApi::requestFriend(_id, 0, m_maximumItems);
        setData(name, DataEngine::Data());
        connect( _job, SIGNAL( result( KJob * ) ), SLOT( slotFriendsResult( KJob * ) ) );

        if (_job) {
            m_personListJobs[_job] = name;
        }

        return _job != 0;
    } else if (name.startsWith("Person-")) {
        QString _id = QString(name).remove(0, 7); // Removes prefix Person-
        kDebug() << "Searching for Person id" << _id;
        PersonJob* _job = Attica::OcsApi::requestPerson(_id);
        if (m_personCache.contains(_id)) {
            // Set data already in the cache, it will hopefully get replaced by more complete data soon
            setPersonData(QString("Person-%1").arg(_id), m_personCache[_id]);
        } else {
            setData(name, DataEngine::Data());
        }
        connect( _job, SIGNAL( result( KJob * ) ), SLOT( slotPersonResult( KJob * ) ) );
        return _job != 0;
    } else if (name.startsWith("PersonSummary-")) {
        QString _id = QString(name).remove(0, 14); // Removes prefix PersonSummary-
        kDebug() << "Searching for Person Summary id" << _id;
        if (m_personCache.contains(_id)) {
            return true;
        } else {
            PersonJob* _job = Attica::OcsApi::requestPerson(_id);
            setData(name, DataEngine::Data());
            connect( _job, SIGNAL( result( KJob * ) ), SLOT( slotPersonResult( KJob * ) ) );
            return _job != 0;
        }
    } else if (name.startsWith("Near-")) {
        QStringList args = QString(name).remove(0, 5).split(':'); // Removes prefix Near-
        if (args.size() != 3) {
            kDebug() << "Don't understand your request." << name;
            kDebug() << "it should go in the format: lat:long:distance (all in degrees)";
            return false;
        }
        qreal lat = args[0].toFloat();
        qreal lon = args[1].toFloat();
        qreal dist = args[2].toFloat();

        kDebug() << "Searching for people near" << lat << lon << "distance:" << dist << m_maximumItems;
        PersonListJob* _job = Attica::OcsApi::requestPersonSearchByLocation(lat, lon, dist, 0, m_maximumItems);
        setData(name, DataEngine::Data());
        connect( _job, SIGNAL( result( KJob * ) ), SLOT( slotNearPersonsResult( KJob * ) ) );

        if (_job) {
            m_personListJobs[_job] = name;
        }

        return _job != 0;

    } else if (name.startsWith("PostLocation-")) {
        QStringList args = QString(name).remove(0, 13).split(':'); // Removes prefix PostLocation-
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
        QString _id = QString(name).remove(0, 14); // Removes prefix KnowledgeBase-
        kDebug() << "Searching for KnowledgeBase id" << _id;
        KnowledgeBaseJob* _job = Attica::OcsApi::requestKnowledgeBase(_id);
        setData(name, DataEngine::Data());
        connect( _job, SIGNAL( result( KJob * ) ), SLOT( slotKnowledgeBaseResult( KJob * ) ) );
        return _job != 0;

    } else if (name.startsWith("KnowledgeBaseList-")) {
        QStringList args = QString(name).remove(0, 18).split(':');
        const int numTokens = args.size();
        if (numTokens != 4 && numTokens != 5) {
            kDebug() << "Don't understand your request." << name;
            return false;
        }
        QString query = args[0];
        QString sortModeString = args[1];
        int page = args[2].toInt();
        int pageSize = args[3].toInt();
        int content = 0;
        if (numTokens == 5) {
            content = args[4].toInt();
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

        //putting the job/query pair into an hash to remember the association later
        if (_job) {
            m_knowledgeBaseListJobs[_job] = name;
        }

        return _job != 0;

    } else if (name.startsWith("Event-")) {
        QString _id = QString(name).remove(0, 6); // Removes prefix Event-
        EventJob* _job = Attica::OcsApi::requestEvent(_id);
        setData(name, DataEngine::Data());
        connect( _job, SIGNAL(result(KJob*)), SLOT(slotEventResult(KJob*)));
        return _job != 0;

    } else if (name.startsWith("FutureEvents-")) {
        QStringList args = QString(name).remove(0, 13).split(':');
        const int numTokens = args.size();
        if (numTokens != 1 && numTokens != 2) {
            kDebug() << "Don't understand your request." << name;
            return false;
        }
        QString country = args[0];
        QString search;
        if (numTokens == 2) {
            search = args[1];
        }

        // FIXME: The size of the request is currently hardcoded
        EventListJob* _job = OcsApi::requestEvent(country, search, QDate::currentDate(),
            OcsApi::Alphabetical, 0, 100);
        setData(name, DataEngine::Data());
        connect(_job, SIGNAL(result(KJob*)), SLOT(slotEventListResult(KJob*)));

        //putting the job/query pair into an hash to remember the association later
        if (_job) {
            m_eventListJobs.insert(_job, name);
        }

        return _job != 0;

    } else if (name.startsWith("MaximumItems-")) {
        m_maximumItems = name.split('-')[1].toInt();
        kDebug() << "Changed maximum number of hits to" << m_maximumItems;
    }
    return false;
}

bool OcsEngine::updateSourceEvent(const QString &name)
{
    sourceRequestEvent(name);
    return true;
    kDebug() << "for name" << name;
    if (name == I18N_NOOP("activity")) {

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

        foreach(const Attica::Activity &activity, m_activities ) {
            Plasma::DataEngine::Data activityData;
            activityData["id"] = activity.id();
            activityData["user"] = activity.user();
            activityData["timestamp"] = activity.timestamp();
            activityData["message"] = activity.message();
            activityData["link"] = activity.link();

            setData("activity", activity.id(), activityData);
        }
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

        QString source;

        KJob *personListJob = m_personJobs[job];
        if (personListJob) {
            if (!m_personListJobs[personListJob].isEmpty()) {
                source = m_personListJobs[personListJob];
                --m_personListJobsRefs[personListJob];
            }
            m_personJobs.remove(job);

            if (m_personListJobsRefs[personListJob] <= 0) {
                m_personListJobsRefs.remove(personListJob);
                m_personListJobs.remove(personListJob);
            }
        } else {
            source = QString("Person-%1").arg(p.id());
        }

        addToPersonCache(p.id(), p, true);
        setPersonData(source, p);
        scheduleSourcesUpdated();
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
        scheduleSourcesUpdated();
    } else {
        kDebug() << "Fetching Knowledgebase failed:" << j->errorString();
    }
}

void OcsEngine::setPersonData(const QString &source, const Attica::Person &person)
{
    kDebug() << "Setting person data"<< source;
    Plasma::DataEngine::Data personData;

    personData["Id"] = person.id();
    personData["FirstName"] = person.firstName();
    personData["LastName"] = person.lastName();
    QString n = QString("%1 %2").arg(person.firstName(), person.lastName());
    personData["Name"] = n.trimmed();
    personData["Birthday"] = person.birthday();
    personData["City"] = person.city();
    personData["Country"] = person.country();
    personData["Latitude"] = person.latitude();
    personData["Longitude"] = person.longitude();
    personData["Avatar"] = person.avatar();
    personData["AvatarUrl"] = person.avatarUrl();

    foreach(const QString &key, person.extendedAttributes().keys()) {
        personData[key] = person.extendedAttributes()[key];
    }
    setData(source, "Person-"+person.id(), personData);
}

void OcsEngine::setKnowledgeBaseData(const QString &source, const Attica::KnowledgeBase &knowledgeBase)
{
    kDebug() << "Setting KnowledgeBase data"<< source;

    Plasma::DataEngine::Data knowledgeBaseData;

    knowledgeBaseData["Id"] = knowledgeBase.id();
    knowledgeBaseData["ContentId"] = knowledgeBase.contentId();
    knowledgeBaseData["User"] = knowledgeBase.user();
    knowledgeBaseData["Status"] = knowledgeBase.status();
    knowledgeBaseData["Changed"] = knowledgeBase.changed();
    knowledgeBaseData["Name"] = knowledgeBase.name();
    knowledgeBaseData["Description"] = knowledgeBase.description();
    knowledgeBaseData["Answer"] = knowledgeBase.answer();
    knowledgeBaseData["Comments"] = knowledgeBase.comments();
    knowledgeBaseData["DetailPage"] = knowledgeBase.detailPage();

    foreach(const QString &key, knowledgeBase.extendedAttributes().keys()) {
        knowledgeBaseData[key] = knowledgeBase.extendedAttributes()[key];
    }

    setData(source, "KnowledgeBase-"+knowledgeBase.id(), knowledgeBaseData);
}

void OcsEngine::slotKnowledgeBaseListResult( KJob *j )
{
    m_job = 0;
    if (!j->error()) {
        Attica::KnowledgeBaseListJob *job = static_cast<Attica::KnowledgeBaseListJob *>( j );

        QString source = m_knowledgeBaseListJobs[job];
        if (!source.isEmpty()) {
            KnowledgeBase::Metadata meta = job->metadata();
            setData(source, "Status", meta.status);
            setData(source, "Message", meta.message);
            setData(source, "TotalItems", meta.totalItems);
            setData(source, "ItemsPerPage", meta.itemsPerPage);
            m_knowledgeBaseListJobs.remove(job);
        }

        foreach (const KnowledgeBase &k, job->knowledgeBaseList()) {
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
        Attica::PersonListJob *listJob = static_cast<Attica::PersonListJob *>( j );

        QString _id = m_personListJobs[j];
        m_personListJobs.remove(j);

        foreach (const Person &p, listJob->personList()) {
            addToPersonCache(p.id(), p);
            setPersonData(_id, p);
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
        Attica::PersonListJob *listJob = static_cast<Attica::PersonListJob *>( j );

        QString _id = m_personListJobs[j];

        foreach (const Person &p, listJob->personList()) {
            addToPersonCache(p.id(), p);
            setPersonData(_id, p);
        }
        scheduleSourcesUpdated();
    } else {
        kDebug() << "Error:" << j->errorString();
    }
}


void OcsEngine::networkStatusChanged(Solid::Networking::Status status)
{
    if (status == Solid::Networking::Connected || status == Solid::Networking::Unknown) {
        kDebug() << "network status changed" << Solid::Networking::Connected << status;
        kDebug() << "All sources:" << sources();
        updateAllSources();
    } else {
        kDebug() << "Disconnected" << status;
    }
}


void OcsEngine::addToPersonCache(const QString& id, const Attica::Person& person, bool replaceCache)
{
    if (replaceCache || !m_personCache.contains(id)) {
        // Add the person to the cache
        m_personCache[id] = person;
    } else {
        // Update and enhance the cache
        Attica::Person& cachePerson = m_personCache[id];
        if (!person.avatar().isNull()) {
            cachePerson.setAvatar(person.avatar());
        }
        if (!person.avatarUrl().isEmpty()) {
            cachePerson.setAvatarUrl(person.avatarUrl());
        }
        if (person.birthday().isValid()) {
            cachePerson.setBirthday(person.birthday());
        }
        if (!person.city().isEmpty()) {
            cachePerson.setCity(person.city());
        }
        if (!person.country().isEmpty()) {
            cachePerson.setCountry(person.country());
        }
        if (!person.firstName().isEmpty()) {
            cachePerson.setFirstName(person.firstName());
        }
        if (!person.homepage().isEmpty()) {
            cachePerson.setHomepage(person.homepage());
        }
        if (!person.id().isEmpty()) {
            cachePerson.setId(person.id());
        }
        if (!person.lastName().isEmpty()) {
            cachePerson.setLastName(person.lastName());
        }
        if (person.latitude() && person.longitude()) {
            cachePerson.setLatitude(person.latitude());
            cachePerson.setLongitude(person.longitude());
        }
        foreach(const QString& key, person.extendedAttributes()) {
            cachePerson.addExtendedAttribute(key, person.extendedAttribute(key));
        }
    }
    setPersonData(QString("PersonSummary-%1").arg(id), m_personCache[id]);
}


void OcsEngine::slotEventResult(KJob* j)
{
    if (!j->error()) {
        Attica::EventJob* job = static_cast<Attica::EventJob*>(j);
        Attica::Event k = job->event();
        setEventData(QString("Event-%1").arg(k.id()), k);
        scheduleSourcesUpdated();
    } else {
        kDebug() << "Fetching Event failed:" << j->errorString();
    }
}

void OcsEngine::setEventData(const QString& source, const Event& event)
{
    Plasma::DataEngine::Data eventData;

    eventData["Id"] = event.id();
    eventData["Name"] = event.name();
    eventData["Description"] = event.description();
    eventData["User"] = event.user();
    eventData["StartDate"] = event.startDate();
    eventData["EndDate"] = event.endDate();
    eventData["Latitude"] = event.latitude();
    eventData["Longitude"] = event.longitude();
    eventData["Homepage"] = event.homepage();
    eventData["Country"] = event.country();
    eventData["City"] = event.city();

    foreach(const QString& key, event.extendedAttributes().keys()) {
        eventData[key] = event.extendedAttributes()[key];
    }

    setData(source, "Event-" + event.id(), eventData);
}

void OcsEngine::slotEventListResult(KJob* j)
{
    if (!j->error()) {
        EventListJob* job = static_cast<EventListJob*>(j);

        QString source = m_eventListJobs[job];

        m_eventListJobs.remove(job);

        foreach (const Event& event, job->eventList()) {
            setEventData(source, event);
        }
        scheduleSourcesUpdated();
    } else {
        kDebug() << "Error:" << j->errorString();
    }
}



#include "ocsengine.moc"

