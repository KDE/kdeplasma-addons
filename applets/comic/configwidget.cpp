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
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>

#include <KLocale>
#include <KServiceTypeTrader>

#include "pluginmanager.h"


class ComicModel : public QAbstractListModel
{
    public:
        ComicModel( QObject *parent = 0 )
            : QAbstractListModel( parent )
        {
            mComics = PluginManager::Instance()->comics();
        }

        virtual int rowCount( const QModelIndex &index = QModelIndex() ) const
        {
            if ( !index.isValid() )
                return mComics.count();
            else
                return 0;
        }

        virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const
        {
            if ( !index.isValid() || index.row() >= mComics.count() )
                return QVariant();

            if ( role == Qt::DisplayRole )
                return mComics[ index.row() ].title;
            else if ( role == Qt::DecorationRole )
                return mComics[ index.row() ].icon;
            else if ( role == Qt::UserRole )
                return mComics[ index.row() ].identifier;
            else
                return QVariant();
        }

    private:

        QList<ComicEntry> mComics;
};


ConfigWidget::ConfigWidget( QWidget *parent )
    : QWidget( parent )
{
    QGridLayout *layout = new QGridLayout( this );
    layout->setMargin( 0 );
    mComicIdentifier = new QComboBox( this );

    QLabel *label = new QLabel( i18n( "Comic:" ), this );
    label->setBuddy( mComicIdentifier );

    mShowComicAuthor = new QCheckBox( i18n( "Show Comic Author" ), this );
    mShowComicTitle = new QCheckBox( i18n( "Show Comic Title" ), this );
    mShowComicUrl = new QCheckBox( i18n( "Show Comic Url" ), this );
    mShowComicIdentifier = new QCheckBox( i18n( "Show Comic Identifier" ), this );

    layout->addWidget( label, 0, 0 );
    layout->addWidget( mComicIdentifier, 0, 1 );
    layout->addWidget( mShowComicAuthor, 1, 0, 1, 2 );
    layout->addWidget( mShowComicTitle, 2, 0, 1, 2 );
    layout->addWidget( mShowComicUrl, 3, 0, 1, 2 );
    layout->addWidget( mShowComicIdentifier, 4, 0, 1, 2 );

    mModel = new ComicModel( this );
    mComicIdentifier->setModel( mModel );
}

ConfigWidget::~ConfigWidget()
{
}

void ConfigWidget::setComicIdentifier( const QString &comic )
{
    for ( int i = 0; i < mModel->rowCount(); ++i ) {
        const QModelIndex index = mModel->index( i, 0 );
        if ( index.data( Qt::UserRole ).toString() == comic ) {
            mComicIdentifier->setCurrentIndex( i );
            break;
        }
    }
}

QString ConfigWidget::comicIdentifier() const
{
    const QModelIndex index = mModel->index( mComicIdentifier->currentIndex(), 0 );
    return index.data( Qt::UserRole ).toString();
}

void ConfigWidget::setShowComicUrl( bool show )
{
    mShowComicUrl->setChecked( show );
}

bool ConfigWidget::showComicUrl() const
{
    return mShowComicUrl->isChecked();
}

void ConfigWidget::setShowComicAuthor( bool show )
{
    mShowComicAuthor->setChecked( show );
}

bool ConfigWidget::showComicAuthor() const
{
    return mShowComicAuthor->isChecked();
}

void ConfigWidget::setShowComicTitle( bool show )
{
    mShowComicTitle->setChecked( show );
}

bool ConfigWidget::showComicTitle() const
{
    return mShowComicTitle->isChecked();
}

void ConfigWidget::setShowComicIdentifier( bool show )
{
    mShowComicIdentifier->setChecked( show );
}

bool ConfigWidget::showComicIdentifier() const
{
    return mShowComicIdentifier->isChecked();
}
