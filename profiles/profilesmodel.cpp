/*
    SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "profilesmodel.h"

// KF
#include <KConfig>
#include <KConfigGroup>
#include <KDirWatch>
// Qt
#include <KIO/CommandLauncherJob>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

ProfilesModel::ProfilesModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

void ProfilesModel::init()
{
    m_dirWatch = new KDirWatch(this);
    const QStringList konsoleDataBaseDirs = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    for (const QString &konsoleDataBaseDir : konsoleDataBaseDirs) {
        m_dirWatch->addDir(konsoleDataBaseDir + QLatin1Char('/') + m_appName);
    }
    connect(m_dirWatch, &KDirWatch::dirty, this, &ProfilesModel::loadProfiles);

    loadProfiles();
}

void ProfilesModel::loadProfiles()
{
    QStringList profilesPaths;
    const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, m_appName, QStandardPaths::LocateDirectory);

    for (const auto &dir : dirs) {
        const QStringList fileNames = QDir(dir).entryList({QStringLiteral("*.profile")});
        for (const QString &fileName : fileNames) {
            profilesPaths.append(dir + QLatin1Char('/') + fileName);
        }
    }

    beginResetModel();
    m_data.clear();
    for (const auto &profilePath : std::as_const(profilesPaths)) {
        QFileInfo info(profilePath);
        const QString profileIdentifier = info.baseName();
        QString niceName = profileIdentifier;
        KConfig cfg(profilePath, KConfig::SimpleConfig);

        if (cfg.hasGroup("General")) {
            KConfigGroup grp(&cfg, "General");

            if (grp.hasKey("Name")) {
                niceName = grp.readEntry("Name");
            }

            m_data.append(ProfileData{niceName, profileIdentifier});
        }
    }
    endResetModel();
}

QHash<int, QByteArray> ProfilesModel::roleNames() const
{
    return {
        {NameRole, "name"},
        {ProfileIdentifierRole, "profileIdentifier"},
    };
}

QVariant ProfilesModel::data(const QModelIndex &index, int role) const
{
    const ProfileData &data = m_data.at(index.row());
    switch (role) {
    case NameRole:
        return data.name;
    case ProfileIdentifierRole:
        return data.profileIdentifier;
    default:
        return QVariant();
    }
}

void ProfilesModel::openProfile(const QString profileIdentifier)
{
    KIO::CommandLauncherJob *job;
    if (m_appName == QLatin1String("konsole")) {
        job = new KIO::CommandLauncherJob(m_appName, QStringList{QStringLiteral("--profile"), profileIdentifier});
    } else {
        Q_UNREACHABLE();
    }

    job->start();
}
