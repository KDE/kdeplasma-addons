/*
 *   SPDX-FileCopyrightText: 2008 Montel Laurent <montel@kde.org>
 *   based on kate session from sebas
 *   SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "konsoleprofiles.h"

// KF
#include <KConfig>
#include <KDirWatch>
#include <KIO/CommandLauncherJob>
#include <KLocalizedString>
#include <KNotificationJobUiDelegate>
// Qt
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

KonsoleProfiles::KonsoleProfiles(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
    : Plasma::AbstractRunner(parent, metaData, args)
{
    setObjectName(QStringLiteral("Konsole Profiles"));

    Plasma::RunnerSyntax s(QStringLiteral(":q:"), i18n("Finds Konsole profiles matching :q:."));
    s.addExampleQuery(QStringLiteral("konsole :q:"));
    addSyntax(s);
    addSyntax(Plasma::RunnerSyntax(QStringLiteral("konsole"), i18n("Lists all the Konsole profiles in your account.")));
}

KonsoleProfiles::~KonsoleProfiles() = default;

void KonsoleProfiles::init()
{
    // Initialize profiles and file watcher
    m_profileFilesWatch = new KDirWatch(this);
    const QStringList konsoleDataBaseDirs = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    for (const QString &konsoleDataBaseDir : konsoleDataBaseDirs) {
        m_profileFilesWatch->addDir(konsoleDataBaseDir + QStringLiteral("/konsole"));
    }

    connect(m_profileFilesWatch, &KDirWatch::dirty, this, &KonsoleProfiles::loadProfiles);
    connect(m_profileFilesWatch, &KDirWatch::created, this, &KonsoleProfiles::loadProfiles);
    connect(m_profileFilesWatch, &KDirWatch::deleted, this, &KonsoleProfiles::loadProfiles);

    loadProfiles();
    setMinLetterCount(3);
}

void KonsoleProfiles::loadProfiles()
{
    m_profiles.clear();

    QStringList profilesPaths;
    const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("konsole"), QStandardPaths::LocateDirectory);

    for (const auto &dir : dirs) {
        const QStringList fileNames = QDir(dir).entryList({QStringLiteral("*.profile")});
        for (const QString &fileName : fileNames) {
            profilesPaths.append(dir + QLatin1Char('/') + fileName);
        }
    }

    for (const auto &profilePath : qAsConst(profilesPaths)) {
        const QString profileName = QFileInfo(profilePath).baseName();

        const KConfig _config(profilePath, KConfig::SimpleConfig);
        if (_config.hasGroup("General")) {
            KonsoleProfileData profileData;
            const KConfigGroup cfg = _config.group("General");
            profileData.displayName = cfg.readEntry("Name", profileName);
            profileData.iconName = cfg.readEntry("Icon", QStringLiteral("utilities-terminal"));
            if (!profileData.displayName.isEmpty()) {
                m_profiles.append(profileData);
            }
        }
    }
    suspendMatching(m_profiles.isEmpty());
}

void KonsoleProfiles::match(Plasma::RunnerContext &context)
{
    QString term = context.query();
    term = term.remove(m_triggerWord).simplified();
    for (const KonsoleProfileData &data : qAsConst(m_profiles)) {
        if (data.displayName.contains(term, Qt::CaseInsensitive)) {
            Plasma::QueryMatch match(this);
            match.setType(Plasma::QueryMatch::PossibleMatch);
            match.setIconName(data.iconName);
            match.setData(data.displayName);
            match.setText(QStringLiteral("Konsole: ") + data.displayName);
            match.setRelevance((float)term.length() / (float)data.displayName.length());
            context.addMatch(match);
        }
    }
}
void KonsoleProfiles::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context)
    const QString profile = match.data().toString();

    auto *job = new KIO::CommandLauncherJob(QStringLiteral("konsole"), {QStringLiteral("--profile"), profile});
    job->setDesktopName(QStringLiteral("org.kde.konsole"));

    auto *delegate = new KNotificationJobUiDelegate;
    delegate->setAutoErrorHandlingEnabled(true);
    job->setUiDelegate(delegate);

    job->start();
}

K_PLUGIN_CLASS_WITH_JSON(KonsoleProfiles, "plasma-runner-konsoleprofiles.json")

#include "konsoleprofiles.moc"
