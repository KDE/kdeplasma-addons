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
#include <QWebView>
#include <QPushButton>

#include <KDebug>
#include <KIO/NetAccess>
#include <QVBoxLayout>
#include <KLocale>

RTM::Auth::Auth(RTM::Permissions permissions, const QString& apiKey, const QString& sharedSecret)
  : frobRequest(0),
  tokenRequest(0)
{
  arguments.insert("perms", getTextPermissions(permissions));
  this->apiKey = apiKey;
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
  if (frobRequest)
    frobRequest->deleteLater();
  
  frobRequest = new RTM::Request("rtm.auth.getFrob", apiKey, sharedSecret);
  connect(frobRequest, SIGNAL(replyReceived(RTM::Request*)), SLOT(showLoginWindowInternal(RTM::Request*)));
  frobRequest->sendRequest();
}


void RTM::Auth::showLoginWindowInternal(RTM::Request *rawReply)
{
  QString reply = rawReply->data();
  //QString reply = rawReply->readAll(); //FIXME: I have no idea why this line isn't working?
  frob = reply.remove(0, reply.indexOf("<frob>")+6);
  frob.truncate(frob.indexOf("</frob>"));
  kDebug() << "Frob: " << frob;
  arguments.insert("frob", frob);
  
  
  QWidget *authWidget = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout(authWidget);
  QPushButton *button = new QPushButton(authWidget);
  QWebView *authPage  = new QWebView(authWidget);
  
  button->setText(i18n("Click here after you've logged in and authorized the applet"));

  authPage->setUrl(getAuthUrl());
  
  authPage->resize(800, 600);
  authPage->scroll(0, 200);
  
  layout->addWidget(authPage);
  layout->addWidget(button);
  
 
  connect(button, SIGNAL(clicked(bool)), authWidget, SLOT(hide()));
  connect(button, SIGNAL(clicked(bool)), authWidget, SLOT(deleteLater()));
  connect(button, SIGNAL(clicked(bool)), SLOT(pageClosed())); // Last because it takes more time.
  
  authWidget->show();
}


void RTM::Auth::pageClosed() {
  continueAuthForToken();
}

QString RTM::Auth::getAuthUrl() {
  if (frob.isEmpty()) {
    kWarning() << "Warning, Frob is EMPTY";
  }
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
  kDebug() << "RTM::Auth::getRequestUrl()" << m_state << RTM::Mutable;
  switch(m_state) {
    case RTM::Mutable:
      sign();
      break;
    case RTM::Hashed:
      unsign();
      sign();
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
  if (tokenRequest)
    tokenRequest->deleteLater();
  
  tokenRequest = new RTM::Request("rtm.auth.getToken", apiKey, sharedSecret);
  tokenRequest->addArgument("frob", arguments.value("frob"));
  connect(tokenRequest, SIGNAL(replyReceived(RTM::Request*)), SLOT(tokenResponse(RTM::Request*)));
  tokenRequest->sendRequest();
}


void RTM::Auth::tokenResponse(RTM::Request* response)
{
  QString reply = response->data();
  kDebug() << "Reply: " << reply;
  QString token = reply.remove(0, reply.indexOf("<token>")+7);
  token.truncate(token.indexOf("</token>"));
  kDebug() << "Token: " << token;
  emit tokenReceived(token);
}

