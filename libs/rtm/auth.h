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

#ifndef RTM_AUTH_H
#define RTM_AUTH_H

#include <QString>
#include <QObject>
#include <QHash>
#include <QBuffer>
#include <QNetworkReply>

#include <KParts/BrowserExtension>

#include "request.h"

class QWebView;


namespace RTM {

class RTM_EXPORT Auth : public Request
{
Q_OBJECT
  public:
    Auth(RTM::Permissions permissions, const QString &apiKey, const QString &sharedSecret);
    void showLoginWebpage();
    QString getAuthUrl();
    void continueAuthForToken();

    ~Auth();
  protected:
    QString getTextPermissions(RTM::Permissions permissions);
    QString requestUrl();
    QString frob;
    QString apiKey;
    Request *frobRequest;
    Request *tokenRequest;

  signals:
    void authUrlReady(QString authUrl);
    void tokenReceived(QString token);

  protected slots:
    void pageClosed();
    void showLoginWindowInternal(RTM::Request* rawReply);
public slots:
    void tokenResponse(RTM::Request*);
};

} // Namespace RTM

#endif
