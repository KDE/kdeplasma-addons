/******************************************************************************
 *  Copyright (C) 2013 by David Baum <david.baum@naraesk.eu>                  *
 *                                                                            *
 *  This library is free software; you can redistribute it and/or modify      *
 *  it under the terms of the GNU Lesser General Public License as published  *
 *  by the Free Software Foundation; either version 2 of the License or (at   *
 *  your option) any later version.                                           *
 *                                                                            *
 *  This library is distributed in the hope that it will be useful,           *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 *  Library General Public License for more details.                          *
 *                                                                            *
 *  You should have received a copy of the GNU Lesser General Public License  *
 *  along with this library; see the file COPYING.LIB.                        *
 *  If not, see <http://www.gnu.org/licenses/>.                               *
 *****************************************************************************/

#include "translatorjob.h"

TranslatorJob::TranslatorJob(const QString &text, const QPair<QString, QString> &language)
: m_manager(0)
{
	m_manager = new QNetworkAccessManager(this);
	
	QNetworkRequest request(QUrl("http://www.google.com/translate_a/t"));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
	
	QUrl postData;
	postData.addQueryItem("client", "t");
	postData.addQueryItem("sl", language.first);
	postData.addQueryItem("tl",language.second);
	postData.addQueryItem("text", text);
	
	m_manager -> post(request, postData.encodedQuery());
	connect(m_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(jobCompleted(QNetworkReply*)));
}

void TranslatorJob::jobCompleted(QNetworkReply* reply)
{
	m_result = QString::fromUtf8(reply->readAll());
	reply->deleteLater();
	emit finished();
}

QString TranslatorJob::result()
{
	return m_result;
}