/*
 *  SPDX-FileCopyrightText: 2011 Shaun Reich <shaun.reich@kdemail.net>
 *  SPDX-FileCopyrightText: 2008 Montel Laurent <montel@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "konsoleprofilesengine.h"
#include "konsoleprofilesservice.h"

// KF
#include <KConfig>
#include <KDirWatch>
// Qt
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

KonsoleProfilesEngine::KonsoleProfilesEngine(QObject *parent, const QVariantList &args)
    : Plasma::DataEngine(parent, args)
    , m_dirWatch(nullptr)
{
    init();
}

KonsoleProfilesEngine::~KonsoleProfilesEngine()
{
}

void KonsoleProfilesEngine::init()
{
    qDebug() << "KonsoleProfilesDataEngine init";

    m_dirWatch = new KDirWatch(this);
    const QStringList konsoleDataBaseDirs = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    for (const QString &konsoleDataBaseDir : konsoleDataBaseDirs) {
        m_dirWatch->addDir(konsoleDataBaseDir + QLatin1String("/konsole"));
    }
    connect(m_dirWatch, &KDirWatch::dirty, this, &KonsoleProfilesEngine::profilesChanged);

    loadProfiles();
}

Plasma::Service *KonsoleProfilesEngine::serviceForSource(const QString &source)
{
    // create a new service for this profile's name, so it can be operated on.
    return new KonsoleProfilesService(this, source);
}

void KonsoleProfilesEngine::profilesChanged()
{
    // wipe the data clean, load it again. (there's not a better way of doing this but no big deal)
    removeAllSources();
    loadProfiles();
}

void KonsoleProfilesEngine::loadProfiles()
{
    QStringList profilesPaths;
    const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("konsole"), QStandardPaths::LocateDirectory);

    for (const auto &dir : dirs) {
        const QStringList fileNames = QDir(dir).entryList({QStringLiteral("*.profile")});
        for (const QString &fileName : fileNames) {
            profilesPaths.append(dir + QLatin1Char('/') + fileName);
        }
    }

    for (const auto &profilePath : qAsConst(profilesPaths)) {
        QFileInfo info(profilePath);
        const QString profileName = info.baseName();
        QString niceName = profileName;
        KConfig cfg(profilePath, KConfig::SimpleConfig);

        if (cfg.hasGroup("General")) {
            KConfigGroup grp(&cfg, "General");

            if (grp.hasKey("Name")) {
                niceName = grp.readEntry("Name");
            }

            qDebug() << "adding sourcename: " << profileName << " ++" << niceName;
            setData(profileName, QStringLiteral("prettyName"), niceName);
        }
    }
}

K_PLUGIN_CLASS_WITH_JSON(KonsoleProfilesEngine, "plasma-dataengine-konsoleprofiles.json")

#include "konsoleprofilesengine.moc"
