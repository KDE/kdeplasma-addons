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

#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QPainter>

#include <plasma/theme.h>

#include "comic.h"
#include "configdialog.h"

static const int s_arrowWidth = 30;

ComicApplet::ComicApplet( QObject *parent, const QVariantList &args )
    : Plasma::Applet( parent, args ),
      mCurrentDate( QDate::currentDate() ),
      mConfigDialog( 0 ),
      mShowPreviousButton( true ),
      mShowNextButton( false )
{
    setHasConfigurationInterface( true );

    loadConfig();

    updateComic();
    updateButtons();
}

ComicApplet::~ComicApplet()
{
    delete mConfigDialog;
}

void ComicApplet::dataUpdated( const QString &name, const Plasma::DataEngine::Data &data )
{
    prepareGeometryChange();
    mImage = data[ name ].value<QImage>();
    updateGeometry();
    update();
}

void ComicApplet::showConfigurationInterface()
{
    if ( !mConfigDialog ) {
        mConfigDialog = new ConfigDialog( 0 );
        connect( mConfigDialog, SIGNAL( applyClicked() ), this, SLOT( applyConfig() ) );
        connect( mConfigDialog, SIGNAL( okClicked() ), this, SLOT( applyConfig() ) );
    }

    mConfigDialog->setComicIdentifier( mComicIdentifier );

    mConfigDialog->show();
    mConfigDialog->raise();
}

void ComicApplet::applyConfig()
{
    mComicIdentifier = mConfigDialog->comicIdentifier();

    saveConfig();

    updateComic();
    updateButtons();
}

void ComicApplet::loadConfig()
{
    KConfigGroup cg = config();
    mComicIdentifier = cg.readEntry( "comic", "userfriendly" );
}

void ComicApplet::saveConfig()
{
    KConfigGroup cg = config();
    cg.writeEntry( "comic", mComicIdentifier );
}

void ComicApplet::slotNextDay()
{
    mCurrentDate = mCurrentDate.addDays( 1 );
    updateComic();
    updateButtons();
}

void ComicApplet::slotPreviousDay()
{
    mCurrentDate = mCurrentDate.addDays( -1 );
    updateComic();
    updateButtons();
}

void ComicApplet::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
    event->ignore();

    if ( event->button() == Qt::LeftButton && contentRect().contains(event->pos()) ) {
        if ( event->pos().x() < s_arrowWidth ) {
            slotPreviousDay();
            event->accept();
        } else if ( mShowNextButton && event->pos().x() > contentSizeHint().width() - s_arrowWidth ) {
            slotNextDay();
            event->accept();
        }
    }
}

QSizeF ComicApplet::contentSizeHint() const
{
    if ( !mImage.isNull() ) {
        const QSizeF size = mImage.size();
        return QSizeF( size.width() + 2*s_arrowWidth, size.height() );
    } else
        return QSizeF( 300, 100 );
}

void ComicApplet::paintInterface( QPainter *p, const QStyleOptionGraphicsItem*, const QRect& )
{
    int imageWidth = ( mImage.isNull() ? 300 - 2*s_arrowWidth : mImage.width() );
    int height = ( mImage.isNull() ? 100 : mImage.height() );

    p->save();
    p->setRenderHint( QPainter::Antialiasing );
    if ( mShowPreviousButton ) {
        QPolygon arrow( 3 );
        arrow.setPoint( 0, QPoint( 3, height / 2 ) );
        arrow.setPoint( 1, QPoint( s_arrowWidth - 5, height / 2 - 15 ) );
        arrow.setPoint( 2, QPoint( s_arrowWidth - 5, height / 2 + 15 ) );

        p->setBrush( Plasma::Theme::self()->textColor() );
        p->drawPolygon( arrow );
    }
    if ( mShowNextButton ) {
        QPolygon arrow( 3 );
        arrow.setPoint( 0, QPoint( s_arrowWidth + imageWidth + s_arrowWidth - 3, height / 2 ) );
        arrow.setPoint( 1, QPoint( s_arrowWidth + imageWidth + 5, height / 2 - 15 ) );
        arrow.setPoint( 2, QPoint( s_arrowWidth + imageWidth + 5, height / 2 + 15 ) );

        p->setBrush( Plasma::Theme::self()->textColor() );
        p->drawPolygon( arrow );
    }
    p->restore();

    p->drawImage( s_arrowWidth, 0, mImage );
}

void ComicApplet::updateComic()
{
    Plasma::DataEngine *engine = dataEngine( "comic" );
    if ( !engine )
        return;

    const QString identifier = mComicIdentifier + ":" + mCurrentDate.toString( Qt::ISODate );

    engine->disconnectSource( identifier, this );
    engine->connectSource( identifier, this );

    const Plasma::DataEngine::Data data = engine->query( identifier );
    mImage = data[ identifier ].value<QImage>();

    if ( !mImage.isNull() )
        update();
}

void ComicApplet::updateButtons()
{
    if ( mCurrentDate == QDate::currentDate() )
        mShowNextButton = false;
    else
        mShowNextButton = true;
}

#include "comic.moc"
