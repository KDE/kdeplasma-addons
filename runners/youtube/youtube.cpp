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

#include "youtube.h"
#include "tubejob.h"
#include "imageiconengine.h"

#include <KDebug>
#include <KToolInvocation>
#include <KRun>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtCore/QTimer>
#include <QtCore/QWaitCondition>
#include <QtCore/QEventLoop>
#include <qfile.h>
#include <QtGui/QIcon>
#include <qpushbutton.h>
#include <qjson/parser.h>
#include <solid/networking.h>

//TODO: I'd really *love* to be able to embed a video *inside* krunner. you know how sexy that'd be? answer: very much.
//but seeing as youtube doesn't fully support html5 (only for non-ad'ed videos), i guess i'll have to hold off on it?
YouTube::YouTube(QObject *parent, const QVariantList& args)
    : Plasma::AbstractRunner(parent, args)
    , m_thumbnailDownloader(0)
{
    Q_UNUSED(args);
    setObjectName(QLatin1String("YouTube"));
    setIgnoredTypes(Plasma::RunnerContext::FileSystem | Plasma::RunnerContext::Directory | Plasma::RunnerContext::NetworkLocation);

    addSyntax(Plasma::RunnerSyntax("videos :q:", i18n("Lists the videos matching the query, using YouTube search")));
    setSpeed(SlowSpeed);
    setPriority(LowPriority);
}

YouTube::~YouTube()
{
}

void YouTube::match(Plasma::RunnerContext &context)
{
    // Check for networkconnection
    if(Solid::Networking::status() == Solid::Networking::Unconnected) {
        return;
    }

    QString term = context.query();

    if (!term.startsWith("videos ")) {
        return;
    } else {
        term = term.remove("videos ");
    }

    if (term.length() < 3) {
        return;
    }

    if (!context.isValid()) {
        return;
    }

    QEventLoop loop;
    TubeJob tubeJob(term, context.singleRunnerQueryMode());
    connect(&tubeJob, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    parseJson(tubeJob.data(), context);
}

void YouTube::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    KRun *opener = new KRun(match.data().toString(), 0);
    opener->setRunExecutables(false);
}

void YouTube::parseJson(const QByteArray& data, Plasma::RunnerContext &context)
{
    QJson::Parser parser;
    const QVariantMap resultsMap = parser.parse(data).toMap();

    QVariantMap related = resultsMap.value("feed").toMap();

    QVariantList subList = related.value("entry").toList();

    const QString term = context.query();

    foreach (const QVariant& variant, subList) {
        QVariantMap subMap = variant.toMap();

        QVariantList linkList = subMap.value("link").toList();
        //FIXME: hardcoded..
        const QString& url = linkList.at(0).toMap().value("href").toString();

        QVariantMap titleMap = subMap.value("title").toMap();
        const QString& title = titleMap.value("$t").toString();

        QVariantMap subSubMap = subMap.value("media$group").toMap();

        QVariantList thumbnailList = subSubMap.value("media$thumbnail").toList();

        QString thumbnail;
        thumbnail = thumbnailList.at(2).toMap().value("url").toString();

        QEventLoop loop;
        m_thumbnailDownloader = new QNetworkAccessManager();
        connect(m_thumbnailDownloader, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));

        QNetworkRequest request = QNetworkRequest(QUrl(thumbnail));

        QNetworkReply *reply= m_thumbnailDownloader->get(request);
        loop.exec();

        Plasma::QueryMatch match(this);
        match.setType(Plasma::QueryMatch::PossibleMatch);

        if (reply->error() != 0) {
            kDebug() << "KRunner::YouTube runner, Json parser error. please report. error code: " << reply->error();
        }

        QByteArray data = reply->readAll();

        QImage image;
        image.loadFromData(data);

        QIcon icon(new ImageIconEngine(image));
        match.setIcon(icon);

        match.setData(url);
        match.setText(i18n("%1 on YouTube", title));

        context.addMatch(term, match);
    }
}

#include "youtube.moc"
