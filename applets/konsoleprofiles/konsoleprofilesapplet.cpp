/***************************************************************************
 *   Copyright (C) 2008 by Montel Laurent <montel@kde.org>                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/

#include "konsoleprofilesapplet.h"
#include <QTreeView>
#include <KStandardDirs>
#include <plasma/widgets/icon.h>
#include <QGraphicsProxyWidget>
#include <QGraphicsGridLayout>
#include <QStandardItemModel>
#include <KIcon>
#include <KToolInvocation>
#include <KDirWatch>
#include <QFileInfo>
#include <kio/global.h>
#include <QGraphicsLinearLayout>
#include <KGlobalSettings>

KonsoleProfilesApplet::KonsoleProfilesApplet(QObject *parent, const QVariantList &args)
    : PlasmaAppletDialog(parent, args), m_listView( 0 )
{
    KDirWatch *dirwatch = new KDirWatch( this );
    QStringList lst = KGlobal::dirs()->findDirs( "data", "konsole/" );
    for ( int i = 0; i < lst.count(); i++ )
    {
        dirwatch->addDir( lst[i] );
    }
    connect( dirwatch, SIGNAL(dirty (const QString &) ), this, SLOT( slotUpdateSessionMenu() ) );
}

KonsoleProfilesApplet::~KonsoleProfilesApplet()
{
}

void KonsoleProfilesApplet::initialize()
{
    m_icon = new Plasma::Icon(KIcon("utilities-terminal"), QString(), this);
}

QWidget *KonsoleProfilesApplet::widget()
{
    if ( !m_listView )
    {
        m_listView= new QTreeView(m_dialog);
        m_listView->setEditTriggers( QAbstractItemView::NoEditTriggers );
        m_listView->setRootIsDecorated(false);
        m_listView->setHeaderHidden(true);
        m_listView->setMouseTracking(true);

        m_konsoleModel = new QStandardItemModel(this);
        m_listView->setModel(m_konsoleModel);
        m_listView->setMouseTracking(true);

        initSessionFiles();

        if (KGlobalSettings::singleClick()) {
            connect(m_listView, SIGNAL(clicked(const QModelIndex &)),
                    this, SLOT(slotOnItemClicked(const QModelIndex &)));
        } else {
            connect(m_listView, SIGNAL(doubleClicked(const QModelIndex &)),
                    this, SLOT(slotOnItemClicked(const QModelIndex &)));
        }
    }
    return m_listView;
}


void KonsoleProfilesApplet::slotUpdateSessionMenu()
{
    m_konsoleModel->clear();
    initSessionFiles();
}

void KonsoleProfilesApplet::initSessionFiles()
{
    QStringList list = KGlobal::dirs()->findAllResources( "data", "konsole/*.profile", KStandardDirs::NoDuplicates );
    for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it)
    {
        QFileInfo info( *it );
        QString profileName = KIO::decodeFileName( info.baseName() );
        QString niceName=profileName;
        KConfig cfg( *it, KConfig::SimpleConfig );
        if ( cfg.hasGroup( "General" ) )
        {
            KConfigGroup grp( &cfg, "General" );
            if ( grp.hasKey( "Name" ) )
                niceName = grp.readEntry( "Name" );
            QStandardItem* item = new QStandardItem();
            item->setData(niceName, Qt::DisplayRole);
            item->setData( profileName, ProfilesName );
            m_konsoleModel->appendRow( item);
        }

    }
    m_konsoleModel->sort( 0 );
}

void KonsoleProfilesApplet::slotOnItemClicked(const QModelIndex &index)
{
    if ( m_closePopup )
        m_dialog->hide();
    QStringList args;
    args<<"--profile"<<index.data(ProfilesName).toString();
    KToolInvocation::kdeinitExec("konsole", args);

}



#include "konsoleprofilesapplet.moc"
