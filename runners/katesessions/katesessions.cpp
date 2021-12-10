/*
 *   SPDX-FileCopyrightText: 2008 Sebastian Kügler <sebas@kde.org>
 *   SPDX-FileCopyrightText: 2017 Kai Uwe Broulik <kde@privat.broulik.de>
 *   SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
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

K_PLUGIN_CLASS_WITH_JSON(KateSessions, "plasma-runner-katesessions.json")

KateSessions::KateSessions(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
    : Plasma::AbstractRunner(parent, metaData, args)
{
    setObjectName(QStringLiteral("Kate Sessions"));

    addSyntax(Plasma::RunnerSyntax(QStringLiteral("kate :q:"), i18n("Finds Kate sessions matching :q:.")));
    addSyntax(Plasma::RunnerSyntax(QStringLiteral("kate"), i18n("Lists all the Kate editor sessions in your account.")));

    m_sessionsFolderPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/kate/sessions");

    // Initialize watchers and sessions
    m_sessionWatch = new KDirWatch(this);
    m_sessionWatch->addDir(m_sessionsFolderPath);
    connect(m_sessionWatch, &KDirWatch::dirty, this, &KateSessions::loadSessions);
    connect(m_sessionWatch, &KDirWatch::created, this, &KateSessions::loadSessions);
    connect(m_sessionWatch, &KDirWatch::deleted, this, &KateSessions::loadSessions);
    loadSessions();
    setTriggerWords({m_triggerWord});
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
    suspendMatching(m_sessions.isEmpty());
}

void KateSessions::match(Plasma::RunnerContext &context)
{
    QString term = context.query();
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

    for (const QString &session : std::as_const(m_sessions)) {
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

    auto *job = new KIO::CommandLauncherJob(QStringLiteral("kate"), {QStringLiteral("--start"), match.data().toString(), QStringLiteral("-n")});
    job->setDesktopName(QStringLiteral("org.kde.kate"));

    auto *delegate = new KNotificationJobUiDelegate;
    delegate->setAutoErrorHandlingEnabled(true);
    job->setUiDelegate(delegate);

    job->start();
}

#include "katesessions.moc"
