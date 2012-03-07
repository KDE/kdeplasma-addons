/******************************************************************************
 *  Copyright (C) 2012 by Shaun Reich <shaun.reich@blue-systems.com           *
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

#include "tubejob.h"

#include <KDebug>
#include <KToolInvocation>

#include <QNetworkAccessManager>
#include <QNetworkReply>

TubeJob::TubeJob(const QString& term, bool singleRunnerMode)
  : QObject()
  , m_manager(0)
{
    m_manager = new QNetworkAccessManager(this);

    QUrl url;

    if (singleRunnerMode) {
        url = ("http://gdata.youtube.com/feeds/api/videos?max-results=30&alt=json&q=" + term);
    } else {
        url = ("http://gdata.youtube.com/feeds/api/videos?max-results=10&alt=json&q=" + term);
    }

    QNetworkRequest request = QNetworkRequest(url);
    m_manager->get(request);

    connect(m_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(jobCompleted(QNetworkReply*)));
}

void TubeJob::jobCompleted(QNetworkReply* reply)
{
    m_data = reply->readAll();

    emit finished();
}

QByteArray TubeJob::data()
{
    return m_data;
}


#include "tubejob.moc"
