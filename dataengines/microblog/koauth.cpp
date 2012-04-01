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
#include <KIO/Job>

#include <KWebView>
#include <KIO/AccessManager>
#include <KSharedConfig>
#include <QWebFrame>

#include "koauth.h"
#include "qoauthwebhelper.h"
#include <KDebug>
#include <QtOAuth/QtOAuth>

namespace OAuth {

// For twitter
const QByteArray ConsumerKey = "22kfJkztvOqb8WfihEjdg";
const QByteArray ConsumerSecret = "RpGc0q0aGl0jMkeqMIawUpGyDkJ3DNBczFUyIQMR698";

// identi.ca
//const QByteArray ConsumerKey = "47a4650a6bd4026b1c4d55d641acdb64";
//const QByteArray ConsumerSecret = "49208b0a87832f4279f9d3742c623910";

class KOAuthPrivate {

public:
    KOAuthPrivate()
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


KOAuth::KOAuth(QObject* parent)
    : QThread(parent),
      d(0)
{

    setObjectName(QLatin1String("KOAuth"));
    init();
}

QByteArray KOAuth::accessToken() const
{
    return d->accessToken;
}

QByteArray KOAuth::accessTokenSecret() const
{
    return d->accessTokenSecret;
}

QString KOAuth::user() const
{
    return d->user;
}

QString KOAuth::serviceBaseUrl() const
{
    return d->serviceBaseUrl;
}

void KOAuth::setUser(const QString& user)
{
    if (user == d->user) {
        return;
    }
    d->user = user;
    updateState();
}

QString KOAuth::password() const
{
    return d->password;
}

void KOAuth::init()
{
    if (!d) {
        d = new KOAuthPrivate;
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

void KOAuth::run()
{
    if (isAuthorized()) {
        emit statusUpdated(d->serviceBaseUrl, "Ok", "User authorized");
    } else {
        //authorize(d->serviceBaseUrl, d->user, d->password);
    }
}

void KOAuth::authorize(const QString &serviceBaseUrl, const QString &user, const QString &password)
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

void KOAuth::requestTokenFromService()
{
    d->interface->setConsumerKey(d->consumerKey);
    d->interface->setConsumerSecret(d->consumerSecret);

    d->interface->setRequestTimeout( 10000 );
    if (!QCA::isSupported("hmac(sha1)")) {
        kError() << "Hashing algo not supported, update your QCA";
        return;
    }

    ParamMap params;
    params.insert("oauth_callback", "oob");
//     kDebug() << "starting token request ...";
    ParamMap reply = d->interface->requestToken(d->requestTokenUrl,
                                                        QOAuth::GET, QOAuth::HMAC_SHA1, params);
//     kDebug() << "token request done......" << reply;

    QString e;
    if (d->interface->error() == QOAuth::NoError) {
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

void KOAuth::appAuthorized(const QString &authorizeUrl, const QString &verifier)
{
    Q_UNUSED(authorizeUrl);
    d->verifier = verifier;
    accessTokenFromService();
}

QString KOAuth::errorMessage(int e) {
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

void KOAuth::accessTokenFromService()
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

void KOAuth::setServiceBaseUrl(const QString &serviceBaseUrl)
{
    if (d->serviceBaseUrl == serviceBaseUrl) {
        return;
    }
    d->serviceBaseUrl = serviceBaseUrl;
    updateState();
}

void KOAuth::updateState()
{
    const QUrl u(d->serviceBaseUrl);

    if (u.host() == "twitter.com") {
        //kDebug() << "Using twitter...";
        d->requestTokenUrl = "https://api.twitter.com/oauth/request_token";
        d->accessTokenUrl = "https://api.twitter.com/oauth/access_token";
        d->authorizeUrl = "https://api.twitter.com/oauth/authorize";
        d->consumerKey = "22kfJkztvOqb8WfihEjdg";
        d->consumerSecret = "RpGc0q0aGl0jMkeqMIawUpGyDkJ3DNBczFUyIQMR698";

    } else {
        //kDebug() << "Using identi.ca...";
        d->requestTokenUrl = "https://identi.ca/api/oauth/request_token";
        d->accessTokenUrl = "https://identi.ca/api/oauth/access_token";
        d->authorizeUrl = "https://identi.ca/api/oauth/authorize";
        d->consumerKey = "47a4650a6bd4026b1c4d55d641acdb64";
        d->consumerSecret = "49208b0a87832f4279f9d3742c623910";
    }


    if (!d->user.isEmpty() && !d->serviceBaseUrl.isEmpty()) {
        KSharedConfigPtr ptr = KSharedConfig::openConfig("oauthrc");
        KConfigGroup config = KConfigGroup(ptr, d->user+"@"+d->serviceBaseUrl);
        d->accessToken = config.readEntry("accessToken", QByteArray());
        d->accessTokenSecret = config.readEntry("accessTokenSecret", QByteArray());
        //kDebug() << "oauthrc config for " << d->user+"@"+d->serviceBaseUrl << d->accessToken << d->accessTokenSecret;
        if (isAuthorized()) {
            emit accessTokenReceived(d->serviceBaseUrl, d->accessToken, d->accessTokenSecret);
            d->busy = false;
        }
    }
}

KOAuth::~KOAuth()
{
    delete d;
}

bool KOAuth::isAuthorized()
{
    return !d->accessToken.isEmpty() && !d->accessTokenSecret.isEmpty();
}


QByteArray KOAuth::authorizationHeader(const KUrl &requestUrl, QOAuth::HttpMethod method, QOAuth::ParamMap params)
{
    QByteArray auth;
    auth = d->interface->createParametersString(requestUrl.url(), method,
                                                d->accessToken, d->accessTokenSecret,
                                                QOAuth::HMAC_SHA1, params,
                                                QOAuth::ParseForHeaderArguments);
    return auth;
}

void KOAuth::sign(KIO::Job *job, const QString &url, OAuth::ParamMap params, OAuth::HttpMethod httpMethod)
{
    signRequest(job, url, httpMethod, accessToken(), accessTokenSecret(), params);
}

QByteArray KOAuth::paramsToString(const OAuth::ParamMap &parameters, OAuth::ParsingMode mode)
{
    QByteArray middleString;
    QByteArray endString;
    QByteArray prependString;

    switch (mode) {
    case ParseForInlineQuery:
        prependString = "?";
    case ParseForRequestContent:
    case ParseForSignatureBaseString:
        middleString = "=";
        endString = "&";
        break;
    case ParseForHeaderArguments:
        prependString = "OAuth ";
        middleString = "=\"";
        endString = "\",";
        break;
    default:
        qWarning() << __FUNCTION__ << "- Unrecognized mode";
        return QByteArray();
    }

    QByteArray parameter;
    QByteArray parametersString;

    Q_FOREACH (parameter, parameters.uniqueKeys()) {
        QList<QByteArray> values = parameters.values(parameter);
        if (values.size() > 1) {
            qSort(values.begin(), values.end());
        }
        QByteArray value;
        Q_FOREACH (value, values) {
            parametersString.append(parameter);
            parametersString.append(middleString);
            parametersString.append(value);
            parametersString.append(endString);
        }
    }

    // remove the trailing end character (comma or ampersand)
    parametersString.chop(1);

    // prepend with the suitable string (or none)
    parametersString.prepend(prependString);
    //kDebug() << "paramterString: " << parametersString;
    return parametersString;
}

QByteArray KOAuth::createSignature(const QString &requestUrl, OAuth::HttpMethod method, const QByteArray &token,
                           const QByteArray &tokenSecret, OAuth::ParamMap *params)
{
    //kDebug() << "creating signature";
    // create nonce

    if (!QCA::isSupported("hmac(sha1)")) {
        kError() << "Your QCA2 does not support the HMAC-SHA1 algorithm. Signing requests using OAuth does not work";
        return QByteArray();
    }
    QCA::InitializationVector iv(16);
    QByteArray nonce = iv.toByteArray().toHex();

    // create timestamp
    uint time = QDateTime::currentDateTime().toTime_t();
    QByteArray timestamp = QByteArray::number(time);

    QByteArray httpMethodString = (method == POST) ? "POST" : "GET";
    // create signature base string
    // prepare percent-encoded request URL
    QByteArray percentRequestUrl = requestUrl.toAscii().toPercentEncoding();
    // prepare percent-encoded parameters string
    params->insert("oauth_consumer_key", ConsumerKey);
    //params->insert("oauth_callback", "oob");
    params->insert("oauth_nonce", nonce);
    params->insert("oauth_signature_method", "HMAC-SHA1");
    params->insert("oauth_timestamp", timestamp);
    params->insert("oauth_version", "1.0");
    // append token only if it is defined (requestToken() doesn't use a token at all)
    if (!token.isEmpty()) {
        params->insert("oauth_token", token);
    }

    QByteArray parametersString = paramsToString(*params, ParseForSignatureBaseString);
    QByteArray percentParametersString = parametersString.toPercentEncoding();

    QByteArray digest;

    // create signature base string
    QByteArray signatureBaseString;
    signatureBaseString.append(httpMethodString + "&");
    signatureBaseString.append(percentRequestUrl + "&");
    signatureBaseString.append(percentParametersString);

//     kDebug() << "SIG BASE STRING: " << signatureBaseString;

    if (!QCA::isSupported("hmac(sha1)")) {
        kError() << "Hashing algo not supported, update your QCA";
        return QByteArray();
    }
    // create key for HMAC-SHA1 hashing
    QByteArray key(ConsumerSecret + "&" + tokenSecret);

    // create HMAC-SHA1 digest in Base64
    QCA::MessageAuthenticationCode hmac("hmac(sha1)", QCA::SymmetricKey(key));
    QCA::SecureArray array(signatureBaseString);
    hmac.update(array);
    QCA::SecureArray resultArray = hmac.final();
    digest = resultArray.toByteArray().toBase64();

    // percent-encode the digest
    QByteArray signature = digest.toPercentEncoding();
    //kDebug() << "Signature: " << key << " // " << signature;
    return signature;
}

void KOAuth::signRequest(KIO::Job *job, const QString &requestUrl, OAuth::HttpMethod method, const QByteArray &token,
                 const QByteArray &tokenSecret, const OAuth::ParamMap &params)
{
    QOAuth::ParamMap parameters = params;

    // create signature
    QByteArray signature = createSignature(requestUrl, method, token, tokenSecret, &parameters);
    //kDebug() << "signature: " << signature;

    // add signature to parameters
    parameters.insert("oauth_signature", signature);
    foreach (QByteArray key, params.keys()) {
        parameters.remove(key);
    }

    QByteArray authorizationHeader = paramsToString(parameters, ParseForHeaderArguments);

    job->addMetaData("customHTTPHeader", QByteArray("Authorization: " + authorizationHeader));
    //kDebug() << "job thign...." << authorizationHeader;
}


} // namespace

#include "koauth.moc"
