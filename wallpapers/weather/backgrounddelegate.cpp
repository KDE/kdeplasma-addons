/*
  Copyright (c) 2007 Paolo Capriotti <p.capriotti@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#include "backgrounddelegate.h"

#include <QPen>
#include <QPainter>

#include <KGlobalSettings>
#include <KLocalizedString>

BackgroundDelegate::BackgroundDelegate(QObject *listener, float ratio, QObject *parent)
    : QAbstractItemDelegate(parent),
      m_listener(listener),
      m_ratio(ratio)
{
}

void BackgroundDelegate::paint(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    QString title = index.model()->data(index, Qt::DisplayRole).toString();
    QString author = index.model()->data(index, AuthorRole).toString();
    QString resolution = index.model()->data(index, ResolutionRole).toString();
    QPixmap pix = index.model()->data(index, ScreenshotRole).value<QPixmap>();

    // draw selection outline
    if (option.state & QStyle::State_Selected) {
        QPen oldPen = painter->pen();
        painter->setPen(option.palette.color(QPalette::Highlight));
        painter->drawRect(option.rect.adjusted(2, 2, -2, -2));
        painter->setPen(oldPen);
    }

    // draw pixmap
    int maxheight = SCREENSHOT_SIZE;
    int maxwidth = int(maxheight * m_ratio);
    if (!pix.isNull()) {
        int x = MARGIN + (maxwidth - pix.width()) / 2;
        int y = MARGIN + (maxheight - pix.height()) / 2;
        QRect imgRect = QRect(option.rect.topLeft(), pix.size()).translated(x, y);
        painter->drawPixmap(imgRect, pix);
    }

    // draw text
    painter->save();
    int x = option.rect.left() + MARGIN * 2 + maxwidth;

    QRect textRect(x,
                   option.rect.top() + MARGIN,
                   option.rect.width() - x - MARGIN,
                   maxheight);

    const QRect boundingRect = painter->boundingRect(textRect, Qt::TextWordWrap, title) & option.rect;
    painter->drawText(boundingRect, Qt::TextWordWrap, title);
    QRect titleRect = painter->boundingRect(boundingRect, Qt::TextWordWrap, title);
    QPoint lastText(titleRect.bottomLeft());

    // Borrowed from Dolphin for consistency and beauty.
    // For the color of the additional info the inactive text color
    // is not used as this might lead to unreadable text for some color schemes. Instead
    // the text color is slightly mixed with the background color.
    const QColor textColor = option.palette.text().color();
    const QColor baseColor = option.palette.base().color();
    const int p1 = 70;
    const int p2 = 100 - p1;
    const QColor detailsColor = QColor((textColor.red() * p1 + baseColor.red() * p2) / 100,
                                       (textColor.green() * p1 + baseColor.green() * p2) / 100,
                                       (textColor.blue() * p1 + baseColor.blue() * p2) /  100);
    if (!resolution.isEmpty()) {
        QRect resolutionRect = QRect(lastText, textRect.size()) & option.rect;

        if (!resolutionRect.isEmpty()) {
            painter->setPen(detailsColor);
            painter->drawText(resolutionRect, Qt::TextWordWrap, resolution);
            lastText = painter->boundingRect(resolutionRect, Qt::TextWordWrap, resolution).bottomLeft();
        }
    }

    if (!author.isEmpty()) {
        QRect authorRect = QRect(lastText, textRect.size()) & option.rect;

        if (!authorRect.isEmpty()) {
            painter->setPen(detailsColor);
            painter->drawText(authorRect, Qt::TextWordWrap, author);
            lastText = painter->boundingRect(authorRect, Qt::TextWordWrap, author).bottomLeft();
        }
    }

    painter->restore();
}

QSize BackgroundDelegate::sizeHint(const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    const QString title = index.model()->data(index, Qt::DisplayRole).toString();
    const int maxwidth = int(SCREENSHOT_SIZE * m_ratio);
    QFontMetrics fm(option.font);
    //kDebug() << QSize(maxwidth + qBound(100, fm.width(title), 500), Background::SCREENSHOT_SIZE + MARGIN * 2);
    return QSize(maxwidth + qBound(100, fm.width(title), 500), SCREENSHOT_SIZE + MARGIN * 2);
}

