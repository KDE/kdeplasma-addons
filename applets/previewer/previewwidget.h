/*
 *  Copyright 2008 by Alessandro Diaferia <alediaferia@gmail.com>

 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of 
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

#include <QGraphicsWidget>
#include <QStyleOptionViewItem>

#include <KFileItemDelegate>
#include <KUrl>

namespace Plasma{
class ScrollBar;
class FrameSvg;
class Svg;
}

class QStyleOptionViewItemV4;
class KUrl;
class QGraphicsSceneWheelEvent;
class QGraphicsSceneResizeEvent;
class KFileItem;
class QTimeLine;

class PreviewWidget : public QGraphicsWidget
{
    Q_OBJECT

    public:
        PreviewWidget(QGraphicsItem *parent);
        ~PreviewWidget();

        void paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 );

        /**
         * Call this method whenever the item list changes.
         * This method takes care of updating the view.
         */
        void setItemsList(const QList<QUrl> &);

        QSize iconSize() const;
        int bottomBorderHeight() const;
        static int suggestedWidth();

        static const int s_spacing = 2;
        static const int s_logoSize = 27;
        static const int s_topBorder = s_logoSize + 8;

    public slots:
        void expand();
        void contract();
        void expandingSlot(qreal progress);

    protected:
        void layoutItems();
        void lookForPreview();
        void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
        void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
        void updatePreview();
        void wheelEvent(QGraphicsSceneWheelEvent *event);
        void updateSelectedItems(const QPoint &);
        void updateHoveredItems(const QPoint &);
        void dropEvent(QGraphicsSceneDragDropEvent *);
        void resizeEvent(QGraphicsSceneResizeEvent *event);
        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

        /**
         * Draws an open/close arrow in the top right corner.
         */
        void drawOpenCloseArrow(QPainter *);

    private slots:
        virtual void setPreview(const KFileItem &, const QPixmap &);
        virtual void scrolled(int);
        void setupOptionViewItem();
        void calculateRects();

    signals:
        void fileOpenRequested(const KUrl &);
        void urlsDropped(const KUrl::List &);

    private:
       Plasma::ScrollBar *m_scrollBar;
       KFileItemDelegate m_delegate;
       QRect m_itemsRect;
       QVector<QRect> m_items;
       QMap<KUrl, QPixmap> m_previews;
       int m_selectedIndex;
       int m_hoveredIndex;
       bool m_previewReady;
       QStyleOptionViewItemV4 m_option;
       QRect m_previewRect;
       bool m_layoutIsValid;
       Plasma::FrameSvg *m_panel;
       Plasma::Svg *m_logo;
       KUrl m_hoveredUrl;
       QList<QUrl> m_previewHistory;
       QPoint m_clicked;
       bool m_closeStatus; // defines whether the widget should be drawn open or close
       QRect m_arrowRect;
       int m_animationHeight;
       int m_animId;
};

#endif
