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

#ifndef RTM_REQUEST_H
#define RTM_REQUEST_H

#include <QBuffer>

#include "rtm.h"

class QNetworkReply;

namespace RTM {

class RequestPrivate;
  
class RTM_EXPORT Request : public QBuffer
{
Q_OBJECT
  public:
    Request(const QString &method, 
            const QString &apiKey, 
            const QString &sharedSecret, 
            const QString &baseUrl = RTM::baseMethodUrl);
    ~Request();
    
    void addArgument(const QString &name, const QString &value);

    QString response() const;
    RTM::State state() const;

    void setReadOnly(bool readOnly);
    bool readOnly() const;

    virtual QString requestUrl(); // Convenience
    QString method() const;

  signals:
    void replyReceived(RTM::Request *request);
    void offlineError();

  public slots:
    void sendRequest();

  protected:
    Request();
    void sign();
    void unsign();
    QString apiKey() const;
    QString sharedSecret() const;

  private:
    friend class RequestPrivate;
    RequestPrivate * const d;
    
    Q_PRIVATE_SLOT(d, void finished(QNetworkReply*));
};

} // namespace RTM

#endif
