/*
 *   Copyright 2008 Sebastian Kügler <sebas@kde.org>
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

#include "katesessions.h"


#include <KDebug>
#include <KDirWatch>
#include <KStandardDirs>
#include <KToolInvocation>
#include <KIcon>
#include <KConfig>
#include <KConfigGroup>
#include <KUrl>
#include <KStringHandler>
#include <QFile>

bool katesessions_runner_compare_sessions(const QString &s1, const QString &s2) {
    return KStringHandler::naturalCompare(s1,s2)==-1;
}

KateSessions::KateSessions(QObject *parent, const QVariantList& args)
    : Plasma::AbstractRunner(parent, args),
    m_sessionWatch(0)
{
    setObjectName(QLatin1String("Kate Sessions"));
    setIgnoredTypes(Plasma::RunnerContext::File | Plasma::RunnerContext::Directory | Plasma::RunnerContext::NetworkLocation);

    Plasma::RunnerSyntax s(QLatin1String(":q:"), i18n("Finds Kate sessions matching :q:."));
    s.addExampleQuery(QLatin1String("kate :q:"));
    addSyntax(s);

    setDefaultSyntax(Plasma::RunnerSyntax(QLatin1String("kate"), i18n("Lists all the Kate editor sessions in your account.")));

    connect(this, SIGNAL(prepare()), SLOT(slotPrepare()));
    connect(this, SIGNAL(teardown()), SLOT(slotTeardown()));
}

KateSessions::~KateSessions()
{
}

void KateSessions::slotPrepare()
{
    loadSessions();

    // listen for changes to the list of kate sessions
    if (!m_sessionWatch) {
        KDirWatch *m_sessionWatch = new KDirWatch(this);
        const QStringList sessiondirs = KGlobal::dirs()->findDirs("data", QLatin1String("kate/sessions/"));
        foreach (const QString &dir, sessiondirs) {
            m_sessionWatch->addDir(dir);
        }
        connect(m_sessionWatch,SIGNAL(dirty(QString)),this,SLOT(loadSessions()));
        connect(m_sessionWatch,SIGNAL(created(QString)),this,SLOT(loadSessions()));
        connect(m_sessionWatch,SIGNAL(deleted(QString)),this,SLOT(loadSessions()));
    }
}

void KateSessions::slotTeardown()
{
    delete m_sessionWatch;
    m_sessionWatch = 0;
    m_sessions.clear();
}

void KateSessions::loadSessions()
{
    // Switch kate session: -u
    // Should we add a match for this option or would that clutter the matches too much?
    QStringList sessions = QStringList();
    const QStringList list = KGlobal::dirs()->findAllResources( "data", QLatin1String("kate/sessions/*.katesession"), KStandardDirs::NoDuplicates );
    KUrl url;
    for (QStringList::ConstIterator it = list.constBegin(); it != list.constEnd(); ++it)
    {
/*        KConfig _config( *it, KConfig::SimpleConfig );
        KConfigGroup config(&_config, "General" );
        QString name =  config.readEntry( "Name" );*/
        url.setPath(*it);
        QString name=url.fileName();
        name = QUrl::fromPercentEncoding(QFile::encodeName(url.fileName()));
        name.chop(12);///.katesession==12
        sessions.append( name );
    }
    qSort(sessions.begin(),sessions.end(),katesessions_runner_compare_sessions);
    m_sessions = sessions;
}

void KateSessions::match(Plasma::RunnerContext &context)
{
    if (m_sessions.isEmpty()) {
        return;
    }

    QString term = context.query();
    if (term.length() < 3) {
        return;
    }

    bool listAll = false;

    if (term.startsWith(QLatin1String("kate"), Qt::CaseInsensitive)) {
        if (term.trimmed().compare(QLatin1String("kate"), Qt::CaseInsensitive) == 0) {
            listAll = true;
            term.clear();
        } else if (term.at(4) == QLatin1Char(' ') ) {
            term.remove(QLatin1String("kate"), Qt::CaseInsensitive);
            term = term.trimmed();
        } else {
            term.clear();
        }
    }

    if (term.isEmpty() && !listAll) {
        return;
    }

    foreach (const QString &session, m_sessions) {
        if (!context.isValid()) {
            return;
        }

        if (listAll || (!term.isEmpty() && session.contains(term, Qt::CaseInsensitive))) {
            Plasma::QueryMatch match(this);
            if (listAll) {
                // All sessions listed, but with a low priority
                match.setType(Plasma::QueryMatch::ExactMatch);
                match.setRelevance(0.8);
            } else {
                if (session.compare(term, Qt::CaseInsensitive) == 0) {
                    // parameter to kate matches session exactly, bump it up!
                    match.setType(Plasma::QueryMatch::ExactMatch);
                    match.setRelevance(1.0);
                } else {
                    // fuzzy match of the session in "kate $session"
                    match.setType(Plasma::QueryMatch::PossibleMatch);
                    match.setRelevance(0.8);
                }
            }
            match.setIcon(KIcon(QLatin1String("kate")));
            match.setData(session);
            match.setText(session);
            match.setSubtext(i18n("Open Kate Session"));
            context.addMatch(term, match);
        }
    }
}

void KateSessions::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context)
    QString session = match.data().toString();
    kDebug() << "Open Kate Session " << session;

    if (!session.isEmpty()) {
        QStringList args;
       	args << QLatin1String("--start") << session << QLatin1String("-n");
        KToolInvocation::kdeinitExec(QLatin1String("kate"), args);
    }
}

#include "katesessions.moc"
