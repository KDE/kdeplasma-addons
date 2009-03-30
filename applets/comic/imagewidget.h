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

#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <QtGui/QImage>
#include <QGraphicsWidget>

class QGraphicsSceneWheelEvent;

namespace Plasma {
class ScrollBar;
}


class ImageWidget : public QGraphicsWidget
{
    Q_OBJECT

    public:
        ImageWidget ( QGraphicsItem * parent = 0, Qt::WindowFlags wFlags = 0 );
        ~ImageWidget();

        void paint( QPainter *p, const QStyleOptionGraphicsItem *, QWidget* widget = 0 );
        void setScaled( bool isScaled );
        void setImage( const QImage &image );
        void setAvailableSize( const QSizeF &size );
        void setIsLeftToRight( bool ltr );
        void setIsTopToBottom( bool ttb );

    public Q_SLOTS:
        void updateScrollBars();

    protected:
        void wheelEvent( QGraphicsSceneWheelEvent* );

    private Q_SLOTS:
        void slotScroll();

    private:
        void resetScrollBars();

        bool mIsScaled;
        bool mDifferentImage;
        bool mIsLeftToRight;
        bool mIsTopToBottom;
        Plasma::ScrollBar *mScrollBarVert;
        Plasma::ScrollBar *mScrollBarHoriz;
        QImage mImage;
        QImage mScaledImage;
        QRectF mImageRect;
        QSizeF mAvailableSize;
        QSizeF mLastSize;
};

#endif
