/***************************************************************************
 *   Copyright (C) 2007 by Tobias Koenig <tokoe@kde.org>                   *
 *   Copyright (C) 2008 Matthias Fuchs <mat69@gmx.net>                     *
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

#include <QtCore/QAbstractListModel>
#include <QtCore/QTimer>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSortFilterProxyModel>

#include <KLocale>
#include <KIcon>
#include <KNS/Engine>

class ComicModel : public QAbstractTableModel
{
    public:
        ComicModel( const Plasma::DataEngine::Data &comics, QObject *parent = 0 )
            : QAbstractTableModel( parent )
        {
            setComics( comics );
        }

        void setComics( const Plasma::DataEngine::Data &comics )
        {
            mComics = comics;
            reset();
        }

        virtual int rowCount( const QModelIndex &index = QModelIndex() ) const
        {
            if ( !index.isValid() )
                return mComics.count();
            else
                return 0;
        }

        virtual int columnCount( const QModelIndex &index = QModelIndex() ) const
        {
            Q_UNUSED( index )
            return 2;
        }

        virtual QVariant data( const QModelIndex &index, int role = Qt::CheckStateRole ) const
        {
            if ( !index.isValid() || index.row() >= mComics.keys().count() )
                return QVariant();

            if ( index.column() == 0 ) {
                if ( role == Qt::CheckStateRole ) {
                    return mState[ mComics.keys()[ index.row() ] ];
                }
            } else if ( index.column() == 1 ) {
                switch( role ) {
                    case Qt::DisplayRole:
                        return mComics[ mComics.keys()[ index.row() ] ].toStringList()[ 0 ];
                    case Qt::DecorationRole:
                        return KIcon( mComics[ mComics.keys()[ index.row() ] ].toStringList()[ 1 ] );
                    case Qt::UserRole:
                        return mComics.keys()[ index.row() ];
                }
            }

            return QVariant();
        }

        virtual Qt::ItemFlags flags( const QModelIndex &index ) const
        {
            if ( index.isValid() && ( index.column() == 0 ) ) {
                return QAbstractItemModel::flags( index ) | Qt::ItemIsUserCheckable;
            } else {
                return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
            }
        }

        virtual bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole )
        {
            if ( index.isValid() && ( role == Qt::CheckStateRole ) ) {
                mState[ mComics.keys()[ index.row() ] ] = static_cast< Qt::CheckState >( value.toInt() );
                emit dataChanged( index, index );
                return true;
            } else {
                return false;
            }
        }

    private:
        Plasma::DataEngine::Data mComics;
        QHash<QString, Qt::CheckState> mState;
};


ConfigWidget::ConfigWidget( Plasma::DataEngine *engine, QWidget *parent )
    : QWidget( parent ), mEngine( engine )
{
    comicSettings = new QWidget();
    comicUi.setupUi( comicSettings );
    comicUi.pushButton_GHNS->setIcon( KIcon( "get-hot-new-stuff" ) );
    comicUi.pushButton_GHNS_2->setIcon( KIcon( "get-hot-new-stuff" ) );

    appearanceSettings = new QWidget();
    appearanceUi.setupUi( appearanceSettings );

    connect( appearanceUi.pushButton_Size, SIGNAL( clicked() ), this, SIGNAL( maxSizeClicked() ) );
    connect( comicUi.pushButton_GHNS, SIGNAL( clicked() ), this, SLOT( getNewStuff() ) );

    mModel = new ComicModel( mEngine->query( "providers" ), this );
    mProxyModel = new QSortFilterProxyModel( this );
    mProxyModel->setSourceModel( mModel );
    mProxyModel->setSortCaseSensitivity( Qt::CaseInsensitive );
    mProxyModel->sort( 1, Qt::AscendingOrder );

    comicUi.timeEdit_tabs->setMinimumTime( QTime( 0, 0, 10 ) );//minimum to 10 seconds

    comicUi.comboBox_comic->setModel( mProxyModel );
    comicUi.comboBox_comic->setModelColumn( 1 );
    comicUi.listView_comic->setModel( mProxyModel );
    comicUi.listView_comic->resizeColumnToContents( 0 );
}

ConfigWidget::~ConfigWidget()
{
}

void ConfigWidget::getNewStuff()
{
    KNS::Engine engine( this );
    if ( engine.init( "comic.knsrc" ) ) {
        KNS::Entry::List entries = engine.downloadDialogModal( this );
        if ( entries.size() > 0 ) {
            QStringList tmp = comicIdentifier();
            mModel->setComics( mEngine->query( "providers" ) );
            setComicIdentifier( tmp );
        }
    }
}

void ConfigWidget::setComicIdentifier( const QStringList &comics )
{
    if ( comics.isEmpty() && comicUi.comboBox_comic->count() > 0 ) {
        comicUi.comboBox_comic->setCurrentIndex( 0 );
        QModelIndex index( mProxyModel->index( 0, 0 ) );
        comicUi.listView_comic->model()->setData( index, Qt::Checked, Qt::CheckStateRole );
    } else {
        QModelIndex indexCheck;
        QModelIndex indexName;

        //select an item of comboBox_comic
        for ( int i = 0; i < mProxyModel->rowCount(); ++i ) {
            indexName = mProxyModel->index( i, 1 );
            if ( indexName.data( Qt::UserRole ).toString() == comics.at( 0 ) ) {
                comicUi.comboBox_comic->setCurrentIndex( i );
                break;
            }
        }

        //check the items of listView_comic
        for ( int k = 0; k < comics.count(); ++k ) {
            for ( int i = 0; i < mProxyModel->rowCount(); ++i ) {
                indexCheck = mProxyModel->index( i, 0 );
                indexName = mProxyModel->index( i, 1 );
                if ( indexName.data( Qt::UserRole ).toString() == comics.at( k ) ) {
                    comicUi.listView_comic->model()->setData( indexCheck, Qt::Checked, Qt::CheckStateRole );
                    break;
                }
            }
        }
    }
}

QStringList ConfigWidget::comicIdentifier() const
{
    QStringList identifiers;

    if ( mProxyModel->rowCount() ) {
        QModelIndex indexCheck;
        QModelIndex indexName;
        for ( int i = 0; i < mProxyModel->rowCount(); ++i ) {
            indexCheck = mProxyModel->index( i, 0 );
            indexName = mProxyModel->index( i, 1 );
            if ( indexCheck.data( Qt::CheckStateRole ) == Qt::Checked ) {
                identifiers << indexName.data( Qt::UserRole ).toString();
            }
        }

        //if no tabs are used or if nothing has been selected on listView_comic use the comboBox_comic
        if ( !comicUi.checkBox_useTabs->isChecked() || identifiers.isEmpty() ) {
            int indexNum = comicUi.comboBox_comic->currentIndex();

            //use the currentIndex or if none has been set then the first
            if ( indexNum > -1 ) {
                indexName  = mProxyModel->index( indexNum, 1 );
            } else {
                indexName  = mProxyModel->index( 0, 1 );
            }
            identifiers = QStringList( indexName.data( Qt::UserRole ).toString() );
        }
    }

    //if identifiers is still empty add an empty string
    if ( identifiers.isEmpty() ) {
        identifiers << QString();
    }

    return identifiers;
}

QStringList ConfigWidget::comicName() const
{
    QStringList names;

    if ( mProxyModel->rowCount() ) {
        QModelIndex indexCheck;
        QModelIndex indexName;
        for ( int i = 0; i < mProxyModel->rowCount(); ++i ) {
            indexCheck = mProxyModel->index( i, 0 );
            indexName = mProxyModel->index( i, 1 );
            if ( indexCheck.data( Qt::CheckStateRole ) == Qt::Checked ) {
                names << indexName.data( Qt::DisplayRole ).toString();
            }
        }

        //if no tabs are used or if nothing has been selected on listView_comic use the comboBox_comic
        if ( !comicUi.checkBox_useTabs->isChecked() || names.isEmpty() ) {
            int indexNum = comicUi.comboBox_comic->currentIndex();


            //use the currentIndex or if none has been set then the first
            if ( indexNum > -1 ) {
                indexName  = mProxyModel->index( indexNum, 1 );
            } else {
                indexName  = mProxyModel->index( 0, 1 );
            }
            names = QStringList( indexName.data( Qt::DisplayRole ).toString() );
        }
    }

    //if names is still empty add an empty string
    if ( names.isEmpty() ) {
        names << QString();
    }

    return names;
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

#include "configwidget.moc"
