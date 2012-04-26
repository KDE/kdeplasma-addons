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

#ifndef KOAUTHWEBHELPER_H
#define KOAUTHWEBHELPER_H

#include <QObject>

#include <KUrl>

namespace KOAuth {
class KOAuthWebHelperPrivate;

class KOAuthWebHelper : public QObject
{
Q_OBJECT

public:
    KOAuthWebHelper(QObject* parent = 0);
    ~KOAuthWebHelper();

    void setUser(const QString &user);
    void setPassword(const QString &password);
    void setServiceBaseUrl(const QString &url);
    bool isIdentica();

Q_SIGNALS:
    void appAuthSucceeded(const QString &authorizeUrl, const QString &verifier);
    void statusUpdated(const QString &userName, const QString &serviceBaseUrl, const QString &status, const QString &message = QString());

private Q_SLOTS:
    void loadFinished();
    void showDialog();
    void authorizeApp(const QString &serviceBaseUrl, const QString &authorizeUrl, const QString &pageUrl);

private:
    KOAuthWebHelperPrivate* d;
};


} // namespace

#endif

