/*
    This file is part of KDE.

    Copyright (c) 2009 Eckhart Wörner <ewoerner@kde.org>

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

#include "personservice.h"

#include <KDebug>

#include <attica/message.h>
#include <attica/postjob.h>
#include "servicejobwrapper.h"


using namespace Attica;

PersonService::PersonService(QSharedPointer<Provider> provider, const QString& id, QSharedPointer<QSignalMapper> serviceUpdates, QObject* parent)
    : Service(parent), m_id(id), m_provider(provider), m_serviceUpdates(serviceUpdates)
{
    setName("ocsPerson");
}


Plasma::ServiceJob* PersonService::createJob(const QString& operation, QMap<QString, QVariant>& parameters)
{
    if (!m_provider) {
        return 0;
    }

    kDebug() << "operation: " << operation << "params: " << parameters;
    if (operation == "sendMessage") {
        Message message;
        message.setTo(m_id);
        message.setSubject(parameters.value("Subject").toString());
        message.setBody(parameters.value("Body").toString());
        return new ServiceJobWrapper(m_provider->postMessage(message), m_id, operation, parameters);
    } else if (operation == "invite") {
        QString message = parameters.value("Message").toString();
        ServiceJobWrapper* job = new ServiceJobWrapper(m_provider->inviteFriend(m_id, message), m_id, operation, parameters);
        m_serviceUpdates.data()->setMapping(job, "SentInvitations");
        connect(job, SIGNAL(finished(KJob*)), m_serviceUpdates.data(), SLOT(map()));
        return job;
    } else if (operation == "approveFriendship") {
        ServiceJobWrapper* job = new ServiceJobWrapper(m_provider->approveFriendship(m_id), m_id, operation, parameters);
        m_serviceUpdates.data()->setMapping(job, "ReceivedInvitations,Friends");
        connect(job, SIGNAL(finished(KJob*)), m_serviceUpdates.data(), SLOT(map()));
        return job;
    } else if (operation == "declineFriendship") {
        ServiceJobWrapper* job = new ServiceJobWrapper(m_provider->declineFriendship(m_id), m_id, operation, parameters);
        m_serviceUpdates.data()->setMapping(job, "ReceivedInvitations");
        connect(job, SIGNAL(finished(KJob*)), m_serviceUpdates.data(), SLOT(map()));
        return job;
    } else if (operation == "cancelFriendship") {
        ServiceJobWrapper* job = new ServiceJobWrapper(m_provider->cancelFriendship(m_id), m_id, operation, parameters);
        m_serviceUpdates.data()->setMapping(job, "Friends");
        connect(job, SIGNAL(finished(KJob*)), m_serviceUpdates.data(), SLOT(map()));
        return job;
    } else if (operation == "setCredentials") {
        kDebug() << "save credentials! " << parameters.value("username").toString() << parameters.value("password").toString();
        m_provider->saveCredentials(parameters.value("username").toString(), parameters.value("password").toString());
        ServiceJobWrapper* job = new ServiceJobWrapper(
            m_provider->checkLogin(parameters.value("username").toString(), parameters.value("password").toString())
            , m_id, operation, parameters);        
        kDebug() << "Set credentials: " << parameters.value("username") << parameters.value("password");
        return job;
    } else {
        return 0;
    }
}
