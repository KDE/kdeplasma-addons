/***************************************************************************
 *   Copyright (C) 2009 Matthias Fuchs <mat69@gmx.net>                     *
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

#include "imagewidget.h"

#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QStyleOptionGraphicsItem>

#include <Plasma/ScrollBar>

ImageWidget::ImageWidget( QGraphicsItem *parent, Qt::WindowFlags wFlags )
    : QGraphicsWidget( parent, wFlags ), mIsScaled( true ), mIsLeftToRight( true ),
      mIsTopToBottom( true ), mLastSize( QSizeF( 200, 200 ) )
{
    setCacheMode( ItemCoordinateCache );//TODO change back to DeviceCoordinateCache, when that works with Dashboard
    mScrollBarHoriz = new Plasma::ScrollBar( this );
    mScrollBarHoriz->setOrientation( Qt::Horizontal );
    mScrollBarHoriz->setSingleStep( 80 );
    mScrollBarHoriz->nativeWidget()->setSizePolicy( QSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Maximum ) );
    mScrollBarHoriz->hide();
    connect( mScrollBarHoriz, SIGNAL( valueChanged( int ) ), this, SLOT( slotScroll() ) );

    mScrollBarVert = new Plasma::ScrollBar( this );
    mScrollBarVert->setSingleStep( 80 );
    mScrollBarVert->nativeWidget()->setSizePolicy( QSizePolicy( QSizePolicy::Maximum, QSizePolicy::MinimumExpanding ) );
    mScrollBarVert->hide();
    connect( mScrollBarVert, SIGNAL( valueChanged( int ) ), this, SLOT( slotScroll() ) );
}

ImageWidget::~ImageWidget()
{
}

void ImageWidget::slotScroll()
{
    update( this->rect() );
}

void ImageWidget::updateScrollBars()
{
    if ( !mIsScaled ) {
        QSizeF temp = this->size();
        const int scrollWidthSpace = mScrollBarVert->preferredSize().width();
        const int scrollHeightSpace = mScrollBarHoriz->preferredSize().height();

        bool hasScrollBarHoriz = mImage.width() > temp.width();
        bool hasScrollBarVert = mImage.height() + hasScrollBarHoriz * scrollHeightSpace  > temp.height();
        hasScrollBarHoriz = mImage.width() + hasScrollBarVert * scrollWidthSpace > temp.width();

        mScrollBarVert->setVisible( hasScrollBarVert );
        mScrollBarVert->setRange( 0, mImage.height() - mImageRect.height() );
        mScrollBarVert->setPageStep( mImageRect.height() );

        mScrollBarHoriz->setVisible( hasScrollBarHoriz );
        mScrollBarHoriz->setRange( 0, mImage.width() - mImageRect.width() );
        mScrollBarHoriz->setPageStep( mImageRect.width() );
    }
}

void ImageWidget::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget* )
{
    QRectF exposed = option->exposedRect;
    QRectF contentRect = this->rect();
    mImageRect = contentRect;

    if ( !mIsScaled ) {
        //check for scrollbars and calculate where they should be
        const int scrollBarWidth = mScrollBarVert->preferredSize().width();
        const int scrollBarHeight = mScrollBarHoriz->preferredSize().height();
        if ( mScrollBarVert->isVisible() ) {
            QRect rectScroll = QRect( contentRect.right() - scrollBarWidth,
                                      contentRect.top(),
                                      scrollBarWidth,
                                      contentRect.height() - mScrollBarHoriz->isVisible() * scrollBarHeight );

            mScrollBarVert->setGeometry( rectScroll );
            mImageRect.setRight( mImageRect.right() - scrollBarWidth );
        }
        if ( mScrollBarHoriz->isVisible() ) {
            QRect rectScroll = QRect( contentRect.left(),
                                      contentRect.bottom() - scrollBarHeight,
                                      contentRect.width() - mScrollBarVert->isVisible() * scrollBarWidth,
                                      scrollBarHeight );

            mScrollBarHoriz->setGeometry( rectScroll );
            mImageRect.setBottom( mImageRect.bottom() - scrollBarHeight );
        }
        updateScrollBars();

        //adapt the exposed rect to only draw the parts of the image that are really needed
        if ( ( exposed.left() >= mImageRect.right() ) || ( exposed.top() >= mImageRect.bottom() ) ) {
            return;
        }
        if ( exposed.right() > mImageRect.right() ) {
            exposed.setRight( mImageRect.right() );
        }
        if ( exposed.bottom() > mImageRect.bottom() ) {
            exposed.setBottom( mImageRect.bottom() );
        }

        //the displayed image has changed -- either a new one has been set, or it was just set to not scaled
        if ( mDifferentImage ) {
            mScaledImage = mImage;
        }
    } else if ( mDifferentImage || ( mScaledImage.size() != mImageRect.size() ) ) {
        mScaledImage = mImage.scaled( mImageRect.size().toSize(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
        mDifferentImage = false;
    }

    QPoint scrollPos = mIsScaled ? QPoint() : QPoint( mScrollBarHoriz->value(), mScrollBarVert->value() );
    QRectF shownImageRect = QRectF( scrollPos + exposed.topLeft(), exposed.size() );
    p->drawImage( exposed, mScaledImage, shownImageRect );
}

void ImageWidget::resetScrollBars()
{
    mScrollBarVert->hide();
    mScrollBarHoriz->hide();
    int vertical = mIsTopToBottom ? 0 : mImage.height() - mImageRect.height();
    int horizontal = mIsLeftToRight ? 0 : mImage.width() - mImageRect.width();
    mScrollBarVert->setValue( vertical );
    mScrollBarHoriz->setValue( horizontal );
}

void ImageWidget::wheelEvent( QGraphicsSceneWheelEvent *event )
{
    const QPointF eventPos = event->pos();
    const int numDegrees = event->delta() / 8;
    const int numSteps = numDegrees / 15;

    if ( mScrollBarVert->isVisible() && ( event->modifiers() == Qt::NoModifier ) &&
         !mScrollBarHoriz->isUnderMouse() ) {
        const int scroll = mScrollBarVert->singleStep();
        mScrollBarVert->setValue( mScrollBarVert->value() - numSteps * scroll );

        event->accept();
        update( this->rect() );
    } else if ( mScrollBarHoriz->isVisible() &&
                ( ( event->modifiers() == Qt::AltModifier ) || ( event->modifiers() == Qt::NoModifier ) ) ) {
        const int scroll = mScrollBarHoriz->singleStep();
        mScrollBarHoriz->setValue( mScrollBarHoriz->value() - numSteps * scroll );

        event->accept();
        update( this->rect() );
    }
    QGraphicsItem::wheelEvent( event );
}

void ImageWidget::setImage( const QImage &image )
{
    mImage = image;
    mDifferentImage = true;
    resetScrollBars();
    updateScrollBars();
    update( this->rect() );
}

void ImageWidget::setScaled( bool isScaled )
{
    mIsScaled = isScaled;
    resetScrollBars();
    if ( !mIsScaled ) {
        mDifferentImage = true;
        updateScrollBars();
    }
}

void ImageWidget::setAvailableSize( const QSizeF &available )
{
    mAvailableSize = available;

    if ( !mImage.isNull() && mImage.size().width() > 0 ) {
        qreal finalWidth = mAvailableSize.width();
        qreal finalHeight = mAvailableSize.height();

        resetScrollBars();

        // uses the mIdealSize, as long as it is not larger, than the maximum size
        if ( mIsScaled ) {
            qreal aspectRatio = qreal( mImage.size().height() ) / mImage.size().width();
            qreal imageHeight =  aspectRatio * mAvailableSize.width();

            // set height (width) for given width (height) keeping image aspect ratio
            if ( imageHeight <= mAvailableSize.height() ) {
                finalHeight = imageHeight;
            } else {
                finalWidth = mAvailableSize.height() / aspectRatio;
            }

            mLastSize = QSizeF( finalWidth, finalHeight );
        } else {
            const int scrollWidthSpace = mScrollBarVert->preferredSize().width();
            const int scrollHeightSpace = mScrollBarHoriz->preferredSize().height();

            bool hasScrollBarHoriz = mImage.width() > mAvailableSize.width();
            bool hasScrollBarVert = mImage.height() + hasScrollBarHoriz * scrollHeightSpace  > mAvailableSize.height();
            hasScrollBarHoriz = mImage.width() + hasScrollBarVert * scrollWidthSpace > mAvailableSize.width();

            mScrollBarHoriz->setVisible( hasScrollBarHoriz );
            mScrollBarVert->setVisible( hasScrollBarVert );

            finalWidth = hasScrollBarHoriz ? mAvailableSize.width() : mImage.width() + hasScrollBarVert * scrollWidthSpace;
            finalHeight = hasScrollBarVert ? mAvailableSize.height() : mImage.height() + hasScrollBarHoriz * scrollHeightSpace;

            mLastSize = QSizeF( finalWidth, finalHeight );
        }

        setPreferredSize( mLastSize );
    }
}

void ImageWidget::setIsLeftToRight( bool ltr )
{
    mIsLeftToRight = ltr;
}

void ImageWidget::setIsTopToBottom( bool ttb )
{
    mIsTopToBottom = ttb;
}



#include "imagewidget.moc"
