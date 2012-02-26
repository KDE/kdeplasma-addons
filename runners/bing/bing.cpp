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

#include <KDebug>
#include <KToolInvocation>

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

    Plasma::RunnerSyntax s(QLatin1String( ":q:" ), i18n("Finds Bing search matching :q:."));
    s.addExampleQuery(QLatin1String("bing :q:"));
    addSyntax(s);

    addSyntax(Plasma::RunnerSyntax(QLatin1String( "bing" ), i18n("Lists the search entries matching the query, using Bing search")));
    addSyntax(Plasma::RunnerSyntax(QLatin1String( "wolfram" ), i18n("Searches using Wolfram Alpha, powered by Bing")));
    addSyntax(Plasma::RunnerSyntax(QLatin1String( "define" ), i18n("Defines words using dictionaries, powered by Bing")));
    setSpeed(SlowSpeed);
    setPriority(LowPriority);

    qRegisterMetaType<Plasma::RunnerContext*>();

    KUrl url;
    url = "http://api.bing.net/json.aspx?AppId=340D9148BE10A564ABFC17937FFB623836112FBB&Query=waterfalls&Sources=Image&Version=2.0&Image.Count=10&Image.Offset=0";
    // "http://api.bing.com/?q=define+ostensibly&format=json&pretty=1");

    m_job = KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
    connect(m_job, SIGNAL(result(KJob*)), this, SLOT(jobFinished(KJob*)));
    m_job->start();
}

Bing::~Bing()
{
}

void Bing::match(Plasma::RunnerContext &context)
{
    kDebug() << "MATCH MADE, emitting matchmade";
//    connect(this, SIGNAL(matchMade(Plasma::RunnerContext*)), this, SLOT(startBingJob(Plasma::RunnerContext*)));
 //   emit matchMade(&context);

    const QString term = context.query();
    if (term.length() < 3) {
        return;
    }

    if (!context.isValid()) {
        return;
    }
}

void Bing::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
//    Q_UNUSED(context)
//    const QString session = match.data().toString();
//    kDebug() << "Open Konsole Session " << session;
//
//    if (!session.isEmpty()) {
//        QStringList args;
//        args << QLatin1String( "--profile" );
//        args << session;
//        kDebug() << "=== START: konsole" << args;
//        KToolInvocation::kdeinitExec(QLatin1String( "konsole" ), args);
//    }
}

void Bing::startBingJob(Plasma::RunnerContext *context)
{

    kDebug() << "%%%%%% RUNNING JOB!";
}

void Bing::dataArrived(KIO::Job* job, const QByteArray& data)
{
//    kDebug()  << "DATA:" << data;
    if (!data.isEmpty()) {
        buffer << data;
//        parseJson(data);
    }
//    const QString term = context->query();
//    Plasma::QueryMatch match(this);
//    match.setType(Plasma::QueryMatch::PossibleMatch);
//
//    //  match.setRelevance(1.0);
//    //  match.setIcon(m_icon);
////    match.setData("TEST");
//    match.setText(QLatin1String( "Bing: " ));
//
//    context->addMatch(term, match);

}

void Bing::jobFinished(KJob *job)
{
    parseJson(m_job->data());
}

void Bing::parseJson(const QByteArray& data)
{
    kDebug() << "JSON PARSER ONLINE";
    QJson::Parser parser;
    const QVariantMap resultsMap = parser.parse(data).toMap();

    QVariantMap relatedMap = resultsMap.value("SearchResponse").toMap();
//kDebug() << relatedMap.values();

    QVariantMap subMap = relatedMap.value("Image").toMap();
 //   kDebug() << subMap.values();
//    kDebug() << subMap.values();
//    kDebug() << subMap.value("Results").toList();
    QVariantList mapList = subMap.value("Results").toList();

    foreach (const QVariant& variant, mapList) {
        QVariantMap subSubMap = variant.toMap();

        kDebug() << subSubMap.value("Title");
        kDebug() << subSubMap.value("Url");

        QVariantMap thumbnailMap = subSubMap.value("Thumbnail").toMap();

        kDebug() << thumbnailMap.value("Url");
    }
}

#include "bing.moc"
