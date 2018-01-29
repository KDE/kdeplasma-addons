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

// KF
#include <KPluginInfo>
#include <KServiceTypeTrader>
#include <KLocalizedString>
// Qt
#include <QMutex>
#include <QEventLoop>
#include <QWaitCondition>
#include <QDesktopServices>
#include <QDebug>


MediaWikiRunner::MediaWikiRunner(QObject *parent, const QVariantList &args)
    : Plasma::AbstractRunner(parent, args)
{
    setObjectName(QStringLiteral("MediaWikiRunner"));

    const QString constraint = QStringLiteral("[X-KDE-PluginInfo-Name] == '%1'").arg(id());
    const KService::List offers = KServiceTypeTrader::self()->query(QStringLiteral("Plasma/Runner"), constraint);

    foreach (const KPluginInfo &info, KPluginInfo::fromServices(offers)) {

        const auto _urls = info.property(QStringLiteral("X-Plasma-Args")).toStringList();
        qDebug() << info.name() << _urls << _urls.count();
        if (_urls.count()) {
            m_apiUrl = QUrl(_urls[0]);
        } else {
            qWarning() << "No X-Plasma-Args found in .desktop file";
        }
        m_name = info.name();
        m_comment = info.comment();
        m_iconName = info.icon();
    }


    addSyntax(Plasma::RunnerSyntax(QStringLiteral("wiki :q:"), i18n("Searches %1 for :q:.", m_name)));

    setSpeed( SlowSpeed );
}

MediaWikiRunner::~MediaWikiRunner()
{
}


void MediaWikiRunner::match(Plasma::RunnerContext &context)
{
    // Check for networkconnection
    if (!m_networkConfigurationManager.isOnline() ||
        !m_apiUrl.isValid()) {
        return;
    }

    QString term = context.query();

    if (!context.singleRunnerQueryMode()) {
        if (!term.startsWith(QLatin1String("wiki "))) {
            return;
        }

        term.remove(0, 5);
    }

    if (term.length() < 3) {
        //qDebug() << "yours is too short" << term;
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
    connect(&mediawiki, &MediaWiki::finished, &loop, &QEventLoop::quit);

    mediawiki.search(term);
    qDebug() << "Wikisearch:" << m_name << term;

    loop.exec();

    if (!context.isValid()) {
        return;
    }
    qreal relevance = 0.5;
    qreal stepRelevance = 0.1;

    foreach(const MediaWiki::Result& res, mediawiki.results()) {
        qDebug() << "Match:" << res.url << res.title;
        Plasma::QueryMatch match(this);
        match.setType(Plasma::QueryMatch::PossibleMatch);
        match.setIconName(m_iconName);
        match.setText(QStringLiteral("%1: %2").arg(m_name, res.title));
        match.setData(res.url);
        match.setRelevance(relevance);
        relevance +=stepRelevance;
        stepRelevance *=0.5;
        context.addMatch(match);
    }
}

void MediaWikiRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context)

    const QString wikiurl = match.data().toUrl().toString();
    qDebug() << "Open MediaWiki page " << wikiurl;

    if (!wikiurl.isEmpty()) {
        QDesktopServices::openUrl(QUrl(wikiurl));
    }
}


K_EXPORT_PLASMA_RUNNER(mediawiki, MediaWikiRunner)

#include "mediawikirunner.moc"
