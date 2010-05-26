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

#include <QtCore/QSignalMapper>
#include <QStringList>
#include <QHash>
#include <QtCore/QSharedPointer>

#include <Solid/Networking>

#include <attica/activity.h>
#include <attica/message.h>
#include <attica/provider.h>
#include <attica/providermanager.h>

#include "personservice.h"

class KJob;

namespace KIO {
    class Job;
}

namespace Attica {
    class BaseJob;
}

/**
 * This class evaluates the basic expressions given in the interface.
 */
class OcsEngine : public Plasma::DataEngine
{
    Q_OBJECT

public:
    OcsEngine ( QObject* parent, const QVariantList& args );
    Plasma::Service* serviceForSource(const QString& source);

protected:
    bool sourceRequestEvent(const QString& name);
    bool updateSourceEvent(const QString& source);

protected Q_SLOTS:
    void slotActivityResult( Attica::BaseJob* j);
    void slotPersonListResult( Attica::BaseJob* j);
    void slotPersonResult( Attica::BaseJob* j );
    void slotKnowledgeBaseResult( Attica::BaseJob* j );
    void slotKnowledgeBaseListResult( Attica::BaseJob* j );
    void slotEventResult( Attica::BaseJob* j);
    void slotEventListResult( Attica::BaseJob* j);
    void slotFolderListResult(Attica::BaseJob* j);
    void slotMessageResult(Attica::BaseJob* j);
    void slotMessageListResult(Attica::BaseJob* j);
    void locationPosted( Attica::BaseJob* j );
    void networkStatusChanged(Solid::Networking::Status);
    void slotPixmapData(KIO::Job* j, const QByteArray& data);
    void slotPixmapResult(KJob* j);

    void providerAdded(const Attica::Provider& provider);
    void serviceUpdates(const QString& command);

private:
    void setPersonData(const QString& source, const Attica::Person& person, bool keyOnly = false);
    void setKnowledgeBaseData(const QString &source, const Attica::KnowledgeBaseEntry &knowledgeBase);
    void setEventData(const QString& source, const Attica::Event& event);
    void setFolderData(const QString& source, const Attica::Folder& folder);
    void setMessageData(const QString& source, const Attica::Message& message);
    void updateProviderData();
    void setStatusData(const QString& source, Attica::BaseJob* job);
    void addToPersonCache(const QString& source, const Attica::Person& person, bool replaceCache = false);
    void addToMessageCache(const QString& source, const Attica::Message& message, bool replaceCache = false);
    static QStringList split(const QString& encodedString);
    static QString encode(const QString& s);
    static QPair<QString, QHash<QString, QString> > parseSource(const QString& source);
    bool providerDependentRequest(const QString& request, const QHash<QString, QString>& arguments, const QString& fullQuery, const QString& baseUrl, Attica::Provider* provider);

    QHash<Attica::BaseJob*, QString> m_jobs;
    QHash<KJob*, QString> m_pixmapJobs;
    QHash<KJob*, QByteArray> m_pixmapData;

    QHash<QPair<QString, QString>, Attica::Person> m_personCache;
    QHash<QPair<QString, QString>, Attica::Message> m_messageCache;
    QHash<QString, QSharedPointer<Attica::Provider> > m_providers;
    QHash<QString, QSet<QString> > m_requestCache;
    //base urls that still did not have a loaded provider when the source was requested
    QHash<QString, QStringList> m_sourcesWithoutProvider;
    Attica::ProviderManager m_pm;
    QSharedPointer<QSignalMapper> m_serviceUpdates;
};

K_EXPORT_PLASMA_DATAENGINE(ocs, OcsEngine )

#endif

