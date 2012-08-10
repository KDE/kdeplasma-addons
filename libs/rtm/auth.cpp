/*
 *   Copyright 2009 Andrew Stromme <astromme@chatonka.com>
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

#include "auth.h"

#include <QHashIterator>
#include <QCryptographicHash>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QCoreApplication>

#include <QtDebug>
#include <QVBoxLayout>

#include "request.h"

class RTM::AuthPrivate {
public:
  AuthPrivate()
    : frobRequest(0),
      tokenRequest(0)
  {
  }
  
  ~AuthPrivate()
  {
    if (frobRequest)
      frobRequest->deleteLater();
    if (tokenRequest)
      tokenRequest->deleteLater();
  }
  
  QString frob;
  Request *frobRequest;
  Request *tokenRequest;

};

RTM::Auth::Auth(RTM::Permissions permissions, const QString& apiKey, const QString& sharedSecret)
: Request(QString(), apiKey, sharedSecret, RTM::baseAuthUrl), 
  d(new RTM::AuthPrivate()) 
{
  d->frobRequest = new RTM::Request("rtm.auth.getFrob", Request::apiKey(), Request::sharedSecret());
  connect(d->frobRequest, SIGNAL(replyReceived(RTM::Request*)), SLOT(onFrobRequestFinished(RTM::Request*)));
  d->frobRequest->sendRequest();

  addArgument("perms", getTextPermissions(permissions));
}

RTM::Auth::~Auth() {
}

void RTM::Auth::onFrobRequestFinished(RTM::Request *reply)
{
  QString data = reply->data();
  d->frob = data.remove(0, data.indexOf("<frob>")+6);
  d->frob.truncate(d->frob.indexOf("</frob>"));
  addArgument("frob", d->frob);
  emit authUrlReady(getAuthUrl());
}

QString RTM::Auth::getAuthUrl() {
  // TODO: don't return until we get a frob?
  while (d->frob.isEmpty())
    QCoreApplication::processEvents();
  return requestUrl();
}

QString RTM::Auth::getTextPermissions(RTM::Permissions permissions)
{
  QString textPermissions;
  switch (permissions)
  {
    case RTM::None:
      textPermissions = "none";
      break;
    case RTM::Read:
      textPermissions = "read";
      break;
    case RTM::Write:
      textPermissions = "write";
      break;
    case RTM::Delete:
      textPermissions = "delete";
      break;
    default:
      qDebug() << "ERROR: No Permissions";
      break;
  }
  return textPermissions;
}

void RTM::Auth::continueAuthForToken()
{
  qDebug() << "Token Time";
  if (d->tokenRequest)
    d->tokenRequest->deleteLater();
  
  d->tokenRequest = new RTM::Request("rtm.auth.getToken", Request::apiKey(), Request::sharedSecret());
  d->tokenRequest->addArgument("frob", d->frob);
  connect(d->tokenRequest, SIGNAL(replyReceived(RTM::Request*)), SLOT(tokenResponse(RTM::Request*)));
  d->tokenRequest->sendRequest();
}


void RTM::Auth::tokenResponse(RTM::Request* response)
{
  QString reply = response->data();
  qDebug() << "Reply: " << reply;
  QString token = reply.remove(0, reply.indexOf("<token>")+7);
  token.truncate(token.indexOf("</token>"));
  qDebug() << "Token: " << token;
  emit tokenReceived(token);
}

