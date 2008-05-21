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

#include "comic.h"

#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QPainter>

#include <KConfigDialog>
#include <KRun>

#include <plasma/theme.h>

#include "configwidget.h"

static const int s_arrowWidth = 30;

ComicApplet::ComicApplet( QObject *parent, const QVariantList &args )
    : Plasma::Applet( parent, args ),
      mCurrentDate( QDate::currentDate() ),
      mShowPreviousButton( true ),
      mShowNextButton( false )
{
    setHasConfigurationInterface( true );

    resize( 480, 160 );

    loadConfig();

    updateComic();
    updateButtons();
}

ComicApplet::~ComicApplet()
{
}

void ComicApplet::dataUpdated( const QString&, const Plasma::DataEngine::Data &data )
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

void ComicApplet::createConfigurationInterface( KConfigDialog *parent )
{
    mConfigWidget = new ConfigWidget( parent );
    mConfigWidget->setComicIdentifier( mComicIdentifier );

    parent->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
    parent->addPage( mConfigWidget, parent->windowTitle(), icon() );

    connect( parent, SIGNAL( applyClicked() ), this, SLOT( applyConfig() ) );
    connect( parent, SIGNAL( okClicked() ), this, SLOT( applyConfig() ) );
}

void ComicApplet::applyConfig()
{
    mComicIdentifier = mConfigWidget->comicIdentifier();

    saveConfig();

    updateComic();
}

void ComicApplet::loadConfig()
{
    KConfigGroup cg = config();
    mComicIdentifier = cg.readEntry( "comic", "garfield" );
}

void ComicApplet::saveConfig()
{
    KConfigGroup cg = config();
    cg.writeEntry( "comic", mComicIdentifier );
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

    if ( event->button() == Qt::LeftButton ) { //&& geometry().contains( event->pos() ) ) {
        QFontMetrics fm = Plasma::Theme::defaultTheme()->fontMetrics();

        const QRectF rect = contentsRect();
        if ( mShowPreviousButton && event->pos().x() > rect.left() &&
                                    event->pos().x() < (rect.left() + s_arrowWidth) ) {
            slotPreviousDay();
            event->accept();
        } else if ( mShowNextButton && event->pos().x() > (rect.right() - s_arrowWidth) &&
                                       event->pos().x() < rect.right() ) {
            slotNextDay();
            event->accept();
        //link clicked
        } else if ( !mWebsiteUrl.isEmpty() &&
                    event->pos().y() > (rect.bottom() - fm.height()) &&
                    event->pos().x() > (rect.right() - fm.width( mWebsiteUrl.host() ) - s_arrowWidth) ) {
            KRun::runUrl( mWebsiteUrl, "text/html", 0 );
            event->accept();
        }
    }
}

QSizeF ComicApplet::contentSizeHint() const
{
    if ( !mImage.isNull() ) {
        const QSize size = mImage.size();
        return QSizeF( geometry().width(), (geometry().width() / size.width() ) * size.height() );
    } else
        return geometry().size();
}

void ComicApplet::paintInterface( QPainter *p, const QStyleOptionGraphicsItem*, const QRect &contentRect )
{
    int urlHeight = 0;
    if ( !mWebsiteUrl.isEmpty() ) {
        QFontMetrics fm = Plasma::Theme::defaultTheme()->fontMetrics();
        urlHeight = fm.height();
        int height = contentRect.bottom() - urlHeight;
        p->setPen( Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor) );
        p->drawText( QRectF( contentRect.left(), height, contentRect.width(), fm.height() ),
                     Qt::AlignRight, mWebsiteUrl.host() );
    }

    p->save();
    p->setRenderHint( QPainter::Antialiasing );
    p->setRenderHint( QPainter::SmoothPixmapTransform );

    int leftImageGap = 0;
    int buttonMiddle = (contentRect.height() / 2) + contentRect.top();
    if ( mShowPreviousButton ) {
        QPolygon arrow( 3 );
        arrow.setPoint( 0, QPoint( contentRect.left() + 3, buttonMiddle ) );
        arrow.setPoint( 1, QPoint( contentRect.left() + s_arrowWidth - 5, buttonMiddle - 15 ) );
        arrow.setPoint( 2, QPoint( contentRect.left() + s_arrowWidth - 5, buttonMiddle + 15 ) );

        p->setBrush( Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor) );
        p->drawPolygon( arrow );

        leftImageGap = s_arrowWidth;
    }

    int rightImageGap = 0;
    if ( mShowNextButton ) {
        QPolygon arrow( 3 );
        arrow.setPoint( 0, QPoint( contentRect.right() - 3, buttonMiddle ) );
        arrow.setPoint( 1, QPoint( contentRect.right() - s_arrowWidth + 5, buttonMiddle - 15 ) );
        arrow.setPoint( 2, QPoint( contentRect.right() - s_arrowWidth + 5, buttonMiddle + 15 ) );

        p->setBrush( Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor) );
        p->drawPolygon( arrow );

        rightImageGap = s_arrowWidth;
    }

    QRect imageRect( contentRect.x() + leftImageGap, contentRect.y(),
                     contentRect.width() - (leftImageGap + rightImageGap), contentRect.height() - urlHeight );
    p->drawImage( imageRect, mImage );

    p->restore();
}

void ComicApplet::updateComic( const QString &identifierSuffix )
{
    Plasma::DataEngine *engine = dataEngine( "comic" );
    if ( !engine )
        return;

    const QString identifier = mComicIdentifier + ':' + identifierSuffix;

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
