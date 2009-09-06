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

#include <KDebug>
#include <KIO/NetAccess>
#include <KHTMLPart>
#include <KHTMLView>
#include <DOM/HTMLInputElement>
#include <DOM/HTMLDocument>

RTM::Auth::Auth(RTM::Permissions permissions, const QString& apiKey, const QString& sharedSecret)
  : authPage(0)
{
  frobRequest = new RTM::Request("rtm.auth.getFrob", apiKey, sharedSecret);
  tokenRequest = new RTM::Request("rtm.auth.getToken", apiKey, sharedSecret);
  arguments.insert("perms", getTextPermissions(permissions));
  this->sharedSecret = sharedSecret;
  arguments.insert("api_key", apiKey);
  m_state = RTM::Mutable;
}

RTM::Auth::~Auth() {
  frobRequest->deleteLater();
  tokenRequest->deleteLater();
}


void RTM::Auth::showLoginWebpage()
{
  connect(frobRequest, SIGNAL(replyReceived(RTM::Request*)), SLOT(showLoginWindowInternal()));
  frobRequest->sendRequest();
}


void RTM::Auth::showLoginWindowInternal()
{
  QString reply = frobRequest->readAll();
  frob = reply.remove(0, reply.indexOf("<frob>")+6);
  frob.truncate(frob.indexOf("</frob>"));
  kDebug() << "Frob: " << frob;
  arguments.insert("frob", frob);
  
  
  authPage = new KHTMLPart();

  authPage->openUrl(getAuthUrl());
  
  authPage->widget()->resize(800, 600);
  authPage->widget()->scroll(0, 200);
  authPage->widget()->show();
  
  
  connect(authPage->widget(), SIGNAL(destroyed(QObject*)), SLOT(pageClosed()));
}


void RTM::Auth::pageClosed() {
  disconnect(authPage);
  continueAuthForToken();
}

QString RTM::Auth::getAuthUrl() {
  if (frob.isEmpty())
    kWarning() << "Warning, Frob is EMPTY";
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
      kDebug() << "ERROR: No Permissions";
      break;
  }
  return textPermissions;
}

QString RTM::Auth::requestUrl() {
  kDebug() << "RTM::Auth::getRequestUrl()";
  switch(m_state) {
    case RTM::Mutable:
      sign();
      break;
    case RTM::Hashed:
      break;
    case RTM::RequestSent:
      break;
    case RTM::RequestReceived:
      break;
   }
    QString url = RTM::baseAuthUrl;
    foreach(const QString &key, arguments.keys()) 
      url.append('&' + key + '=' + arguments.value(key));
    return url;
}

void RTM::Auth::continueAuthForToken()
{
  kDebug() << "Token Time";
  tokenRequest->addArgument("frob", arguments.value("frob"));
  QString reply = tokenRequest->sendSynchronousRequest();

  QString token = reply.remove(0, reply.indexOf("<token>")+7);
  token.truncate(token.indexOf("</token>"));
  kDebug() << "Token: " << token;
  emit tokenReceived(token);
}
