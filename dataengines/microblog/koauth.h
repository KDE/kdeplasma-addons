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

namespace KIO {
    class Job;
}

namespace QOAuth {
    typedef QMultiMap<QByteArray,QByteArray> ParamMap;
}

namespace KOAuth {
    // OAuth methods
enum HttpMethod { POST, GET };
enum ParsingMode {
    ParseForRequestContent,     //!< Inline query format (<tt>foo=bar&bar=baz&baz=foo ...</tt>), suitable for POST requests
    ParseForInlineQuery,        /*!< Same as ParseForRequestContent, but prepends the string with a question mark -
                                    suitable for GET requests (appending parameters to the request URL) */
    ParseForHeaderArguments,    //!< HTTP request header format (parameters to be put inside a request header)
    ParseForSignatureBaseString //!< <a href=http://oauth.net/core/1.0/#anchor14>Signature Base String</a> format, meant for internal use.
};

class KOAuthPrivate;

class KOAuth : public QThread
{
Q_OBJECT

public:

    KOAuth(QObject* parent = 0);
    ~KOAuth();

    void init();
    void run();

    static QStringList authorizedAccounts();

    QString user() const;
    void setUser(const QString &user);

    QString identifier() const;

    QString serviceBaseUrl() const;
    void setServiceBaseUrl(const QString &url);

    QString password() const;

    QByteArray accessToken() const;
    QByteArray accessTokenSecret() const;
    bool isAuthorized();

    void sign(KIO::Job *job, const QString &url, QOAuth::ParamMap params = QOAuth::ParamMap(), HttpMethod httpMethod = GET);
    QByteArray userParameters(const QOAuth::ParamMap &parameters);
    QByteArray authorizationHeader(const KUrl &requestUrl, QOAuth::HttpMethod method, QOAuth::ParamMap params);

Q_SIGNALS:
    void authorizeApp(const QString &serviceBaseUrl, const QString &authorizeUrl, const QString &pageUrl);
    void accessTokenReceived(const QString &userName, const QString &serviceBaseUrl, const QString &accessToken, const QString &accessTokenSecret);
    void authorized(); // We're ready
    void statusUpdated(const QString &userName, const QString &serviceBaseUrl, const QString &status, const QString &message = QString());

public Q_SLOTS:
    void appAuthorized(const QString &authorizeUrl, const QString &verifier);
    void authorize(const QString &serviceBaseUrl, const QString &user, const QString &password);
    void forgetAccount(const QString &user, const QString &serviceUrl);

private:
    void updateState();
    void requestTokenFromService();
    void accessTokenFromService();
    QString errorMessage(int e);

    void saveCredentials() const;
    void forgetCredentials() const;
    bool retrieveCredentials() const;
    void configToWallet();

    QByteArray paramsToString(const QOAuth::ParamMap &parameters, ParsingMode mode);
    QByteArray createSignature(const QString &requestUrl, HttpMethod method, const QByteArray &token,
                               const QByteArray &tokenSecret, QOAuth::ParamMap *params);
    void signRequest(KIO::Job *job, const QString &requestUrl, HttpMethod method, const QByteArray &token,
                         const QByteArray &tokenSecret, const QOAuth::ParamMap &params);

    KOAuthPrivate* d;
    //QString m_serviceBaseUrl;
};

} // namespace KOAuth

#endif

