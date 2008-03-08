/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser/Library General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser/Library General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser/Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "BaseActionWidget.h"
#include <KDebug>
#include <cmath>
#include "Global.h"

#define WIDGET_PADDING 8

#define max(A, B) ((A) >= (B)) ? (A) : (B)

#define setLeft(itemRect, parentRect, alignment) \
     if ((parentRect).width() > (itemRect).width()) { \
         if ((alignment) & Qt::AlignHCenter) \
             (itemRect).moveLeft(WIDGET_PADDING + ((parentRect).width() - (itemRect).width()) / 2); \
         else if ((alignment) & Qt::AlignRight) \
             (itemRect).moveLeft(WIDGET_PADDING + (parentRect).width() - (itemRect).width()); \
     } else { \
         (itemRect).setWidth((parentRect).width()); \
         (itemRect).moveLeft(WIDGET_PADDING); \
     };

namespace Lancelot
{

void BaseActionWidget::init()
{
    setAcceptsHoverEvents(true);
    resize(140, 38);
    setGroupByName("BaseActionWidget");

}

BaseActionWidget::BaseActionWidget(QString name, QString title, QString description, QGraphicsItem * parent)
  : Widget(name, parent), m_icon(NULL), m_iconInSvg(NULL), m_iconSize(32, 32),
    m_innerOrientation(Horizontal), m_alignment(Qt::AlignCenter),
    m_title(title), m_description(description)
{
    init();
}

BaseActionWidget::BaseActionWidget(QString name, QIcon * icon, QString title, QString description, QGraphicsItem * parent)
  : Widget(name, parent), m_icon(icon), m_iconInSvg(NULL), m_iconSize(32, 32),
    m_innerOrientation(Horizontal), m_alignment(Qt::AlignCenter),
    m_title(title), m_description(description)
{
    init();
}

BaseActionWidget::BaseActionWidget(QString name, Plasma::Svg * icon, QString title, QString description, QGraphicsItem * parent)
  : Widget(name, parent), m_icon(NULL), m_iconInSvg(icon), m_iconSize(32, 32),
    m_innerOrientation(Horizontal), m_alignment(Qt::AlignCenter),
    m_title(title), m_description(description)
{
    init();
}

BaseActionWidget::~BaseActionWidget()
{}

/*void BaseActionWidget::resizeSvg() {
    if (m_svg) {
        m_svg->resize(size());
    }
}*/

void BaseActionWidget::paintWidget ( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget ) {
    Q_UNUSED(widget);
    Q_UNUSED(option);


    paintBackground(painter);
    paintForeground(painter);

}

void BaseActionWidget::paintForeground (QPainter * painter) {
    QPainter * _painter = painter;

    // Background Painting
    paintBackground(_painter);

    QPixmap foreground(size().toSize().width(), size().toSize().height());
    foreground.fill(Qt::transparent);

    // Replacing painter with QImage painter
    QPainter fpainter(&foreground);
    painter = &fpainter;

    if (!m_enabled) {
        painter->setPen(QPen(m_group->foregroundColor()->disabled));
    } else if (m_hover) {
        painter->setPen(QPen(m_group->foregroundColor()->active));
    } else {
        painter->setPen(QPen(m_group->foregroundColor()->normal));
    }

    QFont titleFont = painter->font();
    QFont descriptionFont = painter->font();
    descriptionFont.setPointSize(descriptionFont.pointSize() - 2);

    QRectF widgetRect       = QRectF(0, 0, size().width() - 2 * WIDGET_PADDING, size().height() - 2 * WIDGET_PADDING);
    QRectF iconRect         = QRectF(0, 0, m_iconSize.width(), m_iconSize.height());

    if (!m_icon && !m_iconInSvg) iconRect = QRectF(0, 0, 0, 0);

    // painter->setFont(titleFont)); // NOT NEEDED
    QRectF titleRect        = painter->boundingRect(widgetRect,
        Qt::AlignLeft | Qt::AlignTop | Qt::TextSingleLine, m_title);

    painter->setFont(descriptionFont);
    QRectF descriptionRect  = painter->boundingRect(widgetRect,
        Qt::AlignLeft | Qt::AlignTop | Qt::TextSingleLine, m_description);

    if (m_innerOrientation == Vertical || (m_title.isEmpty() && m_description.isEmpty())) {

        // Modified setLeft macro for icon since we can not cut it if it's larger than needed
        // setLeft(iconRect, widgetRect, m_alignment);
        if (m_alignment & Qt::AlignHCenter) {
            iconRect.moveLeft(WIDGET_PADDING + (widgetRect.width() - iconRect.width()) / 2);
        } else if (m_alignment & Qt::AlignRight) {
            iconRect.moveLeft(WIDGET_PADDING + widgetRect.width() - iconRect.width());
        }

        setLeft(titleRect, widgetRect, m_alignment);
        setLeft(descriptionRect, widgetRect, m_alignment);

        float top = WIDGET_PADDING, height =
            iconRect.height() + titleRect.height() + descriptionRect.height();

        if ((m_icon || m_iconInSvg) && !(m_title.isEmpty() && m_description.isEmpty()))
            height += WIDGET_PADDING;

        if (m_alignment & Qt::AlignVCenter)
            top = (widgetRect.height() - height) / 2 + WIDGET_PADDING;
        if (m_alignment & Qt::AlignBottom)
            top = widgetRect.height() - height + WIDGET_PADDING;

        if (m_icon || m_iconInSvg) { // using real painter...
            iconRect.moveTop(top);
            QRect rect(QPoint(lround(iconRect.left()), lround(iconRect.top())), m_iconSize);
            if (m_icon) {
                m_icon->paint(_painter, rect);
            } else {
                m_iconInSvg->resize(m_iconSize);
                m_iconInSvg->paint(_painter, rect.left(), rect.top(), m_hover?"active":"inactive");
            }
            top += m_iconSize.height() + WIDGET_PADDING;
        }

        if (!m_title.isEmpty()) {
            titleRect.moveTop(top);
            painter->setFont(titleFont);
            painter->drawText(titleRect,
                Qt::AlignLeft | Qt::AlignTop | Qt::TextSingleLine | Qt::ElideRight, m_title);
            top += titleRect.height();
        }

        if (!m_description.isEmpty()) {
            descriptionRect.moveTop(top);

            painter->setFont(descriptionFont);
            painter->drawText(descriptionRect,
                Qt::AlignLeft | Qt::AlignTop | Qt::TextSingleLine | Qt::ElideRight, m_description);
        }
    } else {
        float /*left = WIDGET_PADDING,*/ width =
            iconRect.width() + fmaxf(titleRect.width(), descriptionRect.width()) +
            WIDGET_PADDING;

        if (m_alignment & Qt::AlignTop) {
            iconRect.moveTop(WIDGET_PADDING);
            titleRect.moveTop(WIDGET_PADDING);
            descriptionRect.moveTop(titleRect.bottom());
        } else if (m_alignment & (Qt::AlignVCenter | Qt::AlignBottom)) {
            iconRect.moveTop(WIDGET_PADDING +
                    ((m_alignment & Qt::AlignVCenter) ? 0.5 : 1) * (widgetRect.height() - iconRect.height()));
            titleRect.moveTop(WIDGET_PADDING +
                    ((m_alignment & Qt::AlignVCenter) ? 0.5 : 1) * (widgetRect.height() -
                            ((m_description.isEmpty())?0:descriptionRect.height()) - titleRect.height()));
            descriptionRect.moveTop(titleRect.bottom());
        }

        if ((widgetRect.width() < width) || (m_alignment & Qt::AlignLeft)) {
            iconRect.moveLeft(WIDGET_PADDING);
            titleRect.setWidth(widgetRect.width() - ((m_icon || m_iconInSvg) ? iconRect.width() + WIDGET_PADDING : 0));
            descriptionRect.setWidth(titleRect.width());
        } else if (m_alignment & Qt::AlignHCenter) {
            iconRect.moveLeft(WIDGET_PADDING + (widgetRect.width() - width) / 2);
        } else
            iconRect.moveLeft(WIDGET_PADDING + (widgetRect.width() - width));

        titleRect.moveLeft(WIDGET_PADDING + iconRect.right());
        descriptionRect.moveLeft(WIDGET_PADDING + iconRect.right());

        if (m_icon || m_iconInSvg) {  // using real painter...
            QRect rect(QPoint(lround(iconRect.left()), lround(iconRect.top())), m_iconSize);
            if (m_icon) {
                QIcon::Mode mode;
                if (!m_enabled) {
                    mode = QIcon::Disabled;
                } else if (m_hover) {
                    mode = QIcon::Active;
                } else {
                    mode = QIcon::Normal;
                }

                m_icon->paint(_painter, rect, Qt::AlignCenter, mode, QIcon::Off);
            } else {
                m_iconInSvg->resize(m_iconSize);
                m_iconInSvg->paint(_painter, rect.left(), rect.top(), m_hover?"active":"inactive");
            }
        }

        if (!m_title.isEmpty()) {
            painter->setFont(titleFont);
            painter->drawText(titleRect,
                Qt::AlignLeft | Qt::AlignTop | Qt::TextSingleLine, m_title);
        }

        if (!m_description.isEmpty()) {
            if (!m_hover) {
                QPen pen = painter->pen();
                QColor clr = painter->pen().color();
                clr.setAlphaF(0.3);
                painter->setPen(QPen(clr));
            }
            painter->setFont(descriptionFont);
            painter->drawText(descriptionRect,
                Qt::AlignLeft | Qt::AlignTop | Qt::TextSingleLine, m_description);
        }
    }

    QLinearGradient gradient = QLinearGradient(
        QPointF(size().width() - WIDGET_PADDING - 20, 0),
        QPointF(size().width() - WIDGET_PADDING, 0)
    );
    gradient.setColorAt(1, Qt::transparent);
    gradient.setColorAt(0, Qt::black);
    painter->setCompositionMode(QPainter::CompositionMode_DestinationIn);
    painter->fillRect(
        0, 0, (int)size().width(), (int)size().height(),
        gradient);
    //

    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
    _painter->setCompositionMode(QPainter::CompositionMode_SourceOver);

    _painter->drawPixmap(0, 0, foreground);
}

void BaseActionWidget::setIconSize(QSize size) { m_iconSize = size; update(); }
QSize BaseActionWidget::iconSize() const { return m_iconSize; }

void BaseActionWidget::setIcon(QIcon * icon) { m_icon = icon; update(); }
QIcon * BaseActionWidget::icon() const { return m_icon; }

void BaseActionWidget::setIconInSvg(Plasma::Svg * icon) { m_iconInSvg = icon; update(); }
Plasma::Svg * BaseActionWidget::iconInSvg() const { return m_iconInSvg; }

void BaseActionWidget::setTitle(const QString & title) { m_title = title; update(); }
QString BaseActionWidget::title() const { return m_title; }

void BaseActionWidget::setDescription(const QString & description) { m_description = description; update(); }
QString BaseActionWidget::description() const { return m_description; }

void BaseActionWidget::setInnerOrientation(BaseActionWidget::InnerOrientation position) { m_innerOrientation = position; update(); }
BaseActionWidget::InnerOrientation BaseActionWidget::innerOrientation() const { return m_innerOrientation; }

void BaseActionWidget::setAlignment(Qt::Alignment alignment) { m_alignment = alignment; update(); }
Qt::Alignment BaseActionWidget::alignment() const { return m_alignment; }


} // namespace Lancelot

#include "BaseActionWidget.moc"
