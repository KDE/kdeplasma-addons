/*
 *   Copyright 2009 by Dominik Kapusta <d@ayoy.net>
 *   Copyright 2010 Ryan P. Bitanga <ryan.bitanga@gmail.com>
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

#include <QtCrypto>

#include <KDebug>
#include <KIO/Job>

#include "oauth.h"

namespace OAuth{

const QByteArray ConsumerKey = "22kfJkztvOqb8WfihEjdg";
const QByteArray ConsumerSecret = "RpGc0q0aGl0jMkeqMIawUpGyDkJ3DNBczFUyIQMR698";

QByteArray paramsToString(const ParamMap &parameters, ParsingMode mode)
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

    return parametersString;
}

QByteArray createSignature(const QString &requestUrl, HttpMethod method, const QByteArray &token,
                           const QByteArray &tokenSecret, ParamMap *params)
{
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
    params->insert("oauth_consumer_key", ConsumerKey);
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
    return signature;
}

void signRequest(KIO::Job *job, const QString &requestUrl, HttpMethod method, const QByteArray &token,
                 const QByteArray &tokenSecret, const ParamMap &params)
{
    ParamMap parameters = params;

    // create signature
    QByteArray signature = createSignature(requestUrl, method, token, tokenSecret, &parameters);

    // add signature to parameters
    parameters.insert("oauth_signature", signature);
    foreach (QByteArray key, params.keys()) {
        parameters.remove(key);
    }

    QByteArray authorizationHeader = paramsToString(parameters, ParseForHeaderArguments);

    job->addMetaData("customHTTPHeader", QByteArray("Authorization: " + authorizationHeader));
}

} // namespace OAuth
