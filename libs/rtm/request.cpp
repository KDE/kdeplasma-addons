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
#include "request_p.h"

#include <QTime>
#include <QMapIterator>
#include <QDateTime>
#include <QCryptographicHash>
#include <QCoreApplication>
#include <QStringList>

#include <QtDebug>
#include <QTimer>

RTM::Request::Request()
: d(new RTM::RequestPrivate(this))
{
}

RTM::Request::Request(const QString &method, const QString &apiKey, const QString &sharedSecret, const QString&baseUrl)
: d(new RTM::RequestPrivate(this))
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
  d->accessManager->get(QNetworkRequest(url));

  lastRequest = QDateTime::currentDateTime();
}

QString RTM::Request::method() const {
  return d->arguments.value("method");
}

void RTM::Request::sign() {
  QString unistring = d->sharedSecret;
  QStringList keys = d->arguments.keys();
  qSort(keys);
  
  Q_FOREACH (const QString& key, keys) {
      unistring.append(key);
      unistring.append(d->arguments.value(key));
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
    //qDebug() << "Creating url";
    QString url = d->baseUrl;
    QStringList keys = d->arguments.keys();
    foreach (const QString &key, keys) 
      url.append('&' + key + '=' + d->arguments.value(key).toUtf8());
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

#include "moc_request.cpp"

