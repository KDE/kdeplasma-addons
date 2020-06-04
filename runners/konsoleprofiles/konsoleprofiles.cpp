/*
 *   Copyright 2008 Montel Laurent <montel@kde.org>
 *   based on kate session from sebas
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

#include "konsoleprofiles.h"

// KF
#include <KIO/CommandLauncherJob>
#include <KConfig>
#include <KDirWatch>
#include <KLocalizedString>
#include <KNotificationJobUiDelegate>
// Qt
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>


KonsoleProfiles::KonsoleProfiles(QObject *parent, const QVariantList &args)
    : Plasma::AbstractRunner(parent, args)
{
    setObjectName(QStringLiteral("Konsole Profiles"));

    setIgnoredTypes(Plasma::RunnerContext::File
                        | Plasma::RunnerContext::Directory
                        | Plasma::RunnerContext::NetworkLocation);

    Plasma::RunnerSyntax s(QStringLiteral(":q:"), i18n("Finds Konsole profiles matching :q:."));
    s.addExampleQuery(QStringLiteral("konsole :q:"));
    addSyntax(s);
    setDefaultSyntax(Plasma::RunnerSyntax(QStringLiteral("konsole"),
                                          i18n("Lists all the Konsole profiles in your account.")));
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
}

void KonsoleProfiles::loadProfiles()
{
    m_profiles.clear();

    QStringList profilesPaths;
    const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                                                       QStringLiteral("konsole"), QStandardPaths::LocateDirectory);

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
    if (term.length() < 3 || !context.isValid()) {
        return;
    }

    term = term.remove(m_triggerWord).simplified();
    for (const KonsoleProfileData &data: qAsConst(m_profiles)) {
        if (data.displayName.contains(term, Qt::CaseInsensitive)) {
            Plasma::QueryMatch match(this);
            match.setType(Plasma::QueryMatch::PossibleMatch);
            match.setIconName(data.iconName);
            match.setData(data.displayName);
            match.setText(QStringLiteral("Konsole: ") + data.displayName);
            match.setRelevance((float) term.length() / (float) data.displayName.length());
            context.addMatch(match);
        }
    }
}
void KonsoleProfiles::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context)
    const QString profile = match.data().toString();

    auto *job = new KIO::CommandLauncherJob(QStringLiteral("konsole"), {
        QStringLiteral("--profile"), profile
    });
    job->setDesktopName(QStringLiteral("org.kde.konsole"));

    auto *delegate = new KNotificationJobUiDelegate;
    delegate->setAutoErrorHandlingEnabled(true);
    job->setUiDelegate(delegate);

    job->start();
}

K_EXPORT_PLASMA_RUNNER_WITH_JSON(KonsoleProfiles, "plasma-runner-konsoleprofiles.json")

#include "konsoleprofiles.moc"
