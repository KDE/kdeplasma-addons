/*
 *   Copyright (C) 2008 Dirk Mueller <mueller@kde.org>
 *   Copyright (C) 2009 Sebastian K?gler <sebas@kde.org>
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

#include <attica/atticabasejob.h>
#include <attica/content.h>
#include <attica/event.h>
#include <attica/folder.h>
#include <attica/knowledgebaseentry.h>
#include <attica/message.h>
#include <attica/person.h>
#include <attica/postjob.h>
#include <attica/provider.h>
#include <attica/providermanager.h>

#include <QDebug>
#include <KDebug>
#include <KIO/Job>

#include <plasma/datacontainer.h>
#include <attica/metadata.h>


using namespace Attica;

OcsEngine::OcsEngine(QObject* parent, const QVariantList& args)
    : Plasma::DataEngine(parent),
      m_serviceUpdates(new QSignalMapper)
{
    Q_UNUSED(args);
    setName("ocs");

    setMinimumPollingInterval(1000);

    connect(Solid::Networking::notifier(), SIGNAL(statusChanged(Solid::Networking::Status)),
            this, SLOT(networkStatusChanged(Solid::Networking::Status)));

    m_pm.setAuthenticationSuppressed(true);
    connect(&m_pm, SIGNAL(providerAdded(Attica::Provider)), SLOT(providerAdded(Attica::Provider)));
    m_pm.loadDefaultProviders();
    connect(m_serviceUpdates.data(), SIGNAL(mapped(QString)), SLOT(serviceUpdates(QString)));
}


Plasma::Service* OcsEngine::serviceForSource(const QString& source)
{
    QPair<QString, QHash<QString, QString> > parsedSource = parseSource(source);
    QString request = parsedSource.first;
    QHash<QString, QString> arguments = parsedSource.second;

    qDebug() << "Service request:" << request << "- arguments:" << arguments;

    if (request == "Person" || request == "Settings") {
        QString id = arguments.value("id");
        QString providerString = arguments.value("provider");
        if (!id.isEmpty() && !providerString.isEmpty() && m_providers.value(providerString)) {
            return new PersonService(m_providers.value(providerString), id, m_serviceUpdates, this);
        }
    }
    return Plasma::DataEngine::serviceForSource(source);
}


QStringList OcsEngine::split(const QString& encodedString)
{
    QStringList lines;
    const char separator = '\\';
    int startPos = 0;
    int foundPos = -1;
    while (true) {
        foundPos = encodedString.indexOf(separator, foundPos + 1);
        if (foundPos == -1) {
            QString remaining = encodedString.mid(startPos);
            if (!remaining.isEmpty()) {
                lines << remaining;
            }
            return lines;
        } else {
            if (foundPos != encodedString.size() - 1 && encodedString.at(foundPos + 1) == separator) {
                // We found an escaping, just skipping it for now
                ++foundPos;
            } else {
                lines << encodedString.mid(startPos, foundPos - startPos).replace("\\\\", "\\");
                startPos = foundPos + 1;
            }
        }
    }
}


QString OcsEngine::encode(const QString& s) {
    return QString(s).replace('\\', "\\\\");
}


QPair<QString, QHash<QString, QString> > OcsEngine::parseSource(const QString& source) {
    QStringList lines = split(source);

    QString request;
    QHash<QString, QString> arguments;

    for (QStringList::const_iterator i = lines.constBegin(); i != lines.constEnd(); ++i) {
        if (i == lines.constBegin()) {
            request = *i;
        } else {
            int splitPos = (*i).indexOf(':');
            if (splitPos == -1) {
                return qMakePair(QString(), QHash<QString, QString>());
            }
            QString key = (*i).left(splitPos);
            QString value = (*i).mid(splitPos + 1);
            arguments.insert(key, value);
        }
    }

    return qMakePair(request, arguments);
}


bool OcsEngine::providerDependentRequest(const QString& request, const QHash<QString, QString>& arguments, const QString& source, const QString& baseUrl, Provider* provider)
{
    if (request == "Activities") {
        setData(source, DataEngine::Data());
        if (provider) {
            ListJob<Activity>* job = provider->requestActivities();
            connect(job, SIGNAL(finished(Attica::BaseJob*)), SLOT(slotActivityResult(Attica::BaseJob*)));
            m_jobs.insert(job, source);
            job->start();
        }
        return true;
    } else if (request == "Friends") {
        if (!arguments.contains("id")) {
            return false;
        }
        setData(source, DataEngine::Data());
        if (provider) {
            ListJob<Person>* job = provider->requestFriends(arguments.value("id"));
            connect(job, SIGNAL(finished(Attica::BaseJob*)), SLOT(slotPersonListResult(Attica::BaseJob*)));
            m_jobs.insert(job, source);
            job->start();
        }
        return true;
    } else if (request == "SentInvitations") {
        setData(source, DataEngine::Data());
        if (provider) {
            ListJob<Person>* job = provider->requestSentInvitations();
            connect(job, SIGNAL(finished(Attica::BaseJob*)), SLOT(slotPersonListResult(Attica::BaseJob*)));
            m_jobs.insert(job, source);
            job->start();
        }
        return true;
    } else if (request == "ReceivedInvitations") {
        setData(source, DataEngine::Data());
        if (provider) {
            ListJob<Person>* job = provider->requestReceivedInvitations();
            connect(job, SIGNAL(finished(Attica::BaseJob*)), SLOT(slotPersonListResult(Attica::BaseJob*)));
            m_jobs.insert(job, source);
            job->start();
        }
        return true;
    } else if (request == "Person") {
        if (!arguments.contains("id")) {
            return false;
        }
        // FIXME
        QString id = arguments.value("id");
        if (m_personCache.contains(qMakePair(baseUrl, id))) {
            // Set data already in the cache, it will hopefully get replaced by more complete data soon
            setPersonData(source, m_personCache.value(qMakePair(baseUrl, id)));
        } else {
            setData(source, DataEngine::Data());
        }
        if (provider) {
            ItemJob<Person>* job = provider->requestPerson(id);
            connect(job, SIGNAL(finished(Attica::BaseJob*)), SLOT(slotPersonResult(Attica::BaseJob*)));
            m_jobs.insert(job, source);
            job->start();
        }
        return true;
    } else if (request == "PersonCheck") {
        setData(source, DataEngine::Data());
        if (provider) {
            // FIXME: Implement using the self mechanism
            ItemJob<Person>* job = provider->requestPersonSelf();
            connect(job, SIGNAL(finished(Attica::BaseJob*)), SLOT(slotPersonResult(Attica::BaseJob*)));
            m_jobs.insert(job, source);
            job->start();
        }
        return true;
    } else if (request == "PersonSummary") {
        if (!arguments.contains("id")) {
            return false;
        }
        QString id = arguments.value("id");
        if (!m_personCache.contains(qMakePair(baseUrl, id))) {
            setData(source, DataEngine::Data());
            if (provider) {
                ItemJob<Person>* job = provider->requestPerson(id);
                connect(job, SIGNAL(finished(Attica::BaseJob*)), SLOT(slotPersonResult(Attica::BaseJob*)));
                m_jobs.insert(job, source);
            job->start();
            }
        }
        return true;
    } else if (request == "Near") {
        if (!arguments.contains("latitude") || !arguments.contains("longitude")) {
            return false;
        }
        qreal dist = arguments.value("distance").toFloat();
        qreal lat = arguments.value("latitude").toFloat();
        qreal lon = arguments.value("longitude").toFloat();
        setData(source, DataEngine::Data());
        if (provider) {
            ListJob<Person>* job = provider->requestPersonSearchByLocation(lat, lon, dist);
            connect(job, SIGNAL(finished(Attica::BaseJob*)), SLOT(slotPersonListResult(Attica::BaseJob*)));
            m_jobs.insert(job, source);
            job->start();
        }
        return true;
    } else if (request == "PostLocation") {
        // FIXME: This should be implemented as a service
        if (!arguments.contains("city") || !arguments.contains("country") || !arguments.contains("latitude") || !arguments.contains("longitude")) {
            return false;
        }
        QString city = arguments.value("city");
        QString country = arguments.value("country");
        qreal lat = arguments.value("latitude").toDouble();
        qreal lon = arguments.value("longitude").toDouble();
        if (provider) {
            Attica::BaseJob* job = provider->postLocation(lat, lon, city, country);
            connect(job, SIGNAL(finished(Attica::BaseJob*)), SLOT(locationPosted(Attica::BaseJob*)));
            job->start();
        }
        return true;
    } else if (request == "KnowledgeBase") {
        if (!arguments.contains("id")) {
            return false;
        }
        setData(source, DataEngine::Data());
        if (provider) {
            ItemJob<KnowledgeBaseEntry>* job = provider->requestKnowledgeBaseEntry(arguments.value("id"));
            connect(job, SIGNAL(finished(Attica::BaseJob*)), SLOT(slotKnowledgeBaseResult(Attica::BaseJob*)));
            m_jobs.insert(job, source);
            job->start();
        }
        return true;
    } else if (request == "KnowledgeBaseList") {
        if (!arguments.contains("page") || !arguments.contains("pageSize") || !arguments.contains("query") || !arguments.contains("sortMode")) {
            return false;
        }
        setData(source, DataEngine::Data());
        Content content;
        content.setId(arguments.value("contentId"));
        int page = arguments.value("page").toInt();
        int pageSize = arguments.value("pageSize").toInt();
        QString query = arguments.value("query");
        QString sortModeString = arguments.value("sortMode");

        Provider::SortMode sortMode;
        if (sortModeString == "new") {
            sortMode = Provider::Newest;
        } else if (sortModeString == "alpha") {
            sortMode = Provider::Alphabetical;
        } else if (sortModeString == "high") {
            sortMode = Provider::Rating;
        } else {
            sortMode = Provider::Newest;
        }

        if (provider) {
            ListJob<KnowledgeBaseEntry>* job = provider->searchKnowledgeBase(content, query, sortMode, page, pageSize);
            connect(job, SIGNAL(finished(Attica::BaseJob*)), SLOT(slotKnowledgeBaseListResult(Attica::BaseJob*)));
            m_jobs.insert(job, source);
            job->start();
        }
        return true;
    } else if (request == "Event") {
        if (!arguments.contains("id")) {
            return false;
        }
        setData(source, DataEngine::Data());
        if (provider) {
            ItemJob<Event>* job = provider->requestEvent(arguments.value("id"));
            connect(job, SIGNAL(finished(Attica::BaseJob*)), SLOT(slotEventResult(Attica::BaseJob*)));
            m_jobs.insert(job, source);
            job->start();
        }
        return true;
    } else if (request == "FutureEvents") {
        if (!arguments.contains("country")) {
            return false;
        }
        QString country = arguments.value("country");
        QString query = arguments.value("query");
        setData(source, DataEngine::Data());
        if (provider) {
            ListJob<Event>* job = provider->requestEvent(country, query, QDate::currentDate(), Provider::Alphabetical, 0, 100);
            connect(job, SIGNAL(finished(Attica::BaseJob*)), SLOT(slotEventListResult(Attica::BaseJob*)));
            m_jobs.insert(job, source);
            job->start();
        }
        return true;
    } else if (request == "Folders") {
        setData(source, DataEngine::Data());
        if (provider) {
            ListJob<Folder>* job = provider->requestFolders();
            connect(job, SIGNAL(finished(Attica::BaseJob*)), SLOT(slotFolderListResult(Attica::BaseJob*)));
            m_jobs.insert(job, source);
            job->start();
        }
        return true;
    } else if (request == "Messages") {
        if (!arguments.contains("folder")) {
            return false;
        }
        QString folderId = arguments.value("folder");
        QString status = arguments.value("status");
        setData(source, DataEngine::Data());
        if (provider) {
            Folder folder;
            folder.setId(folderId);
            ListJob<Message>* job;
            if (status == "unread") {
                job = provider->requestMessages(folder, Message::Unread);
            } else if (status == "read") {
                job = provider->requestMessages(folder, Message::Read);
            } else if (status == "read") {
                job = provider->requestMessages(folder, Message::Answered);
            } else {
                job = provider->requestMessages(folder);
            }
            connect(job, SIGNAL(finished(Attica::BaseJob*)), SLOT(slotMessageListResult(Attica::BaseJob*)));
            m_jobs.insert(job, source);
            job->start();
        }
        return true;
    } else if (request == "Message") {
        if (!arguments.contains("folder") || !arguments.contains("id")) {
            return false;
        }
        QString folderId = arguments.value("folder");
        QString messageId = arguments.value("id");
        setData(source, DataEngine::Data());
        if (provider) {
            Folder folder;
            folder.setId(folderId);
            ItemJob<Message>* job = provider->requestMessage(folder, messageId);
            connect(job, SIGNAL(finished(Attica::BaseJob*)), SLOT(slotMessageResult(Attica::BaseJob*)));
            m_jobs.insert(job, source);
            job->start();
        }
        return true;
    } else if (request == "MessageSummary") {
        if (!arguments.contains("folder") || !arguments.contains("id")) {
            return false;
        }
        // FIXME: We should not ignore the folder, but since message ids are unique across folders, it doesn't matter
        QString id = arguments.value("id");
        setMessageData(source, m_messageCache.value(qMakePair(baseUrl, id)));
        return true;
    } else if (request == "Credentials") {
        kDebug() << "load credentials...";
        if (provider) {
            QString user;
            QString password;
            if (provider->hasCredentials() && provider->loadCredentials(user, password)) {
                setData(source, "UserName", user);
                setData(source, "Password", password);
                setData(source, "SourceStatus", "success");
                kDebug() << "user: " << user;
            }
        }
        return true;
    }
    return false;
}

bool OcsEngine::sourceRequestEvent(const QString& source)
{
    QPair<QString, QHash<QString, QString> > parsedSource = parseSource(source);
    QString request = parsedSource.first;
    QHash<QString, QString> arguments = parsedSource.second;

    kDebug() << "Source request:" << request << "- arguments:" << arguments;

    if (request.isEmpty()) {
        return false;
    }

    if (!arguments.contains("provider")) {
        if (request == "Providers") {
            kDebug() << "Providers requested";
            updateProviderData();
            return true;
        } else if (request == "Pixmap") {
            if (!arguments.contains("url")) {
                return false;
            }
            setData(source, "Pixmap", QVariant(QPixmap()));
            KIO::TransferJob* job = KIO::get(arguments.value("url"), KIO::NoReload, KIO::HideProgressInfo);
            m_pixmapJobs.insert(job, source);
            connect(job, SIGNAL(data(KIO::Job*,QByteArray)), SLOT(slotPixmapData(KIO::Job*,QByteArray)));
            connect(job, SIGNAL(finished(KJob*)), SLOT(slotPixmapResult(KJob*)));
            job->start();
            return true;
        }
        return false;
    } else {
        // Provider dependent queries
        QString baseUrl = arguments.value("provider");

        Provider* provider = m_providers.value(baseUrl).data();
        if (!provider) {
            m_sourcesWithoutProvider[baseUrl].append(source);
        }
        bool success = providerDependentRequest(request, arguments, source, baseUrl, provider);
        if (!success) {
            qDebug() << "Source failed:" << source;
        } else if (!provider) {
            // Cache the request till the provider becomes available
            m_requestCache[baseUrl].insert(source);
        } else {
            setData(source, "SourceStatus", "retrieving");
        }
        return success;
    }
    return false;
}


bool OcsEngine::updateSourceEvent(const QString &name)
{
    sourceRequestEvent(name);
    return true;
}

void OcsEngine::slotActivityResult(BaseJob* j)
{
    QString source = m_jobs.take(j);
    if (j->metadata().error() == Metadata::NoError) {
        Attica::ListJob<Activity> *job = static_cast<Attica::ListJob<Activity> *>( j );

        foreach(const Attica::Activity &activity, job->itemList()) {
            Plasma::DataEngine::Data activityData;
            activityData["id"] = activity.id();
            activityData["user-Id"] = activity.associatedPerson().id();
            activityData["user-AvatarUrl"] = activity.associatedPerson().avatarUrl();
            activityData["timestamp"] = activity.timestamp();
            activityData["message"] = activity.message();
            activityData["link"] = activity.link();

            setData(source, activity.id(), activityData);
        }
    }
    setStatusData(source, j);
}

void OcsEngine::locationPosted( BaseJob *j )
{
    if (j->metadata().error() == Metadata::NoError) {
        updateSourceEvent("activity");
    } else {
        kDebug() << "location posted returned an error:" << j->metadata().statusString();
    }
}


void OcsEngine::slotPersonResult(BaseJob* j)
{
    QString source = m_jobs.take(j);
    if (j->metadata().error() == Metadata::NoError) {
        ItemJob<Person>* personJob = static_cast<ItemJob<Person>*>(j);
        Attica::Person p = personJob->result();
        kDebug() << p.firstName();

        addToPersonCache(source, p, true);
        setPersonData(source, p);
    }
    setStatusData(source, j);
}


void OcsEngine::slotKnowledgeBaseResult(BaseJob* j)
{
    QString source = m_jobs.take(j);
    if (j->metadata().error() == Metadata::NoError) {
        ItemJob<KnowledgeBaseEntry>* job = static_cast<ItemJob<KnowledgeBaseEntry>*>( j );
        KnowledgeBaseEntry k = job->result();
        setKnowledgeBaseData(source, k);
    }
    setStatusData(source, j);
}


void OcsEngine::setStatusData(const QString& source, BaseJob* job)
{
    Metadata meta = job->metadata();
    if (meta.error() == Metadata::NoError) {
        setData(source, "SourceStatus", "success");
        setData(source, "Status", meta.statusString());
        setData(source, "Message", meta.message());
        setData(source, "TotalItems", meta.totalItems());
        setData(source, "ItemsPerPage", meta.itemsPerPage());
    } else {
        setData(source, "SourceStatus", "failure");
        setData(source, "Status", QVariant());
        setData(source, "Message", QVariant());
        setData(source, "TotalItems", QVariant());
        setData(source, "ItemsPerPage", QVariant());
    }
}


void OcsEngine::setPersonData(const QString& source, const Attica::Person& person, bool keyOnly)
{
    if (keyOnly) {
        setData(source, "Person-" + person.id(), person.id());
    } else {
        kDebug() << "Setting person data" << source;
        Plasma::DataEngine::Data personData;

        personData.insert("Id", person.id());
        personData.insert("FirstName", person.firstName());
        personData.insert("LastName", person.lastName());
        QString name = QString("%1 %2").arg(person.firstName(), person.lastName());
        personData.insert("Name", name.trimmed());
        personData.insert("Birthday", person.birthday());
        personData.insert("City", person.city());
        personData.insert("Country", person.country());
        personData.insert("Latitude", person.latitude());
        personData.insert("Longitude", person.longitude());
        personData.insert("AvatarUrl", person.avatarUrl());

        QMap<QString, QString> attributes = person.extendedAttributes();
        for(QMap<QString, QString>::const_iterator i = attributes.constBegin(); i != attributes.constEnd(); ++i) {
            personData.insert(i.key(), i.value());
        }
        setData(source, "Person-" + person.id(), personData);
    }
}

void OcsEngine::setKnowledgeBaseData(const QString &source, const Attica::KnowledgeBaseEntry &knowledgeBase)
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

void OcsEngine::slotKnowledgeBaseListResult(BaseJob* j)
{
    QString source = m_jobs.take(j);
    if (j->metadata().error() == Metadata::NoError) {
        ListJob<KnowledgeBaseEntry>* job = static_cast<ListJob<KnowledgeBaseEntry>*>(j);

        if (!source.isEmpty()) {
            Metadata meta = job->metadata();
            setData(source, "Status", meta.statusString());
            setData(source, "Message", meta.message());
            setData(source, "TotalItems", meta.totalItems());
            setData(source, "ItemsPerPage", meta.itemsPerPage());
        }

        foreach (const KnowledgeBaseEntry &k, job->itemList()) {
            setKnowledgeBaseData(source, k);
        }
    } else {
        qDebug() << "Getting knowledgebase list" << source << "failed with code" << j->metadata().statusCode();
    }
    setStatusData(source, j);
    forceImmediateUpdateOfAllVisualizations();
}

void OcsEngine::slotPersonListResult(BaseJob* j)
{
    QString source = m_jobs.take(j);
    if (j->metadata().error() == Metadata::NoError) {
        Attica::ListJob<Person> *listJob = static_cast<Attica::ListJob<Person> *>( j );

        Data data;
        foreach (const Person& p, listJob->itemList()) {
            addToPersonCache(source, p);
            data.insert("Person-" + p.id(), QVariant(p.id()));
        }
        setData(source, data);
        qDebug() << "Got a list of persons for" << source << ":" << data.keys();
    } else {
        qDebug() << "Getting person list" << source << "failed with code" << j->metadata().statusCode();
    }
    setStatusData(source, j);
}


void OcsEngine::slotPixmapData(KIO::Job* j, const QByteArray& data) {
    m_pixmapData[j].append(data);
}


void OcsEngine::slotPixmapResult(KJob* j)
{
    QString source = m_pixmapJobs.take(j);
    if (!j->error()) {
        QPixmap pixmap;
        pixmap.loadFromData(m_pixmapData.take(j));
        setData(source, "Pixmap", QVariant(pixmap));
        setData(source, "SourceStatus", "success");
    } else {
        setData(source, "SourceStatus", "failure");
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


void OcsEngine::addToPersonCache(const QString& source, const Attica::Person& person, bool replaceCache)
{
    QPair<QString, QHash<QString, QString> > parsedSource = parseSource(source);
    QString provider = parsedSource.second.value("provider");
    QString id = person.id();
    QPair<QString, QString> key = qMakePair(provider, id);

    if (replaceCache || !m_personCache.contains(key)) {
        // Add the person to the cache
        m_personCache[key] = person;
    } else {
        // Update and enhance the cache
        Attica::Person cachePerson = m_personCache.value(key);
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
        m_personCache[key] = cachePerson;
    }
    setPersonData(QString("PersonSummary\\provider:%1\\id:%2").arg(provider).arg(id), m_personCache.value(key));
}


void OcsEngine::addToMessageCache(const QString& source, const Message& message, bool replaceCache)
{
    QPair<QString, QHash<QString, QString> > parsedSource = parseSource(source);
    QString provider = parsedSource.second.value("provider");
    QString folder = parsedSource.second.value("folder");
    QString id = message.id();
    QPair<QString, QString> key = qMakePair(provider, id);

    if (replaceCache || !m_messageCache.contains(key)) {
        // Add the person to the cache
        m_messageCache[key] = message;
    } else {
        // Update and enhance the cache
        Message cacheMessage = m_messageCache.value(key);
        if (!message.body().isEmpty()) {
            cacheMessage.setBody(message.body());
        }
        if (!message.from().isEmpty()) {
            cacheMessage.setFrom(message.from());
        }
        if (!message.to().isEmpty()) {
            cacheMessage.setTo(message.to());
        }
        cacheMessage.setStatus(message.status());
        if (!message.subject().isEmpty()) {
            cacheMessage.setSubject(message.subject());
        }
        if (message.sent().isValid()) {
            cacheMessage.setSent(message.sent());
        }
        m_messageCache[key] = cacheMessage;
    }
    setMessageData(QString("MessageSummary\\provider:%1\\folder:%2\\id:%3").arg(provider).arg(folder).arg(id), m_messageCache.value(key));
}


void OcsEngine::slotEventResult(BaseJob* j)
{
    QString source = m_jobs.take(j);
    if (j->metadata().error() == Metadata::NoError) {
        ItemJob<Event>* job = static_cast<ItemJob<Event>*>(j);
        Event k = job->result();
        setEventData(source, k);
    }
    setStatusData(source, j);
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

void OcsEngine::setFolderData(const QString& source, const Folder& folder)
{
    Plasma::DataEngine::Data folderData;

    folderData.insert("Id", folder.id());
    folderData.insert("Name", folder.name());
    folderData.insert("Type", folder.type());
    folderData.insert("MessageCount", folder.messageCount());

    setData(source, "Folder-" + folder.id(), folderData);
}

void OcsEngine::setMessageData(const QString& source, const Message& message)
{
    Plasma::DataEngine::Data messageData;

    messageData.insert("Id", message.id());
    messageData.insert("From-Id", message.from());
    messageData.insert("To-Id", message.to());
    messageData.insert("Subject", message.subject());
    messageData.insert("Body", message.body());
    messageData.insert("SendDate", message.sent());
    messageData.insert("Status", message.status() == Message::Answered ? "answered" : message.status() == Message::Read ? "read" : "unread");

    setData(source, "Message-" + message.id(), messageData);
}

void OcsEngine::slotEventListResult(BaseJob* j)
{
    QString source = m_jobs.take(j);
    if (j->metadata().error() == Metadata::NoError) {
        ListJob<Event>* job = static_cast<ListJob<Event>*>(j);

        foreach (const Event& event, job->itemList()) {
            setEventData(source, event);
        }
    }
    setStatusData(source, j);
}


void OcsEngine::slotFolderListResult(BaseJob* j)
{
    QString source = m_jobs.take(j);
    if (j->metadata().error() == Metadata::NoError) {
        ListJob<Folder>* job = static_cast<ListJob<Folder>*>(j);

        foreach (const Folder& folder, job->itemList()) {
            setFolderData(source, folder);
        }
    }
    setStatusData(source, j);
}


void OcsEngine::slotMessageResult(BaseJob* j)
{
    QString source = m_jobs.take(j);
    if (j->metadata().error() == Metadata::NoError) {
        ItemJob<Message>* job = static_cast<ItemJob<Message>*>(j);
        Message message = job->result();
        addToMessageCache(source, message);
        setMessageData(source, message);
    }
    setStatusData(source, j);
}


void OcsEngine::slotMessageListResult(BaseJob* j)
{
    QString source = m_jobs.take(j);
    if (j->metadata().error() == Metadata::NoError) {
        ListJob<Message>* job = static_cast<ListJob<Message>*>(j);

        foreach (const Message& message, job->itemList()) {
            addToMessageCache(source, message);
            setMessageData(source, message);
        }
    }
    setStatusData(source, j);
}


void OcsEngine::providerAdded(const Attica::Provider& provider)
{
    qDebug() << "providerAdded" << provider.baseUrl();

    QString baseUrl = provider.baseUrl().toString();
    if (!m_providers.contains(baseUrl)) {
        m_providers.insert(baseUrl, QSharedPointer<Provider>(new Provider(provider)));

        updateProviderData();

        foreach (QString source, m_sourcesWithoutProvider.value(provider.baseUrl().toString())) {
            sourceRequestEvent(source);
        }
        m_sourcesWithoutProvider.remove(provider.baseUrl().toString());
    }
}


void OcsEngine::updateProviderData()
{
    if (m_providers.size() == 0) {
        // even if no providers are there yet, in order to let the applet connect to us, set dummy data
        setData("Providers", "loading", Plasma::DataEngine::Data());
    } else {
        removeData("Providers", "loading");
    }
    
    foreach(const QSharedPointer<Attica::Provider> &provider, m_providers) {
        Plasma::DataEngine::Data providerData;
        
        providerData.insert("BaseUrl", provider->baseUrl());
        providerData.insert("Name", provider->name());
        
        if (provider->hasCredentials()) {
            QString user;
            QString pass;
            
            provider->loadCredentials(user, pass);
            kDebug() << "credentials found" << user;
            providerData.insert("UserName", user);
        }
        removeData("Providers", provider->baseUrl().toString());
        setData("Providers", provider->baseUrl().toString(), providerData);
    }    
}


void OcsEngine::serviceUpdates(const QString& command)
{
    QStringList commands = command.split(',');
    foreach (const QString& command, commands) {
        foreach (const QString& source, sources()) {
            if (source.startsWith(command + '\\')) {
                qDebug() << "Updating" << source << "- matches:" << command;
                updateSourceEvent(source);
            }
        }
    }
}


#include "ocsengine.moc"

