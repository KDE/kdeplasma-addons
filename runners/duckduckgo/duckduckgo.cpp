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

#include "duckduckgo.h"

#include <KDebug>
#include <KToolInvocation>

#include <QtCore/QTimer>
#include <QtCore/QWaitCondition>
#include <QtCore/QEventLoop>
#include <qjson/parser.h>

DuckDuckGo::DuckDuckGo(QObject *parent, const QVariantList& args)
    : Plasma::AbstractRunner(parent, args)
{
    Q_UNUSED(args);
    setObjectName(QLatin1String("DuckDuckGo"));
    setIgnoredTypes(Plasma::RunnerContext::FileSystem | Plasma::RunnerContext::Directory | Plasma::RunnerContext::NetworkLocation);

    addSyntax(Plasma::RunnerSyntax(QLatin1String( "duckduckgo :q:" ), i18n("Lists the search entries matching the query, using DuckDuckGo search")));
    addSyntax(Plasma::RunnerSyntax(QLatin1String( "wolfram :q:" ), i18n("Searches using Wolfram Alpha, powered by DuckDuckGo")));
    addSyntax(Plasma::RunnerSyntax(QLatin1String( "define :q:" ), i18n("Defines words using dictionaries, powered by DuckDuckGo")));
    setSpeed(SlowSpeed);
    setPriority(LowPriority);

    qRegisterMetaType<Plasma::RunnerContext*>();

    KUrl url = KUrl("http://api.duckduckgo.com/?q=futurama+characters&format=json&pretty=1");
       // "http://api.duckduckgo.com/?q=define+ostensibly&format=json&pretty=1");

    m_job = KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
    connect(m_job, SIGNAL(result(KJob*)), this, SLOT(jobFinished(KJob*)));
    m_job->start();
}

DuckDuckGo::~DuckDuckGo()
{
}

void DuckDuckGo::match(Plasma::RunnerContext &context)
{
    kDebug() << "MATCH MADE, emitting matchmade";
//    connect(this, SIGNAL(matchMade(Plasma::RunnerContext*)), this, SLOT(startDuckDuckGoJob(Plasma::RunnerContext*)));
 //   emit matchMade(&context);

    QString term = context.query();

    if (!term.startsWith("duckduckgo ")) {
        return;
    } else {
        term = term.remove("duckduckgo ");
    }

    if (!term.startsWith("wolfram ")) {
        return;
    } else {
        term = term.remove("wolfram ");
    }

    if (!term.startsWith("define ")) {
        return;
    } else {
        term = term.remove("define ");
    }


    if (term.length() < 3) {
        return;
    }

    if (!context.isValid()) {
        return;
    }
}

void DuckDuckGo::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
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

void DuckDuckGo::startDuckDuckGoJob(Plasma::RunnerContext *context)
{

    kDebug() << "%%%%%% DUCKDUCKGO RUNNING JOB!";
}

void DuckDuckGo::dataArrived(KIO::Job* job, const QByteArray& data)
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
//    match.setText(QLatin1String( "DuckDuckGo: " ));
//
//    context->addMatch(term, match);

}

void DuckDuckGo::jobFinished(KJob *job)
{
    parseJson(m_job->data());
}

void DuckDuckGo::parseJson(const QByteArray& data)
{
    kDebug() << "JSON PARSER ONLINE";
    QJson::Parser parser;
    const QVariantMap resultsMap = parser.parse(data).toMap();

    const QString& match = "duckduckgo";

    if (match == "define") {
        //dictionary mode
        kDebug() << "Heading:" << resultsMap.value("Heading");
        kDebug() << "AbstractSource:" << resultsMap.value("AbstractSource");
        kDebug() << "Abstract:" << resultsMap.value("Abstract");
        kDebug() << "AbstractURL:" << resultsMap.value("AbstractURL");
    } else if (match == "wolfram") {
        //wolfram mode (simple redirection, because web search providers are assholes)
        kDebug() << "Redirect:" << resultsMap.value("Redirect");
    } else if (match == "duckduckgo") {
        QList<QVariant> related = resultsMap.value("RelatedTopics").toList();

        foreach (const QVariant& variant, related) {
            QVariantMap submap = variant.toMap();

            kDebug() << "FirstURL:" << submap.value("FirstURL");
            kDebug() << "Text:" << submap.value("Text");
            kDebug() << "Icon:" << submap.value("Icon").toMap().value("URL");
        }
    }
}


#include "duckduckgo.moc"
