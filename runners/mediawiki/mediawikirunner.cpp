/*
 *   Copyright 2009 Sebastian K?gler <sebas@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "mediawikirunner.h"
#include "mediawiki.h"

#include <QMutex>
#include <QtCore/QEventLoop>
#include <QWaitCondition>

#include <KDebug>
#include <KIcon>
#include <KPluginInfo>
#include <KServiceTypeTrader>
#include <KToolInvocation>
#include <solid/networking.h>

MediaWikiRunner::MediaWikiRunner(QObject *parent, const QVariantList& args)
    : Plasma::AbstractRunner(parent, args)
{
    Q_UNUSED(args);
    setObjectName("MediaWikiRunner");

    const QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(id());
    const KService::List offers = KServiceTypeTrader::self()->query("Plasma/Runner", constraint);

    foreach (const KPluginInfo &info, KPluginInfo::fromServices(offers)) {

        QStringList _urls = info.property("X-Plasma-Args").toStringList();
        QString _u;
        kDebug() << info.name() << info.property("X-Plasma-Args").toStringList() << _urls.count();
        if (_urls.count()) {
            m_apiUrl = _urls[0];
        } else {
            kWarning() << "No X-Plasma-Args found in .desktop file";
        }
        m_name = info.name();
        m_comment = info.comment();
        m_icon = KIcon(info.icon());
    }


    addSyntax(Plasma::RunnerSyntax("wiki :q:", i18n("Searches %1 for :q:.", m_name)));

    setSpeed( SlowSpeed );
}

MediaWikiRunner::~MediaWikiRunner()
{
}


void MediaWikiRunner::match(Plasma::RunnerContext &context)
{
    // Check for networkconnection
    if(Solid::Networking::status() == Solid::Networking::Unconnected) {
        return;
    }

    QString term = context.query();
    if (!context.singleRunnerQueryMode()) {
        if (!term.startsWith("wiki ")) {
            return;
        } else {
            term = term.remove("wiki ");
        }
    }
    if (!m_apiUrl.isValid() || term.length() < 3) {
        //kDebug() << "yours is too short" << term;
        return;
    }

    QEventLoop loop;
    // Wait a second, we don't want to  query on every keypress
    QMutex mutex;
    QWaitCondition waiter;
    mutex.lock();
    waiter.wait(&mutex, 1000);
    mutex.unlock();

    if (!context.isValid()) {
        return;
    }

    MediaWiki mediawiki;
    if (context.singleRunnerQueryMode()) {
        mediawiki.setMaxItems(10);
    } else {
        mediawiki.setMaxItems(3);
    }
    mediawiki.setApiUrl( m_apiUrl );
    connect( &mediawiki, SIGNAL(finished(bool)), &loop, SLOT(quit()) );

    mediawiki.search(term);
    kDebug() << "Wikisearch:" << m_name << term;

    loop.exec();

    if (!context.isValid()) {
        return;
    }
    qreal relevance = 0.5;
    qreal stepRelevance = 0.1;

    foreach(const MediaWiki::Result& res, mediawiki.results()) {
        kDebug() << "Match:" << res.url << res.title;
        Plasma::QueryMatch match(this);
        match.setType(Plasma::QueryMatch::PossibleMatch);
        match.setIcon(m_icon);
        match.setText(QString("%1: %2").arg(m_name, res.title));
        match.setData(res.url);
        match.setRelevance(relevance);
        relevance +=stepRelevance;
        stepRelevance *=0.5;
        context.addMatch(res.title, match);
    }
}

void MediaWikiRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context)
    const QString wikiurl = match.data().toUrl().toString();
    kDebug() << "Open MediaWiki page " << wikiurl;

    if (!wikiurl.isEmpty()) {
        QStringList args;
        args << "openURL" << wikiurl;
        KToolInvocation::kdeinitExec("kfmclient", args);
    }
}

#include "mediawikirunner.moc"
