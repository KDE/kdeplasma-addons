/*
 *   Copyright 2009 by Dominik Kapusta <d@ayoy.net>
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

#ifndef OAUTH_H
#define OAUTH_H

#include <QMultiMap>

namespace KIO
{
    class Job;
} // namespace KIO

namespace OAuth {
    // OAuth methods
    enum HttpMethod { POST, GET };
    enum ParsingMode {
        ParseForRequestContent,     //!< Inline query format (<tt>foo=bar&bar=baz&baz=foo ...</tt>), suitable for POST requests
        ParseForInlineQuery,        /*!< Same as ParseForRequestContent, but prepends the string with a question mark -
                                     suitable for GET requests (appending parameters to the request URL) */
        ParseForHeaderArguments,    //!< HTTP request header format (parameters to be put inside a request header)
        ParseForSignatureBaseString //!< <a href=http://oauth.net/core/1.0/#anchor14>Signature Base String</a> format, meant for internal use.
    };

    typedef QMultiMap<QByteArray,QByteArray> ParamMap;

    QByteArray paramsToString(const ParamMap &parameters, ParsingMode mode);
    QByteArray createSignature(const QString &requestUrl, HttpMethod method, const QByteArray &token,
                               const QByteArray &tokenSecret, ParamMap *params);
    void signRequest(KIO::Job *job, const QString &requestUrl, HttpMethod method, const QByteArray &token,
                         const QByteArray &tokenSecret, const ParamMap &params);

};

#endif
