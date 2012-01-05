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

#include "konqprofilesengine.h"
#include "konqprofilesservice.h"

#include <KStandardDirs>
#include <KDirWatch>
#include <QFileInfo>
#include <kio/global.h>
#include <KGlobalSettings>
#include <KDebug>

KonqProfilesEngine::KonqProfilesEngine(QObject *parent, const QVariantList &args)
    : Plasma::DataEngine(parent, args),
      m_dirWatch(0)
{

}

KonqProfilesEngine::~KonqProfilesEngine()
{
}

void KonqProfilesEngine::init()
{
    kDebug() << "KonqProfilesDataEngine init";

    m_dirWatch = new KDirWatch( this );
    loadProfiles();
    connect(m_dirWatch, SIGNAL(dirty(QString)), this, SLOT(profilesChanged()));
}

Plasma::Service *KonqProfilesEngine::serviceForSource(const QString &source)
{
    //create a new service for this profile's name, so it can be operated on.
    return new KonqProfilesService(this, source);
}

void KonqProfilesEngine::profilesChanged()
{
    //wipe the data clean, load it again. (there's not a better way of doing this but no big deal)
    removeAllSources();
    loadProfiles();
}

void KonqProfilesEngine::loadProfiles()
{
    const QStringList lst = KGlobal::dirs()->findDirs( "data", "konqueror/profiles/" );
    for ( int i = 0; i < lst.count(); i++ )
    {
        m_dirWatch->addDir( lst[i] );
    }

    const QStringList list = KGlobal::dirs()->findAllResources( "data", "konqueror/profiles/*", KStandardDirs::NoDuplicates );
    const QStringList::ConstIterator end = list.constEnd();

   for (QStringList::ConstIterator it = list.constBegin(); it != end; ++it)
    {
        QFileInfo info(*it);
        const QString profileName = KIO::decodeFileName(info.baseName());
        QString niceName=profileName;
        KConfig cfg(*it, KConfig::SimpleConfig);
        if (cfg.hasGroup("Profile")) {

            KConfigGroup grp(&cfg, "Profile");
            if (grp.hasKey( "Name" )) {
                niceName = grp.readEntry("Name");
            }

            QString sourceName = "name:" + profileName;
            kDebug() << "adding sourcename: " << profileName << " ++" << niceName;
            setData(profileName, "prettyName", niceName);
        }
    }
}

K_EXPORT_PLASMA_DATAENGINE(konqprofilesengine, KonqProfilesEngine)

#include "konqprofilesengine.moc"
