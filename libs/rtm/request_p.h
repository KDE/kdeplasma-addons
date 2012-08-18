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
