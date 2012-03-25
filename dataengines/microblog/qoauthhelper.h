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

class QOAuthHelperPrivate;

class QOAuthHelper : public QThread
{
Q_OBJECT

public:
    QOAuthHelper(QObject* parent = 0);
    ~QOAuthHelper();

    void sign(KIO::Job *job, const QString &url);
    void run();
    QString user() const;
    QString password() const;

    QByteArray accessToken() const;
    QByteArray accessTokenSecret() const;
    bool isAuthorized();

    void setServiceBaseUrl(const QString &url);
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
    void requestTokenFromService();
    void accessTokenFromService();
    QString errorMessage(int e);

    QOAuthHelperPrivate* d;
    //QString m_serviceBaseUrl;
};

#endif

