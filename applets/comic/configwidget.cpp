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

ComicUpdater::ComicUpdater( QObject *parent )
  : QObject( parent ),
    mDownloadManager( 0 ),
    mUpdatesActivated( false ),
    mUpdateIntervall( 7 ),
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
    mUpdatesActivated = mGroup.readEntry( "autoUpdates", false );
    if ( mUpdatesActivated ) {
        mUpdateIntervall = mGroup.readEntry( "updateIntervall", 7 );
        mLastUpdate = mGroup.readEntry( "lastUpdate", QDateTime() );
        checkForUpdate();
    }
}

void ComicUpdater::save()
{
    mGroup.writeEntry( "autoUpdates", mUpdatesActivated );
    mGroup.writeEntry( "updateIntervall", mUpdateIntervall );
}

void ComicUpdater::applyConfig( ConfigWidget *widget )
{
    mUpdatesActivated = widget->autoUpdates();
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


ConfigWidget::ConfigWidget( Plasma::DataEngine *engine, ComicModel *model, const QStringList &usedComics, QSortFilterProxyModel *proxy, KConfigDialog *parent )
    : QWidget( parent ), mEngine( engine ), mModel( model ), mProxyModel( proxy ), mNewStuffDialog( 0 )
{
    comicSettings = new QWidget( this );
    comicUi.setupUi( comicSettings );
    comicUi.pushButton_GHNS->setIcon( KIcon( "get-hot-new-stuff" ) );
    comicUi.pushButton_GHNS_2->setIcon( KIcon( "get-hot-new-stuff" ) );

    appearanceSettings = new QWidget();
    appearanceUi.setupUi( appearanceSettings );

    advancedSettings = new QWidget();
    advancedUi.setupUi( advancedSettings );

    connect( appearanceUi.pushButton_Size, SIGNAL( clicked() ), this, SIGNAL( maxSizeClicked() ) );
    connect( comicUi.pushButton_GHNS, SIGNAL( clicked() ), this, SLOT( getNewStuff() ) );

    comicUi.timeEdit_tabs->setMinimumTime( QTime( 0, 0, 10 ) );//minimum to 10 seconds

    //initialize the comboBox
    comicUi.comboBox_comic->setModel( mProxyModel );
    comicUi.comboBox_comic->setModelColumn( 1 );
    if ( mProxyModel->rowCount() && usedComics.count() ) {
        //set the correct initial item of the comboBox if it is defined
        const int index = comicUi.comboBox_comic->findData( usedComics.first() );
        comicUi.comboBox_comic->setCurrentIndex( index );
    }

    comicUi.listView_comic->setModel( mProxyModel );
    comicUi.listView_comic->resizeColumnToContents( 0 );

    connect(comicUi.updateIntervall, SIGNAL(valueChanged(int)), this, SLOT(slotUpdateIntervallChanged(int)));
    connect(comicUi.updateIntervall_2, SIGNAL(valueChanged(int)), this, SLOT(slotUpdateIntervallChanged(int)));
    connect( comicUi.checkBox_useTabs, SIGNAL( clicked( bool ) ), this, SLOT( slotListChosen() ) );
    connect( comicUi.checkBox_useTabs_2, SIGNAL( clicked( bool ) ), this, SLOT( slotComboBoxChosen() ) );
    connect(parent, SIGNAL(applyClicked()), this, SLOT(slotSave()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(slotSave()));
    
    // "Apply" button connections
    connect(comicUi.comboBox_comic , SIGNAL(currentIndexChanged(int)), this , SIGNAL(enableApply()));
    connect(comicUi.pushButton_GHNS , SIGNAL(clicked(bool)), this , SIGNAL(enableApply()));
    connect(comicUi.checkBox_middle , SIGNAL(toggled(bool)), this , SIGNAL(enableApply()));
    connect(comicUi.checkBox_useTabs , SIGNAL(toggled(bool)), this , SIGNAL(enableApply()));
    connect(appearanceUi.checkBox_arrows, SIGNAL(toggled(bool)), this , SIGNAL(enableApply()));
    connect(appearanceUi.checkBox_title, SIGNAL(toggled(bool)), this , SIGNAL(enableApply()));
    connect(appearanceUi.checkBox_identifier, SIGNAL(toggled(bool)), this , SIGNAL(enableApply()));
    connect(appearanceUi.checkBox_author, SIGNAL(toggled(bool)), this , SIGNAL(enableApply()));
    connect(appearanceUi.checkBox_url, SIGNAL(toggled(bool)), this , SIGNAL(enableApply()));
    connect(appearanceUi.kbuttongroup, SIGNAL(changed(int)), this , SIGNAL(enableApply()));
    connect(advancedUi.useMaxComicLimit, SIGNAL(toggled(bool)), this , SIGNAL(enableApply()));
    connect(advancedUi.maxComicLimit, SIGNAL(valueChanged(int)), this, SIGNAL(enableApply()));
    connect(advancedUi.errorPicture, SIGNAL(toggled(bool)), this , SIGNAL(enableApply()));
}

ConfigWidget::~ConfigWidget()
{
}

void ConfigWidget::checkCurrentIndex()
{
    //set all items of mProxyModel to unchecked
    for ( int i = 0; i < mProxyModel->rowCount(); ++i ) {
        QModelIndex index = mProxyModel->index( i, 0 );
        mProxyModel->setData( index, Qt::Unchecked, Qt::CheckStateRole );
    }

    //check the selected index
    QModelIndex index = mProxyModel->index( comicUi.comboBox_comic->currentIndex(), 0 );
    mProxyModel->setData( index, Qt::Checked, Qt::CheckStateRole );
}


void ConfigWidget::slotSave()
{
    //useTabs() is already handled in the proxy itself
    if ( !useTabs() ) {
        checkCurrentIndex();
    }
}


void ConfigWidget::slotComboBoxChosen()
{
    comicUi.comboBox_comic->setCurrentIndex( 0 );
}

void ConfigWidget::slotListChosen()
{
    checkCurrentIndex();
}

void ConfigWidget::slotUpdateIntervallChanged( int newIntervall )
{
    if ( comicUi.updateIntervall->value() != newIntervall ) {
        comicUi.updateIntervall->setValue( newIntervall );
    }
    if ( comicUi.updateIntervall_2->value() != newIntervall ) {
        comicUi.updateIntervall_2->setValue( newIntervall );
    }
}

void ConfigWidget::getNewStuff()
{
    if (!mNewStuffDialog) {
        mNewStuffDialog = new KNS3::DownloadDialog( "comic.knsrc", this );
        connect(mNewStuffDialog, SIGNAL(accepted()), SLOT(newStuffFinished()));
    }
    mNewStuffDialog->show();
}

void ConfigWidget::newStuffFinished()
{
    if ( mNewStuffDialog->changedEntries().count() ) {
        mModel->setComics( mEngine->query( "providers" ), mModel->selected() );

        comicUi.listView_comic->resizeColumnToContents( 0 );

        if ( !useTabs() && mProxyModel->rowCount() ) {
            comicUi.comboBox_comic->setCurrentIndex( 0 );
        }
    }
}

void ConfigWidget::setShowComicUrl( bool show )
{
    appearanceUi.checkBox_url->setChecked( show );
}

bool ConfigWidget::showComicUrl() const
{
    return appearanceUi.checkBox_url->isChecked();
}

void ConfigWidget::setShowComicAuthor( bool show )
{
    appearanceUi.checkBox_author->setChecked( show );
}

bool ConfigWidget::showComicAuthor() const
{
    return appearanceUi.checkBox_author->isChecked();
}

void ConfigWidget::setShowComicTitle( bool show )
{
    appearanceUi.checkBox_title->setChecked( show );
}

bool ConfigWidget::showComicTitle() const
{
    return appearanceUi.checkBox_title->isChecked();
}

void ConfigWidget::setShowComicIdentifier( bool show )
{
    appearanceUi.checkBox_identifier->setChecked( show );
}

bool ConfigWidget::showComicIdentifier() const
{
    return appearanceUi.checkBox_identifier->isChecked();
}

void ConfigWidget::setShowErrorPicture( bool show )
{
    advancedUi.errorPicture->setChecked( show );
}

bool ConfigWidget::showErrorPicture() const
{
    return advancedUi.errorPicture->isChecked();
}


void ConfigWidget::setArrowsOnHover( bool arrows )
{
    return appearanceUi.checkBox_arrows->setChecked( arrows );
}

bool ConfigWidget::arrowsOnHover() const
{
    return appearanceUi.checkBox_arrows->isChecked();
}

void ConfigWidget::setMiddleClick( bool checked )
{
    comicUi.checkBox_middle->setChecked( checked );
    comicUi.checkBox_middle_2->setChecked( checked );
}

bool ConfigWidget::middleClick() const
{
    return comicUi.checkBox_middle->isChecked();
}

void ConfigWidget::setTabSwitchTime( const QTime &time )
{
    comicUi.timeEdit_tabs->setTime( time );
}

QTime ConfigWidget::tabSwitchTime() const
{
    return comicUi.timeEdit_tabs->time();
}

void ConfigWidget::setUseTabs( bool use )
{
    comicUi.checkBox_useTabs->setChecked( use );
    comicUi.checkBox_useTabs_2->setChecked( use );
}

bool ConfigWidget::useTabs() const
{
    return comicUi.checkBox_useTabs->isChecked();
}

void ConfigWidget::setSwitchTabs( bool switchTabs )
{
    comicUi.checkBox_switchTabs->setChecked( switchTabs );
}

bool ConfigWidget::switchTabs() const
{
    return comicUi.checkBox_switchTabs->isChecked();
}

void ConfigWidget::setTabView(int tabView)
{
    appearanceUi.kbuttongroup->setSelected( tabView );
}

int ConfigWidget::tabView() const
{
    return appearanceUi.kbuttongroup->selected();
}

bool ConfigWidget::useMaxComicLimit() const
{
    return advancedUi.useMaxComicLimit->isChecked();
}

void ConfigWidget::setUseMaxComicLimit( bool use )
{
    advancedUi.useMaxComicLimit->setChecked( use );
}

int ConfigWidget::maxComicLimit() const
{
    return advancedUi.maxComicLimit->value();
}

void ConfigWidget::setMaxComicLimit( int limit )
{
    advancedUi.maxComicLimit->setValue( limit );
}

void ConfigWidget::setAutoUpdates( bool activated )
{
    comicUi.autoUpdates->setChecked( activated );
    comicUi.autoUpdates_2->setChecked( activated );
}

bool ConfigWidget::autoUpdates() const
{
    return comicUi.autoUpdates->isChecked();
}

void ConfigWidget::setUpdateIntervall( int days )
{
    comicUi.updateIntervall->setValue( days );
}

int ConfigWidget::updateIntervall() const
{
    return comicUi.updateIntervall->value();
}

#include "configwidget.moc"
