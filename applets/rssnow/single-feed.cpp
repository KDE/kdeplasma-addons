/*
 *   Copyright 2008  Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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


//Own
#include "single-feed.h"
#include "feeddata.h"

//KDE
#include <kdebug.h>
#include <kicon.h>
#include <kcursor.h>
#include <kglobalsettings.h>

//Plasma
#include <plasma/svg.h>
#include <plasma/theme.h>

//Qt
#include <QPainter>
#include <QGraphicsItem>
#include <QFont>

//This implementation is based on a great mockup from pinheiro.

QRectF SingleFeedItem::boundingRect() const
{
    return m_rect;
}

SingleFeedItem::SingleFeedItem(QGraphicsItem * parent) : QGraphicsWidget(parent),
                                                         m_displayExtra(true)
{
    m_background = new Plasma::Svg(this);
    m_background->setImagePath("rssnow/background");
}

SingleFeedItem::~SingleFeedItem()
{
}

void SingleFeedItem::paint(QPainter *p, const QStyleOptionGraphicsItem *option,
                           QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    qreal height = m_rect.height();
    qreal width = m_rect.width();

    p->setRenderHint(QPainter::Antialiasing);
    QFont font = KGlobalSettings::smallestReadableFont();

    //paint background
    m_background->paint(p, 0, 0);

    //paint icon (if available)
    if (m_feeditem.icon != 0) {
        if (height > 0 ) {
           m_feeditem.icon->paint(p, 2, 2, 16, 16);
        }
    }

    //draw title
    // FIXME: either remove since we use a hardcoded background
    // or adjust the background color of the svg as well due to theme
    // settings
    p->setPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
    font.setBold(true);
    p->setFont(font);
    QString text;
    if (m_displayExtra) {
        text = m_feeditem.extrainfo;
    } else {
        text = m_feeditem.title;
    }

    m_doc.setHtml(text);
    text = m_doc.toPlainText();
    QFontMetrics fm(font);
    const int titleHeight = qMax(16, fm.height());
    p->drawText(QRectF(22, 2, width - 24, titleHeight),
                Qt::AlignLeft | Qt::AlignBottom, text);

    //draw text
    font.setBold(false);
    p->setFont(font);
    m_doc.setHtml(m_feeditem.text);
    p->drawText(QRectF(2, titleHeight + 4, width - 4, height - titleHeight - 6),
                Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap,
                m_doc.toPlainText());
}

void SingleFeedItem::setRect(const QRect& rect)
{
    //TODO: Hmm, I don't think I'm supposed to fix size / boudingrect this way...
    //Investigate and improve.
    m_rect = rect;
    m_background->resize(m_rect.width(), m_rect.height());
}

int SingleFeedItem::preferredHeight(int width)
{
    QFont font = KGlobalSettings::smallestReadableFont();
    m_doc.setTextWidth(width);
    m_doc.setDefaultFont(font);
    m_doc.setHtml(m_feeditem.text);
    m_doc.setPlainText(m_doc.toPlainText());
    int totalHeight = m_doc.size().height();

    font.setBold(true);
    m_doc.setDefaultFont(font);
    m_doc.setHtml(m_displayExtra ? m_feeditem.extrainfo : m_feeditem.title);
    m_doc.setPlainText(m_doc.toPlainText());
    totalHeight += m_doc.size().height();

    kDebug() << "preferred height is" << totalHeight;
    return totalHeight;
}

void SingleFeedItem::setFeedData(FeedData feeddata)
{
    //set feed item and repaint
    m_feeditem = feeddata;
    if (!feeddata.url.isEmpty()) {
        setCursor(Qt::PointingHandCursor);
    } else {
        setCursor(Qt::ArrowCursor);
    }

    update();
}

FeedData SingleFeedItem::feedData() const
{
    return m_feeditem;
}

int SingleFeedItem::itemNumber() const
{
    return m_feeditem.itemNumber;
}

void SingleFeedItem::setDisplayExtra(bool extra)
{
    m_displayExtra = extra;
}

bool SingleFeedItem::displayExtra() const
{
    return m_displayExtra;
}
