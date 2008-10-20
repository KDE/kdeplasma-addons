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


KateSessions::KateSessions(QObject *parent, const QVariantList& args)
    : Plasma::AbstractRunner(parent, args)
{
    Q_UNUSED(args);
    KGlobal::locale()->insertCatalog("krunner_katesessions");
    setObjectName("Kate Sessions");
    m_icon = KIcon("kate");

    loadSessions();

    // listen for changes to the list of kate sessions
    KDirWatch *historyWatch = new KDirWatch(this);
    QStringList sessiondirs = KGlobal::dirs()->findDirs("data", "kate/sessions/");
    foreach (const QString &dir, sessiondirs) {
        historyWatch->addDir(dir);
    }
    connect(historyWatch,SIGNAL(dirty(QString)),this,SLOT(loadSessions()));
    connect(historyWatch,SIGNAL(created(QString)),this,SLOT(loadSessions()));
    connect(historyWatch,SIGNAL(deleted(QString)),this,SLOT(loadSessions()));

}

KateSessions::~KateSessions()
{
}

void KateSessions::loadSessions()
{
    // Switch kate session: -u
    // Should we add a match for this option or would that clutter the matches too much?
    QStringList sessions = QStringList();
    QStringList list = KGlobal::dirs()->findAllResources( "data", "kate/sessions/*.katesession", KStandardDirs::NoDuplicates );
    for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it)
    {
        KConfig _config( *it, KConfig::SimpleConfig );
        KConfigGroup config(&_config, "General" );
        QString name =  config.readEntry( "Name" );
        sessions.append( name );
    }
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

    bool list_all = false;

    if (term.toLower().startsWith("kate")) {
        if (term.toLower().trimmed() == "kate") {
            list_all = true;
        }
        term.replace("kate", "", Qt::CaseInsensitive);
        term = term.trimmed();
    }

    if (term.isEmpty() && !list_all) {
        return;
    }

    foreach (const QString &session, m_sessions) {
        if (list_all || (!term.isEmpty() && session.contains(term, Qt::CaseInsensitive))) {
            Plasma::QueryMatch match(this);
            match.setType(Plasma::QueryMatch::PossibleMatch);
            if (list_all) {
                // All sessions listed, but with a low priority
                match.setRelevance(0.5);
            } else {
                if (session.toLower() == term) {
                    // parameter to kate matches session exactly, bump it up!
                    match.setType(Plasma::QueryMatch::ExactMatch);
                    match.setRelevance(1.0);
                } else {
                    // fuzzy match of the session in "kate $session"
                    match.setRelevance(0.8);
                }
            }
            match.setIcon(m_icon);
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
        args << "--start" << session;
        KToolInvocation::kdeinitExec("kate", args);
    }
}

#include "katesessions.moc"
