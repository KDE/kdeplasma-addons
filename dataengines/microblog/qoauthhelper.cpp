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

/*
 * QOAuth example
 *      http://blog.ayoy.net/2009/6/24/oauth
 *
 * Twitter OAuth Docs:
 *      https://dev.twitter.com/docs/auth/oauth
 *
 * How does OAuth work diagram:
 *      https://dev.twitter.com/sites/default/files/images_documentation/oauth_diagram.png
 *
 *
 */

#include <QUrl>
#include <KRun>
#include <KDialog>

#include <KWebView>
#include <KIO/AccessManager>
#include <KSharedConfig>
#include <QWebFrame>

#include "oauth.h"
#include "qoauthhelper.h"
#include "qoauthwebhelper.h"
#include <KDebug>
#include <QtOAuth/QtOAuth>

class QOAuthHelperPrivate {

public:
    QOAuthHelperPrivate()
    {
        interface = new QOAuth::Interface();
        busy = false;
    }

    QOAuth::Interface* interface;

    QString user;
    QString password;

    bool busy;

    QString serviceBaseUrl;
    QString requestTokenUrl;
    QString accessTokenUrl;
    QString authorizeUrl;

    QByteArray consumerKey;
    QByteArray consumerSecret;

    QByteArray requestToken;
    QByteArray requestTokenSecret;

    QByteArray accessToken;
    QByteArray accessTokenSecret;

    QString verifier;

    QOAuthWebHelper *w;
};


QOAuthHelper::QOAuthHelper(QObject* parent)
    : QThread(parent),
      d(0)
{

    setObjectName(QLatin1String("QOAuthHelper"));
    init();
}

QByteArray QOAuthHelper::accessToken() const
{
    return d->accessToken;
}

QByteArray QOAuthHelper::accessTokenSecret() const
{
    return d->accessTokenSecret;
}

QString QOAuthHelper::user() const
{
    return d->user;
}

QString QOAuthHelper::serviceBaseUrl() const
{
    return d->serviceBaseUrl;
}

void QOAuthHelper::setUser(const QString& user)
{
    if (user == d->user) {
        return;
    }
    d->user = user;
    updateState();
}

QString QOAuthHelper::password() const
{
    return d->password;
}

void QOAuthHelper::init()
{
    if (!d) {
        d = new QOAuthHelperPrivate;
#ifndef NO_KIO
        KIO::AccessManager *access = new KIO::AccessManager(this);
        d->interface->setNetworkAccessManager(access);
#endif
        d->w = new QOAuthWebHelper();
        connect(this, SIGNAL(authorizeApp(const QString&, const QString&, const QString&)),
                d->w, SLOT(authorizeApp(const QString&, const QString&, const QString&)));
        connect(d->w, SIGNAL(appAuthSucceeded(const QString&, const QString&)),
                this, SLOT(appAuthorized(const QString&, const QString&)));
        connect(d->w, SIGNAL(statusUpdated(const QString&, const QString&, const QString&)),
                SIGNAL(statusUpdated(const QString&, const QString&, const QString&)));

    }

}

void QOAuthHelper::run()
{
    if (isAuthorized()) {
        emit statusUpdated(d->serviceBaseUrl, "Ok", "User authorized");
    } else {
        //authorize(d->serviceBaseUrl, d->user, d->password);
    }
}

void QOAuthHelper::authorize(const QString &serviceBaseUrl, const QString &user, const QString &password)
{
    if (d->busy || isAuthorized()) {
        return;
    }
    d->user = user;
    d->w->setUser(user);
    d->w->setServiceBaseUrl(serviceBaseUrl);
    d->w->setPassword(password);
    d->password = password;
    d->serviceBaseUrl = serviceBaseUrl;

    //run();
    if (!isAuthorized()) {
        d->busy = true;
        requestTokenFromService();
    }
}

