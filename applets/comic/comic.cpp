/***************************************************************************
 *   Copyright (C) 2007 by Tobias Koenig <tokoe@kde.org>                   *
 *   Copyright (C) 2008 by Marco Martin <notmart@gmail.com>                *
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

#include <KRun>

#include <plasma/theme.h>

#include "comic.h"
#include "configdialog.h"

static const int s_arrowWidth = 30;

ComicApplet::ComicApplet( QObject *parent, const QVariantList &args )
    : Plasma::Applet( parent, args ),
      mCurrentDate( QDate::currentDate() ),
      mConfigDialog( 0 ),
      mScaleComic( true ),
      mShowPreviousButton( true ),
      mShowNextButton( false )
{
    setHasConfigurationInterface( true );

    setContentSize( QSizeF( 300, 100 ) );

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
    mImage = data[ "Image" ].value<QImage>();
    mWebsiteUrl = data[ "Website Url" ].value<KUrl>();
    mNextIdentifierSuffix = data[ "Next identifier suffix" ].toString();
    mPreviousIdentifierSuffix = data[ "Previous identifier suffix" ].toString();

    updateButtons();

    if ( !mImage.isNull() ) {
        prepareGeometryChange();
        updateGeometry();
        update();
    }
}

void ComicApplet::showConfigurationInterface()
{
    if ( !mConfigDialog ) {
        mConfigDialog = new ConfigDialog( 0 );
        connect( mConfigDialog, SIGNAL( applyClicked() ), this, SLOT( applyConfig() ) );
        connect( mConfigDialog, SIGNAL( okClicked() ), this, SLOT( applyConfig() ) );
    }

    mConfigDialog->setComicIdentifier( mComicIdentifier );
    mConfigDialog->setScaleComic( mScaleComic );

    mConfigDialog->show();
    mConfigDialog->raise();
}

void ComicApplet::applyConfig()
{
    mComicIdentifier = mConfigDialog->comicIdentifier();
    mScaleComic = mConfigDialog->scaleComic();

    saveConfig();

    updateComic();
}

void ComicApplet::loadConfig()
{
    KConfigGroup cg = config();
    mComicIdentifier = cg.readEntry( "comic", "userfriendly" );
    mScaleComic = cg.readEntry( "scaleComic", true );
}

void ComicApplet::saveConfig()
{
    KConfigGroup cg = config();
    cg.writeEntry( "comic", mComicIdentifier );
    cg.writeEntry( "scaleComic", mScaleComic );
}

void ComicApplet::slotNextDay()
{
    updateComic( mNextIdentifierSuffix );
}

void ComicApplet::slotPreviousDay()
{
    updateComic( mPreviousIdentifierSuffix );
}

void ComicApplet::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
    event->ignore();

    if ( event->button() == Qt::LeftButton && contentRect().contains( event->pos() ) ) {
        QFontMetrics fm = Plasma::Theme::self()->fontMetrics();

        if ( mShowPreviousButton && event->pos().x() < s_arrowWidth ) {
            slotPreviousDay();
            event->accept();
        } else if ( mShowNextButton && event->pos().x() > contentSizeHint().width() - s_arrowWidth ) {
            slotNextDay();
            event->accept();
        //link clicked
        } else if ( !mWebsiteUrl.isEmpty() &&
                    event->pos().y() > contentSizeHint().height() - fm.height() &&
                    event->pos().x() > contentSizeHint().width() - fm.width( mWebsiteUrl.host() ) - s_arrowWidth ) {
            KRun::runUrl( mWebsiteUrl, "text/html", 0 );
            event->accept();
        }
    }
}

QSizeF ComicApplet::contentSizeHint() const
{
    if ( !mImage.isNull() ) {
        const QSizeF size = mImage.size();

        if ( mScaleComic ) {
            return QSizeF( contentSize().width(), (contentSize().width() / size.width() ) * size.height() );
        } else {
            return QSizeF( size.width() + 2 * s_arrowWidth, size.height() + Plasma::Theme::self()->fontMetrics().height() );
        }
    } else
        return contentSize();
}

void ComicApplet::paintInterface( QPainter *p, const QStyleOptionGraphicsItem*, const QRect& )
{
    int imageWidth = ( mImage.isNull() ? 300 : contentSize().width() ) - 2 * s_arrowWidth;
    int height = ( mImage.isNull() ? 100 : contentSize().height() );

    if ( !mWebsiteUrl.isEmpty() ) {
        QFontMetrics fm = Plasma::Theme::self()->fontMetrics();
        height -= fm.height();
        p->setPen( Plasma::Theme::self()->textColor() );
        p->drawText( QRectF( s_arrowWidth, height, imageWidth, fm.height() ),
                     Qt::AlignRight, mWebsiteUrl.host() );
    }

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

    p->drawImage( QRectF( s_arrowWidth, 0, imageWidth, height ), mImage );

    p->restore();
}

void ComicApplet::updateComic( const QString &identifierSuffix )
{
    Plasma::DataEngine *engine = dataEngine( "comic" );
    if ( !engine )
        return;

    const QString identifier = mComicIdentifier + ":" + identifierSuffix;

    engine->disconnectSource( identifier, this );
    engine->connectSource( identifier, this );

    const Plasma::DataEngine::Data data = engine->query( identifier );
}

void ComicApplet::updateButtons()
{
    if ( mNextIdentifierSuffix.isNull() )
        mShowNextButton = false;
    else
        mShowNextButton = true;

    if ( mPreviousIdentifierSuffix.isNull() )
        mShowPreviousButton = false;
    else
        mShowPreviousButton = true;
}

#include "comic.moc"
