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
#include <QStyleOptionGraphicsItem>
#include <QTreeView>
#include <QVBoxLayout>
#include <KStandardDirs>
#include <KIconLoader>
#include <KInputDialog>
#include <KMessageBox>
#include <KStandardGuiItem>
#include <plasma/widgets/icon.h>
#include <QGraphicsProxyWidget>
#include <QGraphicsGridLayout>
#include <QListWidgetItem>
#include <QGraphicsLinearLayout>
#include <QStandardItemModel>
#include <KIcon>
#include <KToolInvocation>
#include <KDirWatch>
#include <QFileInfo>
#include <kio/global.h>

KonqProfilesApplet::KonqProfilesApplet(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args), m_icon( 0 ), m_layout( 0 ), m_proxy(0), closePopup( false )
{
    int iconSize = IconSize(KIconLoader::Desktop);
    resize(iconSize, iconSize);
    KDirWatch *dirwatch = new KDirWatch( this );
    QStringList lst = KGlobal::dirs()->findDirs( "data", "konqueror/profiles/" );
    for ( int i = 0; i < lst.count(); i++ )
    {
        dirwatch->addDir( lst[i] );
    }
    connect( dirwatch, SIGNAL(dirty (const QString &) ), this, SLOT( slotUpdateKonqProfiles() ) );
}

KonqProfilesApplet::~KonqProfilesApplet()
{
    delete m_widget;
    delete m_icon;
}

void KonqProfilesApplet::init()
{
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    setMaximumSize(INT_MAX, INT_MAX);
    m_layout = new QGraphicsLinearLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    m_layout->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    m_layout->setMaximumSize(INT_MAX, INT_MAX);
    m_layout->setOrientation(Qt::Horizontal);
    setLayout(m_layout);

    m_icon = new Plasma::Icon(KIcon("konqueror"), QString(), this);
    connect(m_icon, SIGNAL(clicked()), this, SLOT(slotOpenMenu()));

    m_widget = new Plasma::Dialog();

    m_proxy = new QGraphicsProxyWidget(this);

    QVBoxLayout *l_layout = new QVBoxLayout();
    l_layout->setSpacing(0);
    l_layout->setMargin(0);

    m_listView= new QTreeView(m_widget);
    m_listView->setEditTriggers( QAbstractItemView::NoEditTriggers );
    m_listView->setRootIsDecorated(false);
    m_listView->setHeaderHidden(true);
    m_listView->setMouseTracking(true);

    m_konqModel = new QStandardItemModel(this);
    m_listView->setModel(m_konqModel);
    m_listView->setMouseTracking(true);

    initSessionFiles();

    connect(m_listView, SIGNAL(clicked (const QModelIndex &)), this, SLOT(slotOnItemClicked(const QModelIndex &)));

    l_layout->addWidget( m_listView );

    m_widget->setLayout( l_layout );
    m_widget->adjustSize();

    constraintsUpdated(Plasma::FormFactorConstraint);
}

void KonqProfilesApplet::constraintsUpdated(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint) {
        // Plasma::Dialog already has standard background
        setBackgroundHints(NoBackground);
        m_layout->removeAt(0);
        switch (formFactor()) {
        case Plasma::Planar:
        case Plasma::MediaCenter:
            closePopup = false;
            setAspectRatioMode(Plasma::IgnoreAspectRatio);
            m_widget->setWindowFlags(Qt::Widget);
            m_proxy->setWidget(m_widget);
            m_proxy->show();
            m_layout->addItem(m_proxy);
            setMinimumSize( 300, 300 );
            break;
        case Plasma::Horizontal:
        case Plasma::Vertical:
            closePopup = true;
            setAspectRatioMode(Plasma::Square);
            m_widget->setWindowFlags(Qt::Popup);
            m_proxy->setWidget(0);
            m_proxy->hide();
            m_layout->addItem(m_icon);
            break;
        }
    }
}

void KonqProfilesApplet::slotUpdateKonqProfiles()
{
    m_konqModel->clear();
    initSessionFiles();
}

void KonqProfilesApplet::initSessionFiles()
{
    QStringList list = KGlobal::dirs()->findAllResources( "data", "konqueror/profiles/*", KStandardDirs::NoDuplicates );
    for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it)
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
}

void KonqProfilesApplet::slotOpenMenu()
{
    if (m_widget->isVisible()) {
        m_widget->hide();
    } else {
        m_widget->move(popupPosition(m_widget->sizeHint()));
        m_widget->show();
    }

    m_widget->clearFocus();
}

void KonqProfilesApplet::slotOnItemClicked(const QModelIndex &index)
{
    if ( closePopup )
        m_widget->hide();
    QStringList args;
    args<<"--profile"<<index.data(ProfilesName).toString();
    KToolInvocation::kdeinitExec("konqueror", args);

}



#include "konqprofilesapplet.moc"