void QOAuthHelper::requestTokenFromService()
{
    d->interface->setConsumerKey(d->consumerKey);
    d->interface->setConsumerSecret(d->consumerSecret);

    d->interface->setRequestTimeout( 10000 );

    QOAuth::ParamMap params;
    params.insert("oauth_callback", "oob");
//     kDebug() << "starting token request ...";
    QOAuth::ParamMap reply = d->interface->requestToken(d->requestTokenUrl,
                                                        QOAuth::GET, QOAuth::HMAC_SHA1, params);
//     kDebug() << "token request done......" << reply;

    QString e;
    if ( d->interface->error() == QOAuth::NoError ) {
        d->requestToken = reply.value(QOAuth::tokenParameterName());
        d->requestTokenSecret = reply.value(QOAuth::tokenSecretParameterName());

        QString auth_url = QString("%1?oauth_token=%2").arg(d->authorizeUrl, QString(d->requestToken));

        emit statusUpdated(d->serviceBaseUrl, "Busy", "Request token received.");
        emit authorizeApp(d->serviceBaseUrl, d->authorizeUrl, auth_url);

    } else {
        e += errorMessage(d->interface->error());
        kDebug() << "Request Token returned error:" << e;
        emit statusUpdated(d->serviceBaseUrl, "Error", "Request Token Error: " + e);
        d->busy = false;

    }
}

void QOAuthHelper::appAuthorized(const QString &authorizeUrl, const QString &verifier)
{
    Q_UNUSED(authorizeUrl);
    d->verifier = verifier;
    accessTokenFromService();
}

QString QOAuthHelper::errorMessage(int e) {
    //     enum ErrorCode {
    //         NoError = 200,              //!< No error occured (so far :-) )
    //         BadRequest = 400,           //!< Represents HTTP status code \c 400 (Bad Request)
    //         Unauthorized = 401,         //!< Represents HTTP status code \c 401 (Unauthorized)
    //         Forbidden = 403,            //!< Represents HTTP status code \c 403 (Forbidden)
    //         Timeout = 1001,             //!< Represents a request timeout error
    //         ConsumerKeyEmpty,           //!< Consumer key has not been provided
    //         ConsumerSecretEmpty,        //!< Consumer secret has not been provided
    //         UnsupportedHttpMethod,      /*!< The HTTP method is not supported by the request.
    //                                          \note \ref QOAuth::Interface::requestToken() and
    //                                          \ref QOAuth::Interface::accessToken()
    //                                          accept only HTTP GET and POST requests. */
    //
    //         RSAPrivateKeyEmpty = 1101,  //!< RSA private key has not been provided
    //         //    RSAPassphraseError,         //!< RSA passphrase is incorrect (or has not been provided)
    //         RSADecodingError,           /*!< There was a problem decoding the RSA private key
    //                                      (the key is invalid or the provided passphrase is incorrect)*/
    //         RSAKeyFileError,            //!< The provided key file either doesn't exist or is unreadable.
    //         OtherError                  //!< A network-related error not specified above
    //     };
    //
    QString out;
    if (e == QOAuth::BadRequest) {
        out.append("Bad request");
    } else if (e == QOAuth::Unauthorized) {
        out.append("Unauthorized");
    } else if (e == QOAuth::Forbidden) {
        out.append("Forbidden");
    } else if (e == QOAuth::Timeout) {
        out.append("Timeout");
    } else if (e == QOAuth::ConsumerKeyEmpty) {
        out.append("ConsumerKeyEmpty");
    } else if (e == QOAuth::ConsumerSecretEmpty) {
        out.append("ConsumerSecretEmpty");
    } else if (e == QOAuth::UnsupportedHttpMethod) {
        out.append("UnsupportedHttpMethod");
    } else if (e == QOAuth::UnsupportedHttpMethod) {
        out.append("ConsumerSecretEmpty");
    } else {
        out.append("Other error." + e);
    }
    return out;
}

void QOAuthHelper::accessTokenFromService()
{
//     kDebug() << "start ... accessToken. TODO insert verifier" << d->verifier;
    QOAuth::ParamMap params = QOAuth::ParamMap();
    params.insert("oauth_callback", "oob");
    if (d->serviceBaseUrl.toLower().contains("identi.ca")) {
        params.insert("oauth_verifier", d->verifier.toLocal8Bit());
    }
    QOAuth::ParamMap reply = d->interface->accessToken(d->accessTokenUrl, QOAuth::GET,
                                                       d->requestToken, d->requestTokenSecret,
                                                       QOAuth::HMAC_SHA1, params);
//      kDebug() << "end ...... accessToken";
//     kDebug() << " MAP: " << params;
    QString e;
    if ( d->interface->error() == QOAuth::NoError ) {
        d->accessToken = reply.value(QOAuth::tokenParameterName());
        d->accessTokenSecret = reply.value(QOAuth::tokenSecretParameterName());

        //QString auth_url = QString("%1?oauth_token=%2").arg(d->authorizeUrl, QString(d->requestToken));
//         kDebug() << "Received Access Token OK!" << d->accessToken << d->accessTokenSecret;
        //kDebug() << "Surf to: " << auth_url;
        emit accessTokenReceived(d->serviceBaseUrl, d->accessToken, d->accessTokenSecret);
        d->busy = false;
        KSharedConfigPtr ptr = KSharedConfig::openConfig("oauthrc");
        KConfigGroup config = KConfigGroup(ptr, d->user+"@"+d->serviceBaseUrl);
        config.writeEntry("accessToken", d->accessToken);
        config.writeEntry("accessTokenSecret", d->accessTokenSecret);
        config.sync();

        emit authorized();
    } else {
        kDebug() << d->interface->error() << reply;
        e += errorMessage(d->interface->error());
        kDebug() << "Request Not working" << e;
        emit statusUpdated(d->serviceBaseUrl, "Error", "Access Token Error:" + e);
        d->busy = false;
    }
}

