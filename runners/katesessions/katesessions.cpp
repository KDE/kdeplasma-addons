/*
 *   Copyright 2008 Sebastian Kügler <sebas@kde.org>
 *   Copyright 2017 Kai Uwe Broulik <kde@privat.broulik.de>
 *   Copyright 2020  Alexander Lohnau <alexander.lohnau@gmx.de>
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
#include <KNotificationJobUiDelegate>

#include <KIO/CommandLauncherJob>

K_EXPORT_PLASMA_RUNNER_WITH_JSON(KateSessions, "plasma-runner-katesessions.json")

KateSessions::KateSessions(QObject *parent, const QVariantList &args)
    : Plasma::AbstractRunner(parent, args)
{
    setObjectName(QStringLiteral("Kate Sessions"));
    setIgnoredTypes(Plasma::RunnerContext::File | Plasma::RunnerContext::Directory
                        | Plasma::RunnerContext::NetworkLocation);

    Plasma::RunnerSyntax s(QStringLiteral("kate :q:"), i18n("Finds Kate sessions matching :q:."));
    addSyntax(s);
    setDefaultSyntax(Plasma::RunnerSyntax(QStringLiteral("kate"),
                                          i18n("Lists all the Kate editor sessions in your account.")));

    m_sessionsFolderPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
                            + QStringLiteral("/kate/sessions");

    // Initialize watchers and sessions
    m_sessionWatch = new KDirWatch(this);
    m_sessionWatch->addDir(m_sessionsFolderPath);
    connect(m_sessionWatch, &KDirWatch::dirty, this, &KateSessions::loadSessions);
    connect(m_sessionWatch, &KDirWatch::created, this, &KateSessions::loadSessions);
    connect(m_sessionWatch, &KDirWatch::deleted, this, &KateSessions::loadSessions);
    loadSessions();
}

KateSessions::~KateSessions()
{
}

void KateSessions::loadSessions()
{
    QStringList sessions;
    const QDir sessionsDir(m_sessionsFolderPath);

    const auto &sessionFiles = sessionsDir.entryInfoList({QStringLiteral("*.katesession")}, QDir::Files, QDir::Name);
    for (const QFileInfo &sessionFile : sessionFiles) {
        sessions.append(QUrl::fromPercentEncoding(sessionFile.baseName().toLocal8Bit()));
    }

    m_sessions = sessions;
}

void KateSessions::match(Plasma::RunnerContext &context)
{
    QString term = context.query();
    if (term.length() < 3 || m_sessions.isEmpty() || !context.isValid()) {
        return;
    }
    // Kate writes sessions as desktop actions in the local .desktop file =>
    // they are already available from the "Applications" Runner and in the normal launcher
    if (!term.startsWith(m_triggerWord, Qt::CaseInsensitive)) {
        return;
    }

    bool listAll = false;
    if (term.trimmed().compare(m_triggerWord, Qt::CaseInsensitive) == 0) {
        listAll = true;
        term.clear();
    } else if (term.at(4) == QLatin1Char(' ')) {
        term = term.remove(m_triggerWord, Qt::CaseInsensitive).trimmed();
    } else {
        // Prevent results for queries like "katee"
        return;
    }

    for (const QString &session: qAsConst(m_sessions)) {
        if (listAll || session.contains(term, Qt::CaseInsensitive)) {
            Plasma::QueryMatch match(this);
            match.setType(Plasma::QueryMatch::ExactMatch);
            match.setRelevance(session.compare(term, Qt::CaseInsensitive) == 0 ? 1 : 0.8);
            match.setIconName(m_triggerWord);
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

    auto *job = new KIO::CommandLauncherJob(QStringLiteral("kate"), {
        QStringLiteral("--start"), match.data().toString(), QStringLiteral("-n")
    });
    job->setDesktopName(QStringLiteral("org.kde.kate"));

    auto *delegate = new KNotificationJobUiDelegate;
    delegate->setAutoErrorHandlingEnabled(true);
    job->setUiDelegate(delegate);

    job->start();
}

#include "katesessions.moc"
