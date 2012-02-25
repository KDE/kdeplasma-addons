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

#include "youtube.h"

#include <KDebug>
#include <KToolInvocation>

#include <QXmlStreamReader>


YouTube::YouTube(QObject *parent, const QVariantList& args)
    : Plasma::AbstractRunner(parent, args)
{
    Q_UNUSED(args);
    setObjectName(QLatin1String("YouTube"));
    setIgnoredTypes(Plasma::RunnerContext::FileSystem | Plasma::RunnerContext::Directory | Plasma::RunnerContext::NetworkLocation);

    Plasma::RunnerSyntax s(QLatin1String( ":q:" ), i18n("Finds YouTube video matching :q:."));
    s.addExampleQuery(QLatin1String("youtube :q:"));
    addSyntax(s);

    addSyntax(Plasma::RunnerSyntax(QLatin1String( "youtube" ), i18n("Lists the videos matching the query, using YouTube search")));
    setSpeed(SlowSpeed);
}

YouTube::~YouTube()
{
}

void YouTube::match(Plasma::RunnerContext &context)
{
    const QString term = context.query();
    if (term.length() < 3) {
        return;
    }

    Plasma::QueryMatch match(this);
    match.setType(Plasma::QueryMatch::PossibleMatch);
    //  match.setRelevance(1.0);
    //  match.setIcon(m_icon);
    //  match.setData(i.key());
    match.setText(QLatin1String( "YouTube: " ));

    context.addMatch(term, match);
}

void YouTube::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    kDebug() << "%%%%%% YOUTUBE RUNNING JORB!";
    KIO::TransferJob *job = KIO::get(KUrl("http://gdata.youtube.com/feeds/api/videos?max-results=1&q=taylor swift"));
    connect(job, SIGNAL(data(KIO::Job*,QByteArray)), this, SLOT(dataArrived(KIO::Job*,QByteArray)));
    job->start();

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

void YouTube::dataArrived(KIO::Job* job, const QByteArray& data)
{
    if (!data.isEmpty()) {
        parseXML(data);
    }
}

void YouTube::parseXML(QByteArray data)
{
    QXmlStreamReader xml(data);

    if (xml.hasError()) {
        kError() << "YouTube Runner xml parse failure";
        return;
    }

    while (!xml.atEnd()) {
        QXmlStreamReader::TokenType token = xml.readNext();

        if (token == QXmlStreamReader::StartDocument) {
            continue;
        }

        if (token == QXmlStreamReader::StartElement) {
            parseVideo(xml);
        }
    }
}

void YouTube::parseVideo(QXmlStreamReader& xml)
{
    QStringRef name = xml.name();

    QStringList videoTitles;
    QStringList videoLinks;

    kDebug() << "NAME: " << name;

    if (name == "group") {
        while (!xml.atEnd() && xml.tokenType() != QXmlStreamReader::EndDocument) {
            kDebug() << "WHILE LOOP(((((((((((((((((()))))))))))))))))), name: " << xml.name();

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
            xml.readElementText();
        }
    }



    if (!videoTitles.isEmpty() && !videoLinks.isEmpty()) {
        kDebug() << "TITLE WAS: " << videoTitles;
        kDebug() << "LINK WAS: " << videoLinks;
    }
}

#include "youtube.moc"
