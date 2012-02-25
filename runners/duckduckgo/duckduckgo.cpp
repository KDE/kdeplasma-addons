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
#include <QtXml/QXmlStreamReader>
#include <QtCore/QEventLoop>

//TODO: I'd really *love* to be able to embed a video *inside* krunner. you know how sexy that'd be? answer: very much.
//but seeing as youtube doesn't fully support html5 (only for non-ad'ed videos), i guess i'll have to hold off on it?
DuckDuckGo::DuckDuckGo(QObject *parent, const QVariantList& args)
    : Plasma::AbstractRunner(parent, args)
    , m_context(0)
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

    KIO::TransferJob *job = KIO::get(KUrl("http://gdata.youtube.com/feeds/api/videos?max-results=1&q=taylor swift"), KIO::NoReload, KIO::HideProgressInfo);
    connect(job, SIGNAL(data(KIO::Job*,QByteArray)), this, SLOT(dataArrived(KIO::Job*,QByteArray)));
    job->start();
}

DuckDuckGo::~DuckDuckGo()
{
}

void DuckDuckGo::match(Plasma::RunnerContext &context)
{
  //  m_context = &context;
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
    kDebug()  << "DATA:" << data;
    if (!data.isEmpty()) {
        parseXML(data);
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

void DuckDuckGo::parseXML(QByteArray data)
{
    QXmlStreamReader xml(data);

    if (xml.hasError()) {
        kError() << "DuckDuckGo Runner xml parse failure";
        return;
    }

    while (!xml.atEnd()) {
        QXmlStreamReader::TokenType token = xml.readNext();

        if (token == QXmlStreamReader::StartDocument) {
            continue;
        }

        if (token == QXmlStreamReader::StartElement) {
            if (xml.name() == "group") {
                parseVideo(xml);
            }
        }
    }
}

void DuckDuckGo::parseVideo(QXmlStreamReader& xml)
{
    QStringRef name = xml.name();
    QString currentElement;

    QStringList videoTitles;
    QStringList videoLinks;

    kDebug() << "NAME: " << name;

    QXmlStreamAttributes attributes = xml.attributes();
    while (!xml.atEnd() && xml.tokenType() != QXmlStreamReader::EndDocument) {
        kDebug() << "WHILE LOOP(((((((((((((((((()))))))))))))))))), name: " << xml.name();


        kDebug() << "CURRENTELEMENT: " << currentElement;

        if (xml.name() == "title") {
            kDebug() << attributes.value("plain").toString();
        }

        if (xml.name() == "thumbnail") {
            QStringRef attribute = attributes.value("url");
            kDebug() << "ATTRIBUTE: " << attribute;
        }
//            if (name == "title") {
//                kDebug() << "GOT TITLE: " << name;
//                videoTitles.append(xml.readElementText());
//
//            } else if (name == "link") {
//
//                if (xml.attributes().value("rel").toString() == "alternate") {
//                    kDebug() << "ATTRIBUTES: " << xml.attributes().value("href");
//                    const QString& link = xml.attributes().value("href").toString();
//                    if (link != "http://www.youtube.com") {
//                        videoLinks.append(link);
//                    }
//                }
//            }

        xml.readNext();
        attributes = xml.attributes();
        currentElement = xml.readElementText();
        kDebug() << currentElement;
    }

    if (!videoTitles.isEmpty() && !videoLinks.isEmpty()) {
        kDebug() << "TITLE WAS: " << videoTitles;
        kDebug() << "LINK WAS: " << videoLinks;
    }
}

#include "duckduckgo.moc"
