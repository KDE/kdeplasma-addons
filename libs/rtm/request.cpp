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

#include "request.h"

#include <QTime>
#include <QMapIterator>
#include <QDateTime>
#include <QCryptographicHash>
#include <QCoreApplication>

#include <KDebug>
#include <KIO/NetAccess>
#include <QTimer>


const int RTM::Request::MAX_RETRIES = 10;

RTM::Request::Request(const QString &method, const QString &apiKey, const QString &sharedSecret)
  : currentJob(0)
{
  arguments.insert("method", method);
  this->sharedSecret = sharedSecret;
  arguments.insert("api_key", apiKey);
  m_state = RTM::Mutable;
  retries = 0;
}

void RTM::Request::addArgument(const QString &name, const QString &value) {
  arguments.insert(name, value);
}

void RTM::Request::sendRequest()
{
  static QDateTime lastRequest;
  
  // Follow RTM's TOS and only do 1 request per second.
  if (lastRequest.secsTo(QDateTime::currentDateTime()) <= 1) {
    QTimer::singleShot(1000, this, SLOT(sendRequest())); 
    //kDebug() << "Postponing Job for 1 second";
    return;
  }
  QString url = requestUrl();
  kDebug() << "Request ready. Url is: " << url;
  currentJob = KIO::get(KUrl(url), KIO::NoReload, KIO::HideProgressInfo);
  connect(currentJob, SIGNAL(data(KIO::Job*,QByteArray)), SLOT(dataIncrement(KIO::Job*,QByteArray)));
  connect(currentJob, SIGNAL(result(KJob*)), this, SLOT(finished(KJob*)));

  lastRequest = QDateTime::currentDateTime();
}

QString RTM::Request::method() const {
  return arguments.value("method");
}

QByteArray RTM::Request::sendSynchronousRequest() {
  KIO::Job *job = KIO::get(KUrl(requestUrl()), KIO::NoReload, KIO::HideProgressInfo);
  job->setAutoDelete(true);
  QByteArray data = this->data();
  KIO::NetAccess::synchronousRun(job, 0, &data);
  return data; // TODO ERROR HANDLING
}

void RTM::Request::dataIncrement(KIO::Job* job, QByteArray data) {
  Q_UNUSED(job)
  //kDebug() << data;
  buffer().append(data);
}

void RTM::Request::finished(KJob* job) {
  if (job->error()) {
    kDebug() << "Network Job Error: " << job->errorString();
    if (retries >= MAX_RETRIES) {
      kDebug() << "ABORT: Maximum Retries reached for " << currentJob->url();
      currentJob = 0;
      return;
    }
    switch (job->error()) {
      case KIO::ERR_CONNECTION_BROKEN: // If the connection is broken, resend the request
        kDebug() << "Connection Error, retrying connection";
        disconnect(currentJob);
        retries++;
        currentJob = 0;
        sendRequest(); 
        return;
      //TODO: Handle other error cases.
    }
  }
  emit (replyReceived(this));
}


void RTM::Request::sign() {
  QString unistring = sharedSecret;
  QMapIterator<QString, QString> i(arguments);
  while (i.hasNext()) {
      i.next();
      unistring.append(i.key());
      unistring.append(i.value());
 }
  QString hash = QCryptographicHash::hash(unistring.toAscii(), QCryptographicHash::Md5).toHex();
  arguments.insert("api_sig", hash);
  m_state = RTM::Hashed;
}

void RTM::Request::unsign() {
  arguments.remove("api_sig");
}

QString RTM::Request::requestUrl()
{
  switch(m_state) {
    case RTM::Mutable:
      sign();
    case RTM::Hashed:
      unsign();
      sign();
      break;
    case RTM::RequestSent:
      break;
    case RTM::RequestReceived:
      break;
   }
    //kDebug() << "Creating url";
    QString url = RTM::baseMethodUrl;
    foreach(const QString &key, arguments.keys()) 
      url.append('&' + key + '=' + arguments.value(key));
    return url;
}

RTM::Request::~Request() {
}
