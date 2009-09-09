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

#include "messagesendservicejob.h"

#include "message.h"
#include "provider.h"


using namespace Attica;


MessageSendServiceJob::MessageSendServiceJob(const Provider& provider, const QString& destination, const QString& operation, const QMap<QString, QVariant>& parameters, QObject* parent)
    : ServiceJob(destination, operation, parameters, parent), m_provider(provider)
{
}


MessageSendServiceJob::~MessageSendServiceJob()
{
    delete m_job;
}


void MessageSendServiceJob::start()
{
    QMap<QString, QVariant> params = parameters();
    Message message;
    message.setTo(destination());
    message.setSubject(params["Subject"].toString());
    message.setBody(params["Body"].toString());
    
    m_job = m_provider.postMessage(message);
    connect(m_job, SIGNAL(result(KJob*)), this, SLOT(result(KJob*)));
}


void MessageSendServiceJob::result(KJob* job)
{
    setError(job->error());
    setErrorText(job->errorText());
    setResult(!job->error());
}


#include "messagesendservicejob.moc"
