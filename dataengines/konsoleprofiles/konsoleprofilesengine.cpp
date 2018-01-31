/*****************************************************************************
*   Copyright (C) 2011 by Shaun Reich <shaun.reich@kdemail.net>              *
*   Copyright (C) 2008 by Montel Laurent <montel@kde.org>                    *
*                                                                            *
*   This program is free software; you can redistribute it and/or            *
*   modify it under the terms of the GNU General Public License as           *
*   published by the Free Software Foundation; either version 2 of           *
*   the License, or (at your option) any later version.                      *
*                                                                            *
*   This program is distributed in the hope that it will be useful,          *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
*   GNU General Public License for more details.                             *
*                                                                            *
*   You should have received a copy of the GNU General Public License        *
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
*****************************************************************************/

#include "konsoleprofilesengine.h"
#include "konsoleprofilesservice.h"

// KF
#include <KDirWatch>
#include <KConfig>
// Qt
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>


KonsoleProfilesEngine::KonsoleProfilesEngine(QObject *parent, const QVariantList &args)
    : Plasma::DataEngine(parent, args),
      m_dirWatch(nullptr)
{
    init();
}

KonsoleProfilesEngine::~KonsoleProfilesEngine()
{
}

void KonsoleProfilesEngine::init()
{
    qDebug() << "KonsoleProfilesDataEngine init";

    m_dirWatch = new KDirWatch( this );
    const QStringList konsoleDataBaseDirs = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    for (const QString& konsoleDataBaseDir : konsoleDataBaseDirs) {
        m_dirWatch->addDir(konsoleDataBaseDir + QLatin1String("/konsole"));
    }
    connect(m_dirWatch, &KDirWatch::dirty, this, &KonsoleProfilesEngine::profilesChanged);

    loadProfiles();
}

Plasma::Service *KonsoleProfilesEngine::serviceForSource(const QString &source)
{
    //create a new service for this profile's name, so it can be operated on.
    return new KonsoleProfilesService(this, source);
}

void KonsoleProfilesEngine::profilesChanged()
{
    //wipe the data clean, load it again. (there's not a better way of doing this but no big deal)
    removeAllSources();
    loadProfiles();
}

void KonsoleProfilesEngine::loadProfiles()
{
    QStringList profilesPaths;
    const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("konsole"), QStandardPaths::LocateDirectory);

    for (const auto& dir : dirs) {
        const QStringList fileNames = QDir(dir).entryList({QStringLiteral("*.profile")});
        for (const QString& fileName : fileNames) {
            profilesPaths.append(dir + QLatin1Char('/') + fileName);
        }
    }

    for (const auto& profilePath : qAsConst(profilesPaths)) {
        QFileInfo info(profilePath);
        const QString profileName = info.baseName();
        QString niceName = profileName;
        KConfig cfg(profilePath, KConfig::SimpleConfig);

        if ( cfg.hasGroup( "General" ) ) {
            KConfigGroup grp( &cfg, "General" );

            if ( grp.hasKey( "Name" ) ) {
                niceName = grp.readEntry( "Name" );
            }

            qDebug() << "adding sourcename: " << profileName << " ++" << niceName;
            setData(profileName, QStringLiteral("prettyName"), niceName);
        }
    }
}

K_EXPORT_PLASMA_DATAENGINE_WITH_JSON(konsoleprofilesengine, KonsoleProfilesEngine, "plasma-dataengine-konsoleprofiles.json")

#include "konsoleprofilesengine.moc"
