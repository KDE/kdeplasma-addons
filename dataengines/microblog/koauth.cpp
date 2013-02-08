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
#include <KWallet/Wallet>

#include <KWebView>
#include <KIO/AccessManager>
#include <KSharedConfig>
#include <QWebFrame>

#include "koauth.h"
#include "koauthwebhelper.h"

#include <KDebug>
#include <QtOAuth/QtOAuth>

namespace KOAuth {

class KOAuthPrivate {

public:
    KOAuthPrivate()
    {
        interface = new QOAuth::Interface();
        busy = false;
        useWallet = true;
    }

    QOAuth::Interface* interface;

    QString user;
    QString password;
    bool useWallet;

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

    QStringList authorizedAccounts;

    QString verifier;

    QCA::Initializer *qcaInitializer;

    KOAuthWebHelper *w;
};


KOAuth::KOAuth(QObject* parent)
    : QThread(parent),
      d(0)
{

    setObjectName(QLatin1String("KOAuth"));
//     d = new KOAuthPrivate;

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
    //just create it to correctly initialize QCA and clean up when createSignature() returns

    if (!d) {
        d = new KOAuthPrivate;
        d->qcaInitializer = new QCA::Initializer();
#ifndef NO_KIO
        KIO::AccessManager *access = new KIO::AccessManager(this);
        d->interface->setNetworkAccessManager(access);
#endif
        d->w = new KOAuthWebHelper();
        connect(this, SIGNAL(authorizeApp(const QString&, const QString&, const QString&)),
                d->w, SLOT(authorizeApp(const QString&, const QString&, const QString&)));
        connect(d->w, SIGNAL(appAuthSucceeded(const QString&, const QString&)),
                this, SLOT(appAuthorized(const QString&, const QString&)));
        connect(d->w, SIGNAL(statusUpdated(const QString&, const QString&, const QString&, const QString&)),
                SIGNAL(statusUpdated(const QString&, const QString&, const QString&, const QString&)));
    }

}

void KOAuth::run()
{
    if (isAuthorized()) {
        emit statusUpdated(d->user, d->serviceBaseUrl, "Ok", "User authorized");
    } else {
        //authorize(d->serviceBaseUrl, d->user, d->password);
    }
}

void KOAuth::authorize(const QString &serviceBaseUrl, const QString &user, const QString &password)
{
    if (!user.isEmpty()) {
        d->user = user;
    }
    d->password = password;
    d->serviceBaseUrl = serviceBaseUrl;
    d->accessToken = QByteArray();
    d->accessTokenSecret = QByteArray();

    d->w->setUser(d->user);
    d->w->setServiceBaseUrl(serviceBaseUrl);
    d->w->setPassword(password);

    //run();
    if (!isAuthorized()) {
        d->busy = true;
        kDebug() << "request token";
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

    QOAuth::ParamMap params;
    params.insert("oauth_callback", "oob");
//     kDebug() << "starting token request ...";
    QOAuth::ParamMap reply = d->interface->requestToken(d->requestTokenUrl,
                                                        QOAuth::GET, QOAuth::HMAC_SHA1, params);
//     kDebug() << "token request done......" << reply;

    QString e;
    if (d->interface->error() == QOAuth::NoError) {
        d->requestToken = reply.value(QOAuth::tokenParameterName());
        d->requestTokenSecret = reply.value(QOAuth::tokenSecretParameterName());

        QString auth_url = QString("%1?oauth_token=%2").arg(d->authorizeUrl, QString(d->requestToken));

        emit statusUpdated(d->user, d->serviceBaseUrl, "Busy", "Request token received.");
        emit authorizeApp(d->serviceBaseUrl, d->authorizeUrl, auth_url);

    } else {
        e += errorMessage(d->interface->error());
        kDebug() << "Request Token returned error:" << e;
        emit statusUpdated(d->user, d->serviceBaseUrl, "Error", "Request Token Error: " + e);
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
    QOAuth::ParamMap params = QOAuth::ParamMap();
    params.insert("oauth_callback", "oob");
    if (d->serviceBaseUrl.toLower().contains("identi.ca")) {
        params.insert("oauth_verifier", d->verifier.toLocal8Bit());
    }
    QOAuth::ParamMap reply = d->interface->accessToken(d->accessTokenUrl, QOAuth::GET,
                                                       d->requestToken, d->requestTokenSecret,
                                                       QOAuth::HMAC_SHA1, params);

    QString e;
    if (d->interface->error() == QOAuth::NoError) {
        d->accessToken = reply.value(QOAuth::tokenParameterName());
        d->accessTokenSecret = reply.value(QOAuth::tokenSecretParameterName());

        //kDebug() << "Received Access Token OK!" << d->accessToken << d->accessTokenSecret;
        emit accessTokenReceived(d->user, d->serviceBaseUrl, d->accessToken, d->accessTokenSecret);
        d->busy = false;

        if (d->useWallet) {
            saveCredentials();
        } else {
            KSharedConfigPtr ptr = KSharedConfig::openConfig("koauthrc");
            KConfigGroup config = KConfigGroup(ptr, d->user+"@"+d->serviceBaseUrl);
            config.writeEntry("accessToken", d->accessToken);
            config.writeEntry("accessTokenSecret", d->accessTokenSecret);
            config.sync();
        }
        emit authorized();
    } else {
        kError() << d->interface->error() << reply;
        e += errorMessage(d->interface->error());
        kError() << "Request Not working" << e;
        emit statusUpdated(d->user, d->serviceBaseUrl, "Error", "Access Token Error:" + e);
        d->busy = false;
    }
}

QStringList KOAuth::authorizedAccounts()
{
//     KSharedConfigPtr ptr = KSharedConfig::openConfig("koauthrc", KConfig::SimpleConfig);
    //KConfigGroup config = KConfigGroup(ptr);
    KWallet::Wallet *wallet = KWallet::Wallet::openWallet(KWallet::Wallet::NetworkWallet(),
                                           0, KWallet::Wallet::Synchronous);
    QStringList aac;
    if (wallet && wallet->isOpen() && wallet->setFolder("Plasma-MicroBlog")) {

        QMap<QString, QMap<QString, QString> > mapMap;
        if (wallet->readMapList("*", mapMap) == 0) {
//             kDebug() << "see my maps: " << mapMap;
            aac = mapMap.keys();
            kDebug() << "!!! read accounts: " << aac;
        } else {
            kWarning() << "Unable to read grouplist from wallet";
        }
    } else {
        kWarning() << "Unable to open wallet";
    }

    return aac;
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

    if (u.host().endsWith("twitter.com")) {
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
        if (d->useWallet) {
            retrieveCredentials();
        } else {
            KSharedConfigPtr ptr = KSharedConfig::openConfig("koauthrc");
            KConfigGroup config = KConfigGroup(ptr, d->user+"@"+d->serviceBaseUrl);
            d->accessToken = config.readEntry("accessToken", QByteArray());
            d->accessTokenSecret = config.readEntry("accessTokenSecret", QByteArray());
        }
        //kDebug() << "oauthrc config for " << d->user+"@"+d->serviceBaseUrl << d->accessToken << d->accessTokenSecret;
        if (isAuthorized()) {
            emit accessTokenReceived(d->user, d->serviceBaseUrl, d->accessToken, d->accessTokenSecret);
            d->busy = false;
        }
    }
}

KOAuth::~KOAuth()
{
    if (!d) return;
    delete d->w;
    delete d->qcaInitializer;
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

void KOAuth::sign(KIO::Job *job, const QString &url, QOAuth::ParamMap params, HttpMethod httpMethod)
{
    signRequest(job, url, httpMethod, accessToken(), accessTokenSecret(), params);
}

QByteArray KOAuth::paramsToString(const QOAuth::ParamMap &parameters, ParsingMode mode)
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

QByteArray KOAuth::createSignature(const QString &requestUrl, HttpMethod method, const QByteArray &token,
                           const QByteArray &tokenSecret, QOAuth::ParamMap *params)
{
    if (!QCA::isSupported("hmac(sha1)")) {
        kError() << "Your QCA2 does not support the HMAC-SHA1 algorithm. Signing requests using OAuth does not work";
        return QByteArray();
    }

    // create nonce
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
    params->insert("oauth_consumer_key", d->consumerKey);
    //params->insert("oauth_callback", "oob");
    params->insert("oauth_nonce", nonce);
    params->insert("oauth_signature_method", "HMAC-SHA1");
    params->insert("oauth_timestamp", timestamp);
    params->insert("oauth_version", "1.0");
    // append token only if it is defined (requestToken() doesn't use a token at all)
    if (!token.isEmpty()) {
        params->insert("oauth_token", token);
    }

    foreach (const QByteArray &_b, params->keys()) {
        if (params->count(_b) > 1) {
            kWarning() << "Request argument " << _b << "multiple times. This might break.";
        }
    }

    QByteArray parametersString = paramsToString(*params, ParseForSignatureBaseString); // TODO use createSignature()
    QByteArray percentParametersString = parametersString.toPercentEncoding();

    QByteArray digest;

    // create signature base string
    QByteArray signatureBaseString;
    signatureBaseString.append(httpMethodString + "&");
    signatureBaseString.append(percentRequestUrl + "&");
    signatureBaseString.append(percentParametersString);

    if (!QCA::isSupported("hmac(sha1)")) {
        kError() << "Hashing algo not supported, update your QCA";
        return QByteArray();
    }
    // create key for HMAC-SHA1 hashing
    QByteArray key(d->consumerSecret + "&" + tokenSecret);

    // create HMAC-SHA1 digest in Base64
    QCA::MessageAuthenticationCode hmac("hmac(sha1)", QCA::SymmetricKey(key));
    QCA::SecureArray array(signatureBaseString);
    hmac.update(array);
    QCA::SecureArray resultArray = hmac.final();
    digest = resultArray.toByteArray().toBase64();

    // percent-encode the digest
    QByteArray signature = digest.toPercentEncoding();
    return signature;
}

void KOAuth::signRequest(KIO::Job *job, const QString &requestUrl, HttpMethod method, const QByteArray &token,
                 const QByteArray &tokenSecret, const QOAuth::ParamMap &params)
{
   QOAuth::ParamMap parameters = params;

    // create signature
    QByteArray signature = createSignature(requestUrl, method, token, tokenSecret, &parameters);

    // add signature to parameters
    parameters.insert("oauth_signature", signature);
    foreach (QByteArray key, params.keys()) {
        parameters.remove(key);
    }
    kDebug() << parameters;

    QByteArray authorizationHeader = paramsToString(parameters, ParseForHeaderArguments);
    job->addMetaData("customHTTPHeader", QByteArray("Authorization: " + authorizationHeader));
}

QByteArray KOAuth::userParameters(const QOAuth::ParamMap& parameters)
{
    if (!parameters.count()) {
        return QByteArray();
    }
    return d->interface->inlineParameters(parameters, QOAuth::ParseForInlineQuery);
}

void KOAuth::forgetAccount(const QString& user, const QString& serviceUrl)
{
    const QString &_id = user + "@" + serviceUrl;
    KWallet::Wallet *wallet = KWallet::Wallet::openWallet(KWallet::Wallet::NetworkWallet(),
                                           0, KWallet::Wallet::Synchronous);
    wallet->setFolder("Plasma-MicroBlog");
    if (!wallet->removeEntry(_id)) {
        kError() << "Error removing : " << _id;
    }

    wallet->sync();
}



void KOAuth::saveCredentials() const
{
    QSharedPointer<KWallet::Wallet> wallet(KWallet::Wallet::openWallet(KWallet::Wallet::NetworkWallet(),
                                           0, KWallet::Wallet::Synchronous));
    const QString folder("Plasma-MicroBlog");
    if (wallet && wallet->isOpen() &&
        (wallet->hasFolder(folder) ||
         wallet->createFolder(folder)) &&
         wallet->setFolder(folder)) {

        //QStringList accounts =
        QMap<QString, QString> map;
        map["accessToken"] = QString(d->accessToken);
        map["accessTokenSecret"] = QString(d->accessTokenSecret);

        if (wallet->writeMap(identifier(), map) != 0) {
            kWarning() << "Unable to write accessToken & Secret to wallet";
        }
    } else {
        kWarning() << "Unable to open Plasma-MicroBlog wallet";
    }
}

QString KOAuth::identifier() const
{
    return QString("%1@%2").arg(d->user, d->serviceBaseUrl);
}

bool KOAuth::retrieveCredentials() const
{
    KWallet::Wallet *wallet = KWallet::Wallet::openWallet(KWallet::Wallet::NetworkWallet(),
                                           0, KWallet::Wallet::Synchronous);
    if (wallet && wallet->isOpen() && wallet->setFolder("Plasma-MicroBlog")) {

//         QMap<QString, QMap<QString, QString> > mapMap;
        QMap<QString, QMap<QString, QString> > mapMap;
        if (wallet->readMapList("*", mapMap) == 0) {
            d->authorizedAccounts = mapMap.keys();
            if (d->authorizedAccounts.contains(identifier())) {
                d->accessToken = mapMap[identifier()]["accessToken"].toAscii();
                d->accessTokenSecret = mapMap[identifier()]["accessTokenSecret"].toAscii();
                return true;
            }
            return false;
        } else {
            //kWarning() << "Unable to read credentials from wallet";
        }
    } else {
        kWarning() << "Unable to open wallet";
    }

    return false;
}
void KOAuth::forgetCredentials() const
{
    d->user = QByteArray();
    d->serviceBaseUrl = QByteArray();
    d->password = QString();
    saveCredentials();
}

void KOAuth::configToWallet()
{
    KSharedConfigPtr gptr = KSharedConfig::openConfig("koauthrc", KConfig::SimpleConfig);
    KSharedConfigPtr ptr = KSharedConfig::openConfig("koauthrc");
    foreach (const QString g, gptr->groupList()) {
        KConfigGroup config = KConfigGroup(ptr, g);
        d->user = g.split('@')[0];
        d->serviceBaseUrl = g.split('@')[1];
        d->accessToken = config.readEntry("accessToken", QByteArray());
        d->accessTokenSecret = config.readEntry("accessTokenSecret", QByteArray());
        saveCredentials();
    }
}



} // namespace

#include "koauth.moc"
