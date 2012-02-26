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
#include "tubejob.h"

#include <KDebug>
#include <KToolInvocation>

#include <QtCore/QTimer>
#include <QtCore/QWaitCondition>
#include <QtCore/QEventLoop>
#include <qjson/parser.h>

//TODO: I'd really *love* to be able to embed a video *inside* krunner. you know how sexy that'd be? answer: very much.
//but seeing as youtube doesn't fully support html5 (only for non-ad'ed videos), i guess i'll have to hold off on it?
YouTube::YouTube(QObject *parent, const QVariantList& args)
    : Plasma::AbstractRunner(parent, args)
{
    Q_UNUSED(args);
    setObjectName(QLatin1String("YouTube"));
    setIgnoredTypes(Plasma::RunnerContext::FileSystem | Plasma::RunnerContext::Directory | Plasma::RunnerContext::NetworkLocation);

    Plasma::RunnerSyntax s(QLatin1String(":q:"), i18n("Finds YouTube video matching :q:."));
    s.addExampleQuery(QLatin1String("youtube :q:"));
    addSyntax(s);

    addSyntax(Plasma::RunnerSyntax(QLatin1String("youtube"), i18n("Lists the videos matching the query, using YouTube search")));
    setSpeed(SlowSpeed);
    setPriority(LowPriority);
}

YouTube::~YouTube()
{
}

void YouTube::match(Plasma::RunnerContext &context)
{
    kDebug() << "YouTube Runner, MATCH MADE";

    const QString term = context.query();

    if (term.length() < 3) {
        return;
    }

    if (!context.isValid()) {
        return;
    }

    kDebug() << "YouTube Runner, Sleeping for 1 seconds";

    QEventLoop loop;
    // Wait a second, we don't want to  query on every keypress
    QMutex mutex;
    QWaitCondition waiter;
    mutex.lock();
    waiter.wait(&mutex, 1000);
    mutex.unlock();

    TubeJob tubeJob(term);
    connect(&tubeJob, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    parseJson(tubeJob.data());
}

void YouTube::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
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

void YouTube::parseJson(const QByteArray& data)
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

#include "youtube.moc"
