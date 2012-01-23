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
#include <QtOAuth/QtOAuth>

#include <KUrl>

class QOAuthHelperPrivate;

class QOAuthHelper : public QObject
{
Q_OBJECT

public:
    QOAuthHelper(QObject* parent = 0);
    ~QOAuthHelper();

    void authorize();

    void setServiceBaseUrl(const QString &url);
    QByteArray authorizationHeader(const KUrl &requestUrl, QOAuth::HttpMethod method, QOAuth::ParamMap params);

private Q_SLOTS:
    void appAuthorized();

private:
    void requestTokenFromService();
    void accessTokenFromService();
    QString errorMessage(int e);

    QOAuthHelperPrivate* d;
};

#endif

