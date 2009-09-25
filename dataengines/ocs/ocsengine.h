/*
 *   Copyright (C) 2008 Dirk Mueller <mueller@kde.org>
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

#ifndef OCSENGINE_H
#define OCSENGINE_H

#include "plasma/dataengine.h"

#include <QStringList>
#include <QHash>

#include <Solid/Networking>

#include "activity.h"
#include "personservice.h"
#include "provider.h"

class QTimer;
class KJob;

namespace Attica {
    class PersonJob;
    class KnowledgeBaseEntryJob;
    class Event;
}

/**
 * This class evaluates the basic expressions given in the interface.
 */
class OcsEngine : public Plasma::DataEngine
{
    Q_OBJECT

public:
    OcsEngine ( QObject* parent, const QVariantList& args );
    ~OcsEngine ();
    virtual QStringList sources() const;
    virtual Plasma::Service* serviceForSource(const QString& source);

protected:
    virtual bool sourceRequestEvent(const QString& name);
    virtual bool updateSourceEvent(const QString& source);

protected Q_SLOTS:
    virtual void slotActivityResult( KJob* j);
    virtual void slotFriendsResult( KJob* j);
    virtual void slotNearPersonsResult( KJob* j);
    virtual void slotPersonResult( KJob* j);
    virtual void slotKnowledgeBaseResult( KJob *j );
    virtual void slotKnowledgeBaseListResult( KJob *j );
    virtual void slotEventResult(KJob* j);
    virtual void slotEventListResult(KJob* j);
    virtual void locationPosted( KJob *j );
    void networkStatusChanged(Solid::Networking::Status);
    void initializeProvider(KJob* j);

private:
    void setPersonData(const QString &source, const Attica::Person &person);
    void setKnowledgeBaseData(const QString &source, const Attica::KnowledgeBaseEntry &knowledgeBase);
    void setEventData(const QString& source, const Attica::Event& event);
    void addToPersonCache(const QString& id, const Attica::Person& person, bool replaceCache = false);
    bool cacheRequest(const QString& query);

    QHash<KJob*, QString> m_eventListJobs;
    QHash<KJob*, QString> m_knowledgeBaseListJobs;
    QHash<KJob*, QString> m_personListJobs;

    Attica::Activity::List m_activities;
    KJob* m_job;
    int m_maximumItems;
    QHash<QString,PersonService*> m_personServices;
    QHash<QString, Attica::Person> m_personCache;
    Attica::Provider m_provider;
    bool m_providerInitialized;
    QSet<QString> m_requestCache;
};

K_EXPORT_PLASMA_DATAENGINE(attica, OcsEngine )

#endif

