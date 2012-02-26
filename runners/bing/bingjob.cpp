/******************************************************************************
 *  Copyright (C) 2012 by Shaun Reich <sreich@kde.org                         *
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

#include "bingjob.h"

#include <KDebug>
#include <KToolInvocation>

#include <QtCore/qurl.h>
#include <QNetworkAccessManager>
#include <QNetworkReply>

BingJob::BingJob(const QString& term)
  : QObject()
  , m_manager(0)
{
    kDebug() << "%%%%%% TubeJob ctor hit! QUERY TERM: " + term;

    m_manager = new QNetworkAccessManager(this);

    QUrl url = QUrl("http://api.bing.net/json.aspx?AppId=340D9148BE10A564ABFC17937FFB623836112FBB&Query=" + term + "&Sources=Image&Version=2.0&Image.Count=10&Image.Offset=0");
    QNetworkRequest request = QNetworkRequest(url);

    m_manager->get(request);

    connect(m_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(jobCompleted(QNetworkReply*)));
}

void BingJob::jobCompleted(QNetworkReply* reply)
{
    m_data = reply->readAll();
    kDebug() << "JOBCOMPLETED";

    emit finished();
}

QByteArray BingJob::data()
{
    return m_data;
}


#include "bingjob.moc"
