/***************************************************************************
 *   Copyright (C) 2007 by Tobias Koenig <tokoe@kde.org>                   *
 *   Copyright (C) 2008-2010 Matthias Fuchs <mat69@gmx.net>                *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "configwidget.h"
#include "comicmodel.h"

#include <QtCore/QTimer>
#include <QtGui/QSortFilterProxyModel>

#include <KConfigDialog>
#include <KNS3/DownloadDialog>
#include <knewstuff3/downloadmanager.h>
#include "comicconfig.h"

ComicUpdater::ComicUpdater( QObject *parent )
  : QObject( parent ),
    mDownloadManager( 0 ),
    mUpdateIntervall( 3 ),
    m_updateTimer( 0 )
{
}

ComicUpdater::~ComicUpdater()
{
}

void ComicUpdater::init(const KConfigGroup &group)
{
    mGroup = group;
}

void ComicUpdater::load()
{
    //check when the last update happened and update if necessary
    mUpdateIntervall = mGroup.readEntry( "updateIntervall", 3 );
    if ( mUpdateIntervall ) {
        mLastUpdate = mGroup.readEntry( "lastUpdate", QDateTime() );
        checkForUpdate();
    }
}

void ComicUpdater::save()
{
    mGroup.writeEntry( "updateIntervall", mUpdateIntervall );
}

void ComicUpdater::applyConfig( ConfigWidget *widget )
{
    mUpdateIntervall = widget->updateIntervall();
}

void ComicUpdater::checkForUpdate()
{
    //start a timer to check each hour, if KNS3 should look for updates
    if ( !m_updateTimer ) {
        m_updateTimer = new QTimer(this);
        connect(m_updateTimer, SIGNAL(timeout()), this, SLOT(checkForUpdate()));
        m_updateTimer->start( 1 * 60 * 60 * 1000 );
    }

    if ( !mLastUpdate.isValid() || ( mLastUpdate.addDays( mUpdateIntervall ) < QDateTime::currentDateTime() ) ) {
        mGroup.writeEntry( "lastUpdate", QDateTime::currentDateTime() );
        downloadManager()->checkForUpdates();
    }
}

void ComicUpdater::slotUpdatesFound( const KNS3::Entry::List &entries )
{
    for ( int i = 0; i < entries.count(); ++i ) {
        downloadManager()->installEntry( entries[ i ] );
    }
}

KNS3::DownloadManager *ComicUpdater::downloadManager()
{
    if ( !mDownloadManager ) {
        mDownloadManager = new KNS3::DownloadManager( "comic.knsrc", this );
        connect(mDownloadManager, SIGNAL(searchResult(KNS3::Entry::List)), this, SLOT(slotUpdatesFound(KNS3::Entry::List)));
    }

    return mDownloadManager;
}


ConfigWidget::ConfigWidget( Plasma::DataEngine *engine, ComicModel *model, QSortFilterProxyModel *proxy, KConfigDialog *parent )
    : QWidget( parent ), mEngine( engine ), mModel( model ), mProxyModel( proxy ), mNewStuffDialog( 0 )
{
    comicSettings = new QWidget( this );
    comicUi.setupUi( comicSettings );
    comicUi.pushButton_GHNS->setIcon( KIcon( "get-hot-new-stuff" ) );

    appearanceSettings = new QWidget();
    appearanceUi.setupUi( appearanceSettings );

    advancedSettings = new QWidget();
    advancedUi.setupUi( advancedSettings );

    connect( appearanceUi.pushButton_Size, SIGNAL(clicked()), this, SIGNAL(maxSizeClicked()) );
    connect( comicUi.pushButton_GHNS, SIGNAL(clicked()), this, SLOT(getNewStuff()) );

    comicUi.listView_comic->setModel( mProxyModel );
    comicUi.listView_comic->resizeColumnToContents( 0 );

    // "Apply" button connections
    connect(comicUi.pushButton_GHNS , SIGNAL(clicked(bool)), this , SIGNAL(enableApply()));
    connect(comicUi.kcfg_MiddleClick , SIGNAL(toggled(bool)), this , SIGNAL(enableApply()));
    connect(comicUi.kcfg_UpdateIntervall, SIGNAL(valueChanged(int)), this, SIGNAL(enableApply()));
    connect(comicUi.kcfg_UpdateIntervallComicStrips, SIGNAL(valueChanged(int)), this, SIGNAL(enableApply()));
    connect(appearanceUi.kcfg_ArrowsOnHover, SIGNAL(toggled(bool)), this , SIGNAL(enableApply()));
    connect(appearanceUi.kcfg_ShowComicTitle, SIGNAL(toggled(bool)), this , SIGNAL(enableApply()));
    connect(appearanceUi.kcfg_ShowComicIdentifier, SIGNAL(toggled(bool)), this , SIGNAL(enableApply()));
    connect(appearanceUi.kcfg_ShowComicAuthor, SIGNAL(toggled(bool)), this , SIGNAL(enableApply()));
    connect(appearanceUi.kcfg_ShowComicUrl, SIGNAL(toggled(bool)), this , SIGNAL(enableApply()));
    connect(appearanceUi.kcfg_TabView, SIGNAL(changed(int)), this , SIGNAL(enableApply()));
    connect(advancedUi.kcfg_MaxComicLimit, SIGNAL(valueChanged(int)), this, SIGNAL(enableApply()));
    connect(advancedUi.kcfg_ShowErrorPicture, SIGNAL(toggled(bool)), this , SIGNAL(enableApply()));

    mEngine->connectSource( QLatin1String( "providers" ), this );
}

ConfigWidget::~ConfigWidget()
{
    mEngine->disconnectSource( QLatin1String( "providers" ), this );
}

void ConfigWidget::getNewStuff()
{
    if (!mNewStuffDialog) {
        mNewStuffDialog = new KNS3::DownloadDialog( "comic.knsrc", this );
    }
    mNewStuffDialog->show();
}

void ConfigWidget::dataUpdated(const QString &name, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(name);
    mModel->setComics( data, mModel->selected() );
    comicUi.listView_comic->resizeColumnToContents( 0 );
}

void ConfigWidget::setShowComicUrl( bool show )
{
    appearanceUi.kcfg_ShowComicUrl->setChecked( show );
}

bool ConfigWidget::showComicUrl() const
{
    return appearanceUi.kcfg_ShowComicUrl->isChecked();
}

void ConfigWidget::setShowComicAuthor( bool show )
{
    appearanceUi.kcfg_ShowComicAuthor->setChecked( show );
}

bool ConfigWidget::showComicAuthor() const
{
    return appearanceUi.kcfg_ShowComicAuthor->isChecked();
}

void ConfigWidget::setShowComicTitle( bool show )
{
    appearanceUi.kcfg_ShowComicTitle->setChecked( show );
}

bool ConfigWidget::showComicTitle() const
{
    return appearanceUi.kcfg_ShowComicTitle->isChecked();
}

void ConfigWidget::setShowComicIdentifier( bool show )
{
    appearanceUi.kcfg_ShowComicIdentifier->setChecked( show );
}

bool ConfigWidget::showComicIdentifier() const
{
    return appearanceUi.kcfg_ShowComicIdentifier->isChecked();
}

void ConfigWidget::setShowErrorPicture( bool show )
{
    advancedUi.kcfg_ShowErrorPicture->setChecked( show );
}

bool ConfigWidget::showErrorPicture() const
{
    return advancedUi.kcfg_ShowErrorPicture->isChecked();
}


void ConfigWidget::setArrowsOnHover( bool arrows )
{
    return appearanceUi.kcfg_ArrowsOnHover->setChecked( arrows );
}

bool ConfigWidget::arrowsOnHover() const
{
    return appearanceUi.kcfg_ArrowsOnHover->isChecked();
}

void ConfigWidget::setMiddleClick( bool checked )
{
    comicUi.kcfg_MiddleClick->setChecked( checked );
}

bool ConfigWidget::middleClick() const
{
    return comicUi.kcfg_MiddleClick->isChecked();
}

void ConfigWidget::setTabView(int tabView)
{
    appearanceUi.kcfg_TabView->setSelected( tabView );
}

int ConfigWidget::tabView() const
{
    return appearanceUi.kcfg_TabView->selected();
}

int ConfigWidget::maxComicLimit() const
{
    return advancedUi.kcfg_MaxComicLimit->value();
}

void ConfigWidget::setMaxComicLimit( int limit )
{
    advancedUi.kcfg_MaxComicLimit->setValue( limit );
}

void ConfigWidget::setUpdateIntervall( int days )
{
    comicUi.kcfg_UpdateIntervall->setValue( days );
}

int ConfigWidget::updateIntervall() const
{
    return comicUi.kcfg_UpdateIntervall->value();
}

void ConfigWidget::setCheckNewComicStripsIntervall( int minutes )
{
    comicUi.kcfg_UpdateIntervallComicStrips->setValue( minutes );
}

int ConfigWidget::checkNewComicStripsIntervall() const
{
    return comicUi.kcfg_UpdateIntervallComicStrips->value();
}

#include "configwidget.moc"
