/*
 *   Copyright 2009 Andrew Stromme <astromme@chatonka.com>
 *   Copyright 2012 Jeremy Whiting <jpwhiting@kde.org>
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

#ifndef RTM_REQUEST_P_H
#define RTM_REQUEST_P_H

#include "request.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMap>
#include <QtDebug>

class RTM::RequestPrivate
{
public:
    RequestPrivate(Request *parent)
    : q(parent),
      m_readOnly(true),
      accessManager(new QNetworkAccessManager(q))
    {
        QObject::connect(accessManager, SIGNAL(finished(QNetworkReply*)),
                         q, SLOT(finished(QNetworkReply*)));
    }

    void finished(QNetworkReply* reply)
    {
      if (reply->error()) {
        qDebug() << "Network Error: " << reply->error();
        if (retries >= RTM::RequestPrivate::MAX_RETRIES) {
          qDebug() << "ABORT: Maximum Retries reached for " << reply->url();
          return;
        }
        switch (reply->error()) {
          case QNetworkReply::TimeoutError:
          default:
          // If the connection is broken, resend the request
            qDebug() << "Connection Error, retrying connection";
            retries++;
            q->sendRequest(); 
            return;
          case QNetworkReply::HostNotFoundError: // Guess that we're offline
            qDebug() << "Unknown host, we're probably offline";
            emit q->offlineError();
            q->deleteLater();
            return;
          //TODO: Handle other error cases.
        }
      }
      q->buffer().append(reply->readAll());
      emit q->replyReceived(q);
    }

    RTM::Request *q;
    QMap<QString,QString> arguments;
    QString m_response;

    QString baseUrl;
    int retries;
    static const int MAX_RETRIES;
    bool m_readOnly;

    RTM::State m_state;
    QString apiKey;
    QString sharedSecret;
    QNetworkAccessManager *accessManager;
};

const int RTM::RequestPrivate::MAX_RETRIES = 10;

#endif
