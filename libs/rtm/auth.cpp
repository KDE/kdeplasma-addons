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
#include <KWebView>
#include <QPushButton>

#include <KDebug>
#include <KIO/NetAccess>
#include <QVBoxLayout>
#include <KLocale>
#include <KWebView>

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
  addArgument("perms", getTextPermissions(permissions));
}

RTM::Auth::~Auth() {
}


void RTM::Auth::showLoginWebpage()
{
  if (d->frobRequest)
    d->frobRequest->deleteLater();
  
  d->frobRequest = new RTM::Request("rtm.auth.getFrob", Request::apiKey(), Request::sharedSecret());
  connect(d->frobRequest, SIGNAL(replyReceived(RTM::Request*)), SLOT(showLoginWindowInternal(RTM::Request*)));
  d->frobRequest->sendRequest();
}


void RTM::Auth::showLoginWindowInternal(RTM::Request *rawReply)
{
  QString reply = rawReply->data(); // Get the full data of the reply, readAll() doesn't guarentee that.
  d->frob = reply.remove(0, reply.indexOf("<frob>")+6);
  d->frob.truncate(d->frob.indexOf("</frob>"));
  addArgument("frob", d->frob);
  
  
  QWidget *authWidget = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout(authWidget);
  QPushButton *button = new QPushButton(authWidget);
  KWebView *authPage  = new KWebView(authWidget);
  
  button->setText(i18n("Click here after you have logged in and authorized the applet"));

  QUrl authUrl = getAuthUrl();
  authPage->setUrl(authUrl);
  
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
  if (d->frob.isEmpty())
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

void RTM::Auth::continueAuthForToken()
{
  kDebug() << "Token Time";
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
  kDebug() << "Reply: " << reply;
  QString token = reply.remove(0, reply.indexOf("<token>")+7);
  token.truncate(token.indexOf("</token>"));
  kDebug() << "Token: " << token;
  emit tokenReceived(token);
}

