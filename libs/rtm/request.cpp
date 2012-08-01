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

#include <QtDebug>
#include <KIO/NetAccess>
#include <QTimer>


class RTM::RequestPrivate
{
public:
    RequestPrivate()
    : m_readOnly(true),
      currentJob(0)
    {
    }
	
    QMap<QString,QString> arguments;
    QString m_response;

    QString baseUrl;
    int retries;
    static const int MAX_RETRIES;
    bool m_readOnly;
	
    RTM::State m_state;
    QString apiKey;
    QString sharedSecret;
    KIO::TransferJob* currentJob;
};

const int RTM::RequestPrivate::MAX_RETRIES = 10;

RTM::Request::Request()
: d(new RTM::RequestPrivate())
{
}

RTM::Request::Request(const QString &method, const QString &apiKey, const QString &sharedSecret, const QString&baseUrl)
: d(new RTM::RequestPrivate())
{
  if (!method.isEmpty())
    d->arguments.insert("method", method);
  d->baseUrl = baseUrl;
  d->apiKey = apiKey;
  d->sharedSecret = sharedSecret;
  d->arguments.insert("api_key", apiKey);
  d->m_state = RTM::Mutable;
  d->retries = 0;
}

QString RTM::Request::response() const 
{ 
	return d->m_response; 
}

RTM::State RTM::Request::state() const 
{ 
	return d->m_state; 
}

void RTM::Request::setReadOnly(bool readOnly) 
{ 
	d->m_readOnly = readOnly; 
}

bool RTM::Request::readOnly() const 
{ 
	return d->m_readOnly; 
}

void RTM::Request::addArgument(const QString &name, const QString &value) {
  d->arguments.insert(name, value);
}

void RTM::Request::sendRequest()
{
  static QDateTime lastRequest = QDateTime::fromMSecsSinceEpoch(QDateTime::currentMSecsSinceEpoch() - 1050);
  static unsigned int queueSize = 0;

  qint64 margin = lastRequest.msecsTo(QDateTime::currentDateTime());

  // Follow RTM's TOS and only do 1 request per second.
  if (margin <= 1000) {
    const int timeout = 1000 * (queueSize + 1) - margin + queueSize * 2 + 1;
    QTimer::singleShot(timeout, this, SLOT(sendRequest())); 
    //qDebug() << "Postponing Job for"<<timeout<<"ms";
    ++queueSize;
    return;
  }
  queueSize = 0;
  QString url = requestUrl();
  qDebug() << "Request ready. Url is: " << url;
  d->currentJob = KIO::get(KUrl(url.toUtf8()), KIO::NoReload, KIO::HideProgressInfo);
  connect(d->currentJob, SIGNAL(data(KIO::Job*,QByteArray)), SLOT(dataIncrement(KIO::Job*,QByteArray)));
  connect(d->currentJob, SIGNAL(result(KJob*)), this, SLOT(finished(KJob*)));

  lastRequest = QDateTime::currentDateTime();
}

QString RTM::Request::method() const {
  return d->arguments.value("method");
}

void RTM::Request::dataIncrement(KIO::Job* job, QByteArray data) {
  Q_UNUSED(job)
  //qDebug() << data;
  buffer().append(data);
}

void RTM::Request::finished(KJob* job) {
  if (job->error()) {
    qDebug() << "Network Job Error: " << job->errorString();
    if (d->retries >= RTM::RequestPrivate::MAX_RETRIES) {
      qDebug() << "ABORT: Maximum Retries reached for " << d->currentJob->url();
      d->currentJob = 0;
      return;
    }
    switch (job->error()) {
      case KIO::ERR_CONNECTION_BROKEN: // If the connection is broken, resend the request
        qDebug() << "Connection Error, retrying connection";
        disconnect(d->currentJob);
        d->retries++;
        d->currentJob = 0;
        sendRequest(); 
        return;
      case KIO::ERR_UNKNOWN_HOST: // Guess that we're offline
        qDebug() << "Unknown host, we're probably offline";
        emit offlineError();
        this->deleteLater();
        return;
      //TODO: Handle other error cases.
    }
  }
  emit (replyReceived(this));
}


void RTM::Request::sign() {
  QString unistring = d->sharedSecret;
  QMapIterator<QString, QString> i(d->arguments);
  while (i.hasNext()) {
      i.next();
      unistring.append(i.key());
      unistring.append(i.value());
 }

  QString hash = QCryptographicHash::hash(unistring.toUtf8(), QCryptographicHash::Md5).toHex();
  d->arguments.insert("api_sig", hash);
  d->m_state = RTM::Hashed;
}

void RTM::Request::unsign() {
  d->arguments.remove("api_sig");
}

QString RTM::Request::requestUrl()
{
  switch(d->m_state) {
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
    //qDebug() << "Creating url";
    QString url = d->baseUrl;
    foreach(const QString &key, d->arguments.keys()) 
      url.append('&' + key + '=' + d->arguments.value(key).toUtf8().toPercentEncoding());
    return url;
}

QString RTM::Request::apiKey() const
{
    return d->apiKey;
}

QString RTM::Request::sharedSecret() const
{
    return d->sharedSecret;
}

RTM::Request::~Request() {
}
