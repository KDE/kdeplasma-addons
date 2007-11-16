/***************************************************************************
 *   Copyright (C) 2007 by Tobias Koenig <tokoe@kde.org>                   *
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

#include <QtCore/QAbstractListModel>
#include <QtGui/QComboBox>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>

#include <klocale.h>

#include "configdialog.h"

class ComicModel : public QAbstractListModel
{
    public:
        ComicModel( QObject *parent = 0 )
            : QAbstractListModel( parent )
        {
          mComics << ComicEntry( "userfriendly", i18n( "Userfriendly" ), QPixmap( ":userfriendly" ) );
          mComics << ComicEntry( "dilbert", i18n( "Dilbert" ), QPixmap( ":dilbert" ) );
          mComics << ComicEntry( "garfield", i18n( "Garfield" ), QPixmap( ":garfield" ) );
          mComics << ComicEntry( "snoopy", i18n( "Snoopy" ), QPixmap( ":snoopy" ) );
          mComics << ComicEntry( "xkcd", i18n( "XKCD" ), QPixmap( ":xkcd" ) );
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
        class ComicEntry
        {
            public:
                ComicEntry( const QString &_identifier, const QString &_title, const QPixmap &_icon )
                    : identifier( _identifier ), title( _title ), icon( _icon )
                {
                }

                QString identifier;
                QString title;
                QPixmap icon;
        };

        QList<ComicEntry> mComics;
};


ConfigDialog::ConfigDialog( QWidget *parent )
    : KDialog( parent )
{
    setCaption( i18n( "Comic Configuration" ) );

    setButtons( Ok | Apply | Cancel );
    setDefaultButton( Ok );
    showButtonSeparator( true );

    QGridLayout *layout = new QGridLayout( mainWidget() );
    mComicIdentifier = new QComboBox( mainWidget() );

    QLabel *label = new QLabel( i18n( "Comic:" ), mainWidget() );
    label->setBuddy( mComicIdentifier );
    layout->addWidget( label, 0, 0 );
    layout->addWidget( mComicIdentifier, 0, 1 );

    mModel = new ComicModel( this );
    mComicIdentifier->setModel( mModel );
}

ConfigDialog::~ConfigDialog()
{
}

void ConfigDialog::setComicIdentifier( const QString &comic )
{
    for ( int i = 0; i < mModel->rowCount(); ++i ) {
        const QModelIndex index = mModel->index( i, 0 );
        if ( index.data( Qt::UserRole ).toString() == comic ) {
            mComicIdentifier->setCurrentIndex( i );
            break;
        }
    }
}

QString ConfigDialog::comicIdentifier() const
{
    const QModelIndex index = mModel->index( mComicIdentifier->currentIndex(), 0 );
    return index.data( Qt::UserRole ).toString();
}
