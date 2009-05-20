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

#include <QMapIterator>
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

RTM::Auth::Auth(RTM::Permissions permissions, QString apiKey, QString sharedSecret)
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

void RTM::Auth::login(const QString& authUrl, const QString& username, const QString& password) {
  kDebug() << "Starting Login for user: " << username;
  KIO::Job *job = KIO::get(KUrl(authUrl), KIO::NoReload, KIO::HideProgressInfo);
  QByteArray data;
  KIO::NetAccess::synchronousRun(job, 0, &data);
  job->deleteLater();
  
  authPage = new KHTMLPart();
  authPage->setJScriptEnabled(false);
  authPage->begin(KUrl(authUrl));
  authPage->write(data.constData());
  authPage->end();
  
  //authPage->view()->resize(500, 400);
  //authPage->widget()->show();
  
  m_username = username;
  m_password = password;
  
  connect(authPage, SIGNAL(completed()), SLOT(pageLoaded()));
  connect(authPage->browserExtension(), SIGNAL(openUrlRequestDelayed(const KUrl&, const KParts::OpenUrlArguments&, const KParts::BrowserArguments&)),
          SLOT(pageLoadingReq(const KUrl&, const KParts::OpenUrlArguments&, const KParts::BrowserArguments&))); 
          
  authCount = 0;
}

void RTM::Auth::pageLoaded() {
  kDebug() << "Stage " << authCount;
  
  if (authCount >= 2) {
    disconnect(authPage);
    continueAuthForToken();
    return;
  }
    
  DOM::HTMLInputElement auth = authPage->htmlDocument().getElementById("authorize_yes");
  if (!auth.isNull()) {
    kDebug() << "Entering Stage 2";
    authCount = 2;
    auth.click();
    return;
  }
  
  DOM::HTMLInputElement uname = authPage->htmlDocument().getElementById("username");
  DOM::HTMLInputElement pword = authPage->htmlDocument().getElementById("password");
  DOM::HTMLFormElement form = authPage->htmlDocument().getElementById("loginform");
  
  if (uname.isNull()) {
    authCount = 2;
    disconnect(authPage);
    authPage->deleteLater();
    continueAuthForToken();
    return;
  }
  
  uname.setValue(m_username);
  pword.setValue(m_password);
  
  kDebug() << "Entering Stage 1";
    
  form.submit();
  
  authCount = 1;
}

void RTM::Auth::pageLoadingReq(const KUrl& url, const KParts::OpenUrlArguments& args, const KParts::BrowserArguments& browserArgs)
{
  kDebug() << url;
  authPage->setArguments(args);
  authPage->browserExtension()->setBrowserArguments(browserArgs);
  authPage->openUrl(url);
}

QString RTM::Auth::getAuthUrl() {
  QString reply = frobRequest->sendSynchronousRequest();
  frob = reply.remove(0, reply.indexOf("<frob>")+6);
  frob.truncate(frob.indexOf("</frob>"));
  kDebug() << "Frob: " << frob;
  arguments.insert("frob", frob);
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
    foreach(QString key, arguments.keys()) 
      url.append('&' + key + '=' + arguments.value(key));
    return url;
}

void RTM::Auth::continueAuthForToken()
{
  tokenRequest->addArgument("frob", arguments.value("frob"));
  QString reply = tokenRequest->sendSynchronousRequest();

  QString token = reply.remove(0, reply.indexOf("<token>")+7);
  token.truncate(token.indexOf("</token>"));
  kDebug() << "Token: " << token;
  emit tokenReceived(token);
}
