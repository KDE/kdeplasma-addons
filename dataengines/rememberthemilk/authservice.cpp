/*
 *   Copyright 2009 Andrew Stromme  <astromme@chatonka.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#include "authservice.h"

#include <KDebug>
#include <rtm/session.h>

AuthService::AuthService(RTM::Session* session, QObject* parent)
  : Plasma::Service(parent)
{
    m_session = session;
    setName("rtmauth");
    setOperationEnabled("Login", true);
    setOperationEnabled("AuthWithToken", true);
}

Plasma::ServiceJob* AuthService::createJob(const QString& operation, QMap< QString, QVariant >& parameters) {
    kDebug() << "Creating Job";
    return new AuthJob(m_session, operation, parameters, this);
}


AuthJob::AuthJob(RTM::Session* session, const QString& operation, QMap< QString, QVariant >& parameters, QObject* parent)
  : Plasma::ServiceJob("Auth", operation, parameters, parent),
    m_session(session)
{
  kDebug() << m_session;
}

void AuthJob::start() {
  connect(m_session, SIGNAL(tokenCheck(bool)), SLOT(result(bool)));
  //FIXME: error handling?
  if (operationName() == "Login") {
    m_session->showLoginWindow();
  } 
  else if (operationName() == "AuthWithToken") {
    m_session->setToken(parameters().value("token").toString());
  }
}

void AuthJob::result(bool tokenValid) {
  kDebug() << "TokenValid" << tokenValid;
  setError(false);
  setResult(true);
  
  this->deleteLater();
}

