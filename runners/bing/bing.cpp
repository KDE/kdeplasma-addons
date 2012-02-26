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

#include "bing.h"
#include "bingjob.h"
#include "imageiconengine.h"

#include <KDebug>
#include <KToolInvocation>
#include <KRun>

#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtCore/QTimer>
#include <QtCore/QWaitCondition>
#include <QtCore/QEventLoop>
#include <QtCore/QMap>
#include <qjson/parser.h>

Bing::Bing(QObject *parent, const QVariantList& args)
    : Plasma::AbstractRunner(parent, args)
{
    Q_UNUSED(args);
    setObjectName(QLatin1String("Bing"));
    setIgnoredTypes(Plasma::RunnerContext::FileSystem | Plasma::RunnerContext::Directory | Plasma::RunnerContext::NetworkLocation);

    Plasma::RunnerSyntax s(QLatin1String( ":q:" ), i18n("Finds Bing images search matching :q:."));
    s.addExampleQuery(QLatin1String("image :q:"));
    addSyntax(s);

    addSyntax(Plasma::RunnerSyntax(QLatin1String( "image" ), i18n("Lists the search entries matching the query, using Bing search")));
    setSpeed(SlowSpeed);
    setPriority(LowPriority);
}

Bing::~Bing()
{
}

void Bing::match(Plasma::RunnerContext &context)
{
    kDebug() << "Bing Runner, MATCH MADE";

    const QString term = context.query();

    if (term.length() < 3) {
        return;
    }

    if (!context.isValid()) {
        return;
    }

    QEventLoop loop;

    BingJob bingJob(term);
    connect(&bingJob, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    parseJson(bingJob.data(), context);
}

void Bing::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    KRun *opener = new KRun(match.data().toString(), 0);
    opener->setRunExecutables(false);
}

void Bing::parseJson(const QByteArray& data, Plasma::RunnerContext &context)
{
    kDebug() << "JSON PARSER ONLINE";
    QJson::Parser parser;
    const QVariantMap resultsMap = parser.parse(data).toMap();

    QVariantMap related = resultsMap.value("SearchResponse").toMap();

    QVariantMap subMap = related.value("Image").toMap();

    QVariantList resultsList = subMap.value("Results").toList();

    foreach (const QVariant& variant, resultsList) {

        QVariantMap subMap = variant.toMap();
        kDebug() << subMap.keys();

        const QString& url = subMap.value("Url").toString();
        const QString& title = subMap.value("Title").toString();

        QVariantMap thumbnailMap = subMap.value("Thumbnail").toMap();

        const QString& thumbnail = thumbnailMap.value("Url").toString();

        QEventLoop loop;
        m_thumbnailDownloader = new QNetworkAccessManager();
        connect(m_thumbnailDownloader, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));

        QNetworkRequest request = QNetworkRequest(QUrl(thumbnail));

        QNetworkReply *reply= m_thumbnailDownloader->get(request);
        loop.exec();

        Plasma::QueryMatch match(this);
        match.setType(Plasma::QueryMatch::PossibleMatch);

        if (reply->error() != 0) {
            kDebug() << "KRunner::Bing runner, Json parser error. please report. error code: " << reply->error();
        }

        QByteArray data = reply->readAll();

        QImage image;
        image.loadFromData(data);

        QIcon icon(new ImageIconEngine(image));
        match.setIcon(icon);

        match.setData(url);
        match.setText(QString(title + " on Bing"));

        context.addMatch(context.query(), match);
    }
}

#include "bing.moc"
