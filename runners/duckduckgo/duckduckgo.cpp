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

//TODO: I'd really *love* to be able to embed a video *inside* krunner. you know how sexy that'd be? answer: very much.
//but seeing as youtube doesn't fully support html5 (only for non-ad'ed videos), i guess i'll have to hold off on it?
DuckDuckGo::DuckDuckGo(QObject *parent, const QVariantList& args)
    : Plasma::AbstractRunner(parent, args)
{
    Q_UNUSED(args);
    setObjectName(QLatin1String("DuckDuckGo"));
    setIgnoredTypes(Plasma::RunnerContext::FileSystem | Plasma::RunnerContext::Directory | Plasma::RunnerContext::NetworkLocation);

    Plasma::RunnerSyntax s(QLatin1String( ":q:" ), i18n("Finds DuckDuckGo search matching :q:."));
    s.addExampleQuery(QLatin1String("duckduckgo :q:"));
    addSyntax(s);

    addSyntax(Plasma::RunnerSyntax(QLatin1String( "duckduckgo" ), i18n("Lists the search entries matching the query, using DuckDuckGo search")));
    setSpeed(SlowSpeed);
    setPriority(LowPriority);

    qRegisterMetaType<Plasma::RunnerContext*>();

    KIO::TransferJob *job = KIO::get(KUrl("http://api.duckduckgo.com/?q=simpsons+characters&format=json&pretty=1"), KIO::NoReload, KIO::HideProgressInfo);
    connect(job, SIGNAL(data(KIO::Job*,QByteArray)), this, SLOT(dataArrived(KIO::Job*,QByteArray)));
    connect(job, SIGNAL(finished(KJob*)), this, SLOT(jobFinished(KJob*)));
    job->start();
}

DuckDuckGo::~DuckDuckGo()
{
}

void DuckDuckGo::match(Plasma::RunnerContext &context)
{
    kDebug() << "MATCH MADE, emitting matchmade";
//    connect(this, SIGNAL(matchMade(Plasma::RunnerContext*)), this, SLOT(startDuckDuckGoJob(Plasma::RunnerContext*)));
 //   emit matchMade(&context);

    const QString term = context.query();
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
    parseJson(buffer);
}

void DuckDuckGo::parseJson(const QDataStream& data)
{
    kDebug() << "JSON PARSER ONLINE";
    QJson::Parser parser;
    const QVariantMap resultsMap = parser.parse(data.device()).toMap();
//    QVariantList resultList;
    kDebug() << resultsMap.keys();


//    if (xml.hasError()) {
//        kError() << "DuckDuckGo Runner xml parse failure";
//        return;
//    }
//
//    while (!xml.atEnd()) {
//        QXmlStreamReader::TokenType token = xml.readNext();
//
//        if (token == QXmlStreamReader::StartDocument) {
//            continue;
//        }
//
//        if (token == QXmlStreamReader::StartElement) {
//            if (xml.name() == "group") {
//                parseVideo(xml);
//            }
//        }
//    }
}


#include "duckduckgo.moc"
