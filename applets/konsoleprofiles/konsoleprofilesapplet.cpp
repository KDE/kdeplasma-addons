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
#include <QStandardItemModel>
#include <KIcon>
#include <KToolInvocation>
#include <KDirWatch>
#include <QFileInfo>
#include <kio/global.h>
#include <QGraphicsLinearLayout>

KonsoleProfilesApplet::KonsoleProfilesApplet(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args), m_icon( 0 ), m_proxy(0), m_layout( 0 )
{
    int iconSize = IconSize(KIconLoader::Desktop);
    resize(iconSize, iconSize);
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
    delete m_widget;
    delete m_icon;
}

void KonsoleProfilesApplet::init()
{
    m_widget = new Plasma::Dialog();
    QVBoxLayout *l_layout = new QVBoxLayout();
    l_layout->setSpacing(0);
    l_layout->setMargin(0);

    m_listView= new QTreeView(m_widget);
    m_listView->setEditTriggers( QAbstractItemView::NoEditTriggers );
    m_listView->setRootIsDecorated(false);
    m_listView->setHeaderHidden(true);
    m_listView->setMouseTracking(true);

    m_konsoleModel = new QStandardItemModel(this);
    m_listView->setModel(m_konsoleModel);
    m_listView->setMouseTracking(true);

    initSessionFiles();

    connect(m_listView, SIGNAL(clicked (const QModelIndex &)), this, SLOT(slotOnItemClicked(const QModelIndex &)));

    l_layout->addWidget( m_listView );

    m_widget->setLayout( l_layout );
    m_widget->adjustSize();

}

void KonsoleProfilesApplet::constraintsEvent(Plasma::Constraints constraints)
{
    // on the panel we don't want a background, and our proxy widget in Planar has one
    setBackgroundHints(NoBackground);

    bool isSizeConstrained = formFactor() != Plasma::Planar && formFactor() != Plasma::MediaCenter;

    if (constraints & Plasma::FormFactorConstraint) {
        if (isSizeConstrained) {
            delete m_layout;
            m_layout = 0;

            if (m_proxy) {
                m_proxy->setWidget(0);
                delete m_proxy;
                m_proxy = 0;
            }

            initSysTray();
        } else {
            delete m_icon;
            m_icon = 0;

            m_widget->setWindowFlags(Qt::Widget);
            m_layout = new QGraphicsLinearLayout();
            m_layout->setContentsMargins(0,0,0,0);
            m_layout->setSpacing(0);
            m_proxy = new QGraphicsProxyWidget(this);
            m_proxy->setWidget(m_widget);
            m_proxy->show();
            setLayout( m_layout );
            //Laurent size fixed until I was able to resize it correctly
            //setMinimumContentSize(m_widget->size());
            //setMaximumContentSize(m_widget->size());

        }
    }

    if (m_icon && constraints & Plasma::SizeConstraint) {
        m_icon->resize(geometry().size());
    }
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
}

void KonsoleProfilesApplet::initSysTray()
{
    if (m_icon) {
        return;
    }

    m_widget->setWindowFlags(Qt::Popup);

    m_icon = new Plasma::Icon(KIcon("utilities-terminal"), QString(), this);
    connect(m_icon, SIGNAL(clicked()), this, SLOT(slotOpenMenu()));
    m_icon->resize(m_icon->sizeFromIconSize(IconSize(KIconLoader::Small)));
    updateGeometry();
}

void KonsoleProfilesApplet::slotOpenMenu()
{
    if (m_widget->isVisible()) {
        m_widget->hide();
    } else {
        m_widget->move(popupPosition(m_widget->sizeHint()));
        m_widget->show();
    }

    m_widget->clearFocus();
}

void KonsoleProfilesApplet::slotOnItemClicked(const QModelIndex &index)
{
    if ( m_icon )
        m_widget->hide();
    QStringList args;
    args<<"--profile"<<index.data(ProfilesName).toString();
    KToolInvocation::kdeinitExec("konsole", args);

}



#include "konsoleprofilesapplet.moc"
