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
#include <KFuzzyMatcher>
#include <KLocalizedString>
#include <KNotificationJobUiDelegate>

#include <KIO/CommandLauncherJob>

K_PLUGIN_CLASS_WITH_JSON(KateSessions, "plasma-runner-katesessions.json")

KateSessions::KateSessions(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
    : AbstractRunner(parent, metaData, args)
{
    setObjectName(QStringLiteral("Kate Sessions"));

    addSyntax(QStringLiteral("kate :q:"), i18n("Finds Kate sessions matching :q:."));
    addSyntax(QStringLiteral("kate"), i18n("Lists all the Kate editor sessions in your account."));

    // Initialize watchers and sessions
    setTriggerWords({m_triggerWord});

    m_model.setAppName(m_triggerWord);
}

void KateSessions::match(RunnerContext &context)
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

    QList<QueryMatch> matches;
    int maxScore = 0;

    for (int i = 0, count = m_model.rowCount(); i < count; ++i) {
        // Does the query match exactly?
        // no query = perfect match => list everything
        QString session = m_model.index(i).data(ProfilesModel::NameRole).toString();
        if (listAll || session.compare(term, Qt::CaseInsensitive) == 0) {
            QueryMatch match(this);
            match.setType(QueryMatch::ExactMatch);
            match.setRelevance(session.compare(term, Qt::CaseInsensitive) == 0 ? 1 : 0.8);
            match.setIconName(m_triggerWord);
            match.setData(session);
            match.setText(session);
            match.setSubtext(i18n("Open Kate Session"));
            context.addMatch(match);
        } else {
            // Do fuzzy matching
            const auto res = KFuzzyMatcher::match(term, session);
            if (res.matched) {
                QueryMatch match(this);
                match.setRelevance(res.score); // store the score here for now
                match.setIconName(m_triggerWord);
                match.setData(session);
                match.setText(session);
                match.setSubtext(i18n("Open Kate Session"));
                matches.push_back(match);
                maxScore = std::max(res.score, maxScore);
            }
        }
    }

    auto calculate_relevance = [maxScore](double score) {
        return score / maxScore;
    };
    for (auto &match : matches) {
        match.setRelevance(calculate_relevance(match.relevance()));
    }
    context.addMatches(matches);
}

void KateSessions::run(const RunnerContext &context, const QueryMatch &match)
{
    Q_UNUSED(context)

    m_model.openProfile(match.data().toString());
}

#include "katesessions.moc"
