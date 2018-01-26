/*
 *   Copyright 2008 Sebastian Kügler <sebas@kde.org>
 *   Copyright 2017 Kai Uwe Broulik <kde@privat.broulik.de>
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

#include <QCollator>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

#include <KDirWatch>
#include <KLocalizedString>
#include <KToolInvocation>

K_EXPORT_PLASMA_RUNNER(katesessionsrunner, KateSessions)

KateSessions::KateSessions(QObject *parent, const QVariantList& args)
    : Plasma::AbstractRunner(parent, args),
    m_sessionWatch(nullptr)
{
    setObjectName(QLatin1String("Kate Sessions"));
    setIgnoredTypes(Plasma::RunnerContext::File | Plasma::RunnerContext::Directory | Plasma::RunnerContext::NetworkLocation);

    Plasma::RunnerSyntax s(QLatin1String(":q:"), i18n("Finds Kate sessions matching :q:."));
    s.addExampleQuery(QLatin1String("kate :q:"));
    addSyntax(s);

    setDefaultSyntax(Plasma::RunnerSyntax(QLatin1String("kate"), i18n("Lists all the Kate editor sessions in your account.")));

    m_sessionsFolderPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
                         + QLatin1String("/kate/sessions");

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
        m_sessionWatch = new KDirWatch(this);
        m_sessionWatch->addDir(m_sessionsFolderPath);
        connect(m_sessionWatch, &KDirWatch::dirty, this, &KateSessions::loadSessions);
        connect(m_sessionWatch, &KDirWatch::created, this, &KateSessions::loadSessions);
        connect(m_sessionWatch, &KDirWatch::deleted, this, &KateSessions::loadSessions);
    }
}

void KateSessions::slotTeardown()
{
    delete m_sessionWatch;
    m_sessionWatch = nullptr;
    m_sessions.clear();
}

void KateSessions::loadSessions()
{
    QStringList sessions;

    QDir sessionsDir(m_sessionsFolderPath);

    const auto &sessionFiles = sessionsDir.entryInfoList({QStringLiteral("*.katesession")}, QDir::Files);

    for (const QFileInfo &sessionFile : sessionFiles) {
        const QString name = QUrl::fromPercentEncoding(sessionFile.baseName().toLocal8Bit()); // is this the right encoding?
        sessions.append(name);
    }

    QCollator collator;
    collator.setCaseSensitivity(Qt::CaseInsensitive);
    std::sort(sessions.begin(), sessions.end(), [&collator](const QString &a, const QString &b) {
        return collator.compare(a, b) < 0;
    });

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
            match.setIconName(QStringLiteral("kate"));
            match.setData(session);
            match.setText(session);
            match.setSubtext(i18n("Open Kate Session"));
            context.addMatch(match);
        }
    }
}

void KateSessions::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context)
    QString session = match.data().toString();

    if (!session.isEmpty()) {
        QStringList args;
       	args << QLatin1String("--start") << session << QLatin1String("-n");
        KToolInvocation::kdeinitExec(QLatin1String("kate"), args);
    }
}

#include "katesessions.moc"
