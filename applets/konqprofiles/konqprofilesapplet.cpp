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

#include "konqprofilesapplet.h"
#include <QTreeView>
#include <KStandardDirs>
#include <plasma/widgets/iconwidget.h>
#include <QGraphicsProxyWidget>
#include <QGraphicsLinearLayout>
#include <QStandardItemModel>
#include <KIcon>
#include <KToolInvocation>
#include <KDirWatch>
#include <KGlobalSettings>
#include <QFileInfo>
#include <kio/global.h>

KonqProfilesApplet::KonqProfilesApplet(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args), m_listView( 0 )
{
    KDirWatch *dirwatch = new KDirWatch( this );
    QStringList lst = KGlobal::dirs()->findDirs( "data", "konqueror/profiles/" );
    for ( int i = 0; i < lst.count(); i++ )
    {
        dirwatch->addDir( lst[i] );
    }
    connect( dirwatch, SIGNAL(dirty (const QString &) ), this, SLOT( slotUpdateKonqProfiles() ) );
    setPopupIcon("konqueror");
}

KonqProfilesApplet::~KonqProfilesApplet()
{
    delete m_listView;
}

QWidget *KonqProfilesApplet::widget()
{
    if ( !m_listView )
    {
        m_listView= new QTreeView();
        m_listView->setAttribute(Qt::WA_NoSystemBackground);
        m_listView->setEditTriggers( QAbstractItemView::NoEditTriggers );
        m_listView->setRootIsDecorated(false);
        m_listView->setHeaderHidden(true);
        m_listView->setMouseTracking(true);

        m_konqModel = new QStandardItemModel(this);
        m_listView->setModel(m_konqModel);
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


void KonqProfilesApplet::slotUpdateKonqProfiles()
{
    m_konqModel->clear();
    initSessionFiles();
}

void KonqProfilesApplet::initSessionFiles()
{
    const QStringList list = KGlobal::dirs()->findAllResources( "data", "konqueror/profiles/*", KStandardDirs::NoDuplicates );
    for (QStringList::ConstIterator it = list.constBegin(); it != list.constEnd(); ++it)
    {
        QFileInfo info( *it );
        QString profileName = KIO::decodeFileName( info.baseName() );
        QString niceName=profileName;
        KConfig cfg( *it, KConfig::SimpleConfig );
        if ( cfg.hasGroup( "Profile" ) )
        {
            KConfigGroup grp( &cfg, "Profile" );
            if ( grp.hasKey( "Name" ) )
                niceName = grp.readEntry( "Name" );
            QStandardItem* item = new QStandardItem();
            item->setData(niceName, Qt::DisplayRole);
            item->setData( profileName, ProfilesName );
            m_konqModel->appendRow( item);
        }

    }
    m_konqModel->sort( 0 );
}

void KonqProfilesApplet::slotOnItemClicked(const QModelIndex &index)
{
    hidePopup();
    QStringList args;
    args<<"--profile"<<index.data(ProfilesName).toString();
    KToolInvocation::kdeinitExec("konqueror", args);
}

#include "konqprofilesapplet.moc"

