#ifndef RTM_REQUEST_P_H
#define RTM_REQUEST_P_H

#include "request.h"
 
#include <QNetworkReply>
#include <QMap>
#include <QtDebug>

#include <KIO/NetAccess>

class RTM::RequestPrivate
{
public:
    RequestPrivate(Request *parent)
    : q(parent),
      m_readOnly(true),
      currentJob(0)
    {
    }

    void dataIncrement(KIO::Job* job, QByteArray data)
    {
      Q_UNUSED(job)
      q->buffer().append(data);
    }
    
    void finished(KJob* job)
    {
      if (job->error()) {
        qDebug() << "Network Job Error: " << job->errorString();
        if (retries >= RTM::RequestPrivate::MAX_RETRIES) {
          qDebug() << "ABORT: Maximum Retries reached for " << currentJob->url();
          currentJob = 0;
          return;
        }
        switch (job->error()) {
          case KIO::ERR_CONNECTION_BROKEN: // If the connection is broken, resend the request
            qDebug() << "Connection Error, retrying connection";
            retries++;
            currentJob = 0;
            q->sendRequest(); 
            return;
          case KIO::ERR_UNKNOWN_HOST: // Guess that we're offline
            qDebug() << "Unknown host, we're probably offline";
            emit q->offlineError();
            q->deleteLater();
            return;
          //TODO: Handle other error cases.
        }
      }
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
    KIO::TransferJob *currentJob;
};

const int RTM::RequestPrivate::MAX_RETRIES = 10;

#endif