void QOAuthHelper::setServiceBaseUrl(const QString &serviceBaseUrl)
{
    if (d->serviceBaseUrl == serviceBaseUrl) {
        return;
    }
    d->serviceBaseUrl = serviceBaseUrl;
    updateState();
}

void QOAuthHelper::updateState()
{
    const QUrl u(d->serviceBaseUrl);
//     kDebug() << "set service " << u << u.host();
    if (u.host() == "twitter.com") {
//         kDebug() << "Using twitter...";
        d->requestTokenUrl = "https://api.twitter.com/oauth/request_token";
        d->accessTokenUrl = "https://api.twitter.com/oauth/access_token";
        d->authorizeUrl = "https://api.twitter.com/oauth/authorize";
        d->consumerKey = "22kfJkztvOqb8WfihEjdg";
        d->consumerSecret = "RpGc0q0aGl0jMkeqMIawUpGyDkJ3DNBczFUyIQMR698";

    } else {
//         kDebug() << "Using identi.ca...";
        d->requestTokenUrl = "https://identi.ca/api/oauth/request_token";
        d->accessTokenUrl = "https://identi.ca/api/oauth/access_token";
        d->authorizeUrl = "https://identi.ca/api/oauth/authorize";
        d->consumerKey = "47a4650a6bd4026b1c4d55d641acdb64";
        d->consumerSecret = "49208b0a87832f4279f9d3742c623910";
    }

    // Reset other data, we need to re-retrieve it
    //d->requestToken = QByteArray();
    //d->requestTokenSecret = QByteArray();

    if (!d->user.isEmpty() && !d->serviceBaseUrl.isEmpty()) {
        KSharedConfigPtr ptr = KSharedConfig::openConfig("oauthrc");
//         kDebug() << "::: oauth reads from Config: " << d->user+"@"+d->serviceBaseUrl;
        KConfigGroup config = KConfigGroup(ptr, d->user+"@"+d->serviceBaseUrl);
        d->accessToken = config.readEntry("accessToken", QByteArray());
        d->accessTokenSecret = config.readEntry("accessTokenSecret", QByteArray());
        //d->accessToken = QByteArray();
        //d->accessTokenSecret = QByteArray();
//         kDebug() << " CONFIG " << d->accessToken << d->accessTokenSecret;
        if (isAuthorized()) {
            emit accessTokenReceived(d->serviceBaseUrl, d->accessToken, d->accessTokenSecret);
            d->busy = false;
        }
    }
}

QOAuthHelper::~QOAuthHelper()
{
    delete d;
}

bool QOAuthHelper::isAuthorized()
{
//     kDebug() << " hmm? " << d->accessToken << d->accessTokenSecret;
//     kDebug() << d->accessToken.isEmpty() << !d->accessTokenSecret.isEmpty() << " -> " << (!d->accessToken.isEmpty() && !d->accessTokenSecret.isEmpty());
    return !d->accessToken.isEmpty() && !d->accessTokenSecret.isEmpty();
}


QByteArray QOAuthHelper::authorizationHeader(const KUrl &requestUrl, QOAuth::HttpMethod method, QOAuth::ParamMap params)
{
    QByteArray auth;
    auth = d->interface->createParametersString(requestUrl.url(), method,
                                                d->accessToken, d->accessTokenSecret,
                                                QOAuth::HMAC_SHA1, params,
                                                QOAuth::ParseForHeaderArguments);
    return auth;
}

void QOAuthHelper::sign(KIO::Job *job, const QString &url, OAuth::ParamMap params, OAuth::HttpMethod httpMethod)
{
    OAuth::signRequest(job, url, httpMethod, accessToken(), accessTokenSecret(), params);
}

#include "qoauthhelper.moc"
