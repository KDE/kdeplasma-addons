/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "BaseWidget.h"
#include <KDebug>
#include <cmath>

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

void BaseWidget::init()
{
    setAcceptsHoverEvents(true);
    resize(140, 38);
}

BaseWidget::BaseWidget(QString name, QString title, QString description, QGraphicsItem * parent)
  : Plasma::Widget(parent), m_hover(false), m_svg(NULL), m_svgElementPrefix(""), 
    m_svgElementSufix(""), m_icon(NULL), m_iconInSvg(NULL), m_iconSize(32, 32), 
    m_innerOrientation(HORIZONTAL), m_alignment(Qt::AlignCenter), 
    m_title(title), m_description(description), m_name(name)
{
    init();
}

BaseWidget::BaseWidget(QString name, QIcon * icon, QString title, QString description, QGraphicsItem * parent)
: Plasma::Widget(parent), m_hover(false), m_svg(NULL), m_svgElementPrefix(""), 
  m_svgElementSufix(""), m_icon(icon), m_iconInSvg(NULL), m_iconSize(32, 32), 
  m_innerOrientation(HORIZONTAL), m_alignment(Qt::AlignCenter),
  m_title(title), m_description(description), m_name(name)
{
    init();
}

BaseWidget::BaseWidget(QString name, Plasma::Svg * icon, QString title, QString description, QGraphicsItem * parent)
: Plasma::Widget(parent), m_hover(false), m_svg(NULL), m_svgElementPrefix(""), 
  m_svgElementSufix(""), m_icon(NULL), m_iconInSvg(icon), m_iconSize(32, 32), 
  m_innerOrientation(HORIZONTAL), m_alignment(Qt::AlignCenter),
  m_title(title), m_description(description), m_name(name) 
{
    init();
}

BaseWidget::~BaseWidget()
{}

void BaseWidget::resize (const QSizeF &size) {
    Plasma::Widget::resize(size);
    resizeSvg();
}

void BaseWidget::resize (qreal width, qreal height) {
    Plasma::Widget::resize(width, height);
    resizeSvg();
}


void BaseWidget::resizeSvg() {
    if (m_svg) m_svg->resize(size());
}

void BaseWidget::hoverEnterEvent ( QGraphicsSceneHoverEvent * event ) {
    m_hover = true;
    Plasma::Widget::hoverEnterEvent(event);
    emit mouseHoverEnter();
    update();
}

void BaseWidget::hoverLeaveEvent ( QGraphicsSceneHoverEvent * event ) {
    m_hover = false;
    Plasma::Widget::hoverEnterEvent(event);
    emit mouseHoverLeave();
    update();
}

void BaseWidget::paintWidget ( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget ) {
    Q_UNUSED(widget);
    Q_UNUSED(option);
    
    // Background Painting
    if (m_svg) {
        resizeSvg();
        QString element = m_svgElementPrefix + (m_hover?"button_active":"button_inactive") + m_svgElementSufix;
        m_svg->paint(painter, 0, 0, element);
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
    
    if (m_innerOrientation == VERTICAL || (m_title.isEmpty() && m_description.isEmpty())) {
        setLeft(iconRect, widgetRect, m_alignment);
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
        
        if (m_icon || m_iconInSvg) {
            iconRect.moveTop(top);
            QRect rect(QPoint(lround(iconRect.left()), lround(iconRect.top())), m_iconSize);
            if (m_icon) {
                m_icon->paint(painter, rect);
            } else {
                m_iconInSvg->resize(m_iconSize);
                m_iconInSvg->paint(painter, rect.left(), rect.top(), m_hover?"active":"inactive");
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
        
        // TODO: Horizontal Alignment
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

        if (m_icon || m_iconInSvg) {
            QRect rect(QPoint(lround(iconRect.left()), lround(iconRect.top())), m_iconSize);
            if (m_icon) {
                m_icon->paint(painter, rect);
            } else {
                m_iconInSvg->resize(m_iconSize);
                m_iconInSvg->paint(painter, rect.left(), rect.top(), m_hover?"active":"inactive");
            }
        }
        
        if (!m_title.isEmpty()) {
            painter->setFont(titleFont);
            painter->drawText(titleRect,
                Qt::AlignLeft | Qt::AlignTop | Qt::TextSingleLine, m_title);
        }
        
        if (!m_description.isEmpty()) {
            painter->setFont(descriptionFont);
            painter->drawText(descriptionRect,
                Qt::AlignLeft | Qt::AlignTop | Qt::TextSingleLine, m_description);
        }
    }
}

void BaseWidget::setSvg(Plasma::Svg * svg) {
    m_svg = svg;
    update();
}

Plasma::Svg * BaseWidget::svg() const {
    return m_svg;
}

void BaseWidget::setIconSize(QSize size) { m_iconSize = size; update(); }
QSize BaseWidget::iconSize() const { return m_iconSize; }

void BaseWidget::setIcon(QIcon * icon) { m_icon = icon; update(); }
QIcon * BaseWidget::icon() const { return m_icon; }

void BaseWidget::setIconInSvg(Plasma::Svg * icon) { m_iconInSvg = icon; update(); }
Plasma::Svg * BaseWidget::iconInSvg() const { return m_iconInSvg; }

void BaseWidget::setName(QString name) { m_name = name; }
QString BaseWidget::name() const { return m_name; }

void BaseWidget::setTitle(QString title) { m_title = title; update(); }
QString BaseWidget::title() const { return m_title; }

void BaseWidget::setDescription(QString description) { m_description = description; update(); }
QString BaseWidget::description() const { return m_description; }

void BaseWidget::setInnerOrientation(BaseWidget::InnerOrientation position) { m_innerOrientation = position; update(); }
BaseWidget::InnerOrientation BaseWidget::innerOrientation() const { return m_innerOrientation; }

void BaseWidget::setAlignment(Qt::Alignment alignment) { m_alignment = alignment; update(); }
Qt::Alignment BaseWidget::alignment() const { return m_alignment; }



} // namespace Lancelot

#include "BaseWidget.moc"
