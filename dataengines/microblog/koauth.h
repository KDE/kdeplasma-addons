/*
 *   Copyright 2012 Sebastian Kügler <sebas@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
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

#ifndef QOAUTHHELPER_H
#define QOAUTHHELPER_H

#include <QObject>
#include <QThread>

#include <QtOAuth/QtOAuth>

#include <KUrl>

#include "oauth.h"

class KOAuthPrivate;

class KOAuth : public QThread
{
Q_OBJECT

public:
    KOAuth(QObject* parent = 0);
    ~KOAuth();

    void sign(KIO::Job *job, const QString &url, OAuth::ParamMap params = OAuth::ParamMap(), OAuth::HttpMethod httpMethod = OAuth::GET);
    void run();

    QString user() const;
    void setUser(const QString &user);

    QString serviceBaseUrl() const;
    void setServiceBaseUrl(const QString &url);

    QString password() const;

    QByteArray accessToken() const;
    QByteArray accessTokenSecret() const;
    bool isAuthorized();

    QByteArray authorizationHeader(const KUrl &requestUrl, QOAuth::HttpMethod method, QOAuth::ParamMap params);

Q_SIGNALS:
    void authorizeApp(const QString &serviceBaseUrl, const QString &authorizeUrl, const QString &pageUrl);
    void accessTokenReceived(const QString &serviceBaseUrl, const QString &accessToken, const QString &accessTokenSecret);
    void authorized(); // We're ready
    void statusUpdated(const QString &serviceBaseUrl, const QString &status, const QString &message = QString());

public Q_SLOTS:
    void appAuthorized(const QString &authorizeUrl, const QString &verifier);
    void authorize(const QString &serviceBaseUrl, const QString &user, const QString &password);

private:
    void init();
    void updateState();
    void requestTokenFromService();
    void accessTokenFromService();
    QString errorMessage(int e);

    KOAuthPrivate* d;
    //QString m_serviceBaseUrl;
};

#endif

