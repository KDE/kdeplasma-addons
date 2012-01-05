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

#include <KStandardDirs>
#include <KDirWatch>
#include <QFileInfo>
#include <kio/global.h>
#include <KGlobalSettings>
#include <KDebug>

KonsoleProfilesEngine::KonsoleProfilesEngine(QObject *parent, const QVariantList &args)
    : Plasma::DataEngine(parent, args),
      m_dirWatch(0)
{

}

KonsoleProfilesEngine::~KonsoleProfilesEngine()
{
}

void KonsoleProfilesEngine::init()
{
    kDebug() << "KonsoleProfilesDataEngine init";

    m_dirWatch = new KDirWatch( this );
    loadProfiles();
    connect(m_dirWatch, SIGNAL(dirty(QString)), this, SLOT(profilesChanged()));
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
    const QStringList lst = KGlobal::dirs()->findDirs( "data", "konsole/" );
    for ( int i = 0; i < lst.count(); i++ )
    {
        m_dirWatch->addDir( lst[i] );
    }

    const QStringList list = KGlobal::dirs()->findAllResources( "data", "konsole/*.profile", KStandardDirs::NoDuplicates );
    const QStringList::ConstIterator end = list.constEnd();
    for (QStringList::ConstIterator it = list.constBegin(); it != end; ++it)
    {
        QFileInfo info( *it );
        const QString profileName = KIO::decodeFileName( info.baseName() );
        QString niceName = profileName;
        KConfig cfg( *it, KConfig::SimpleConfig );

        if ( cfg.hasGroup( "General" ) ) {
            KConfigGroup grp( &cfg, "General" );

            if ( grp.hasKey( "Name" ) ) {
                niceName = grp.readEntry( "Name" );
            }

            QString sourceName = "name:" + profileName;
            kDebug() << "adding sourcename: " << profileName << " ++" << niceName;
            setData(profileName, "prettyName", niceName);
        }
    }
}

K_EXPORT_PLASMA_DATAENGINE(konsoleprofilesengine, KonsoleProfilesEngine)

#include "konsoleprofilesengine.moc"
