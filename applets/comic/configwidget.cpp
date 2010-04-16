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

#include <KNS3/DownloadDialog>

ConfigWidget::ConfigWidget( Plasma::DataEngine *engine, ComicModel *model, const QStringList &usedComics, QSortFilterProxyModel *proxy, QWidget *parent )
    : QWidget( parent ), mEngine( engine ), mModel( model ), mUsedComics( usedComics ), mProxyModel( proxy ), mNewStuffDialog( 0 )
{
    comicSettings = new QWidget( this );
    comicUi.setupUi( comicSettings );
    comicUi.pushButton_GHNS->setIcon( KIcon( "get-hot-new-stuff" ) );
    comicUi.pushButton_GHNS_2->setIcon( KIcon( "get-hot-new-stuff" ) );

    appearanceSettings = new QWidget();
    appearanceUi.setupUi( appearanceSettings );

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
        mCurrentIndex = mProxyModel->index( index, 0 );
    }

    comicUi.listView_comic->setModel( mProxyModel );
    comicUi.listView_comic->resizeColumnToContents( 0 );

    connect( comicUi.comboBox_comic, SIGNAL( currentIndexChanged( int ) ), this, SLOT( slotCurrentIndexChanged( int ) ) );
    connect( comicUi.checkBox_useTabs_2, SIGNAL( clicked( bool ) ), this, SLOT( slotComboBoxChosen() ) );
}

ConfigWidget::~ConfigWidget()
{
}

void ConfigWidget::slotComboBoxChosen()
{
    comicUi.comboBox_comic->setCurrentIndex( 0 );
}

void ConfigWidget::slotCurrentIndexChanged( int newIndex )
{
    if ( useTabs() || !mProxyModel->rowCount() ) {
        return;
    }

    //decide wether to modify newIndex or not
    const bool oldIsChecked = mCurrentIndex.isValid() && mCurrentIndex.data( Qt::CheckStateRole ) == Qt::Checked;
    if ( !oldIsChecked ) {
        //is not checked, i.e. comics have been added/removed, choose the first entry instead
        newIndex = 0;
    }

    //set all items of mProxyModel to unchecked
    for ( int i = 0; i < mProxyModel->rowCount(); ++i ) {
        QModelIndex index = mProxyModel->index( i, 0 );
        mProxyModel->setData( index, Qt::Unchecked, Qt::CheckStateRole );
    }

    //check the selected index
    mCurrentIndex = mProxyModel->index( newIndex, 0 );
    mProxyModel->setData( mCurrentIndex, Qt::Checked, Qt::CheckStateRole );//FIXME things might be selected that are not checked!!!

    if ( !oldIsChecked ) {
        comicUi.comboBox_comic->setCurrentIndex( 0 );
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
        mModel->setComics( mEngine->query( "providers" ), mModel->selected() );//HACK if nothing is checked/selected, then do check/select something automatically??

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

void ConfigWidget::setHideTabBar( bool hide )
{
   comicUi.checkBox_hideTabBar->setChecked( hide );
}

bool ConfigWidget::hideTabBar() const
{
    return comicUi.checkBox_hideTabBar->isChecked();
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

#include "configwidget.moc"
