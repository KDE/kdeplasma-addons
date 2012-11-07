/*
  Copyright (c) 2007 Paolo Capriotti <p.capriotti@gmail.com>
  Copyright (c) 2010 Dario Andres Rodriguez  <andresbajotierra@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#include "backgrounddelegate.h"

#include <QApplication>
#include <QPainter>
#include <QTextDocument>
#include <QTime>

#include <KDebug>
#include <KGlobalSettings>
#include <KLocalizedString>

#include <Plasma/PaintUtils>

static const int BLUR_PAD = 6;

BackgroundDelegate::BackgroundDelegate(QObject *parent)
    : QAbstractItemDelegate(parent)
{
    m_maxHeight = SCREENSHOT_SIZE/1.6 + BLUR_INCREMENT;
    m_maxWidth = SCREENSHOT_SIZE + BLUR_INCREMENT;
}

void BackgroundDelegate::paint(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    const QString title = index.model()->data(index, Qt::DisplayRole).toString();
    const QString author = index.model()->data(index, AuthorRole).toString();
    const QString resolution = index.model()->data(index, ResolutionRole).toString();
    const QPixmap pix = index.model()->data(index, ScreenshotRole).value<QPixmap>();

    // Highlight selected item
    QStyleOptionViewItemV4 opt(option);
    opt.showDecorationSelected = true;
    QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

    // Draw wallpaper thumbnail
    if (pix.isNull()) {
        painter->fillRect(option.rect, option.palette.brush(QPalette::Base));
    } else {
        // blur calculation
        QImage blur(pix.size() + QSize(BLUR_INCREMENT + BLUR_PAD, BLUR_INCREMENT + BLUR_PAD), QImage::Format_ARGB32);
        QRect blurRect = QRect(QPoint((blur.width() - pix.width()) / 2, (blur.height() - pix.height()) / 2), pix.size());
        blur.fill(Qt::transparent);
        QPainter p(&blur);

        QColor color = option.palette.color(QPalette::Base);
        bool darkBaseColor = qGray(color.rgb()) < 192;
        p.fillRect(blurRect, darkBaseColor ? Qt::white : Qt::black);
        p.end();

        // apply blur with a radius of 2 as thumbnail shadow
        Plasma::PaintUtils::shadowBlur(blur, 2, darkBaseColor ? Qt::white : Qt::black);

        // calculate point
        const int bx = (option.rect.width() - blur.width()) / 2;
        const int by = MARGIN + qMax(0, m_maxHeight - blur.height());
        QRect shadowRect = QRect(option.rect.topLeft(), blur.size()).translated(bx, by);
        // draw the blur
        painter->drawImage(shadowRect.topLeft(), blur);
        // draw the actual thumbnail
        painter->drawPixmap(QRect(shadowRect.topLeft() + QPoint((shadowRect.width() - pix.width()) / 2, (shadowRect.height() - pix.height()) / 2),
                                  pix.size()), pix);
    }

    //Use a QTextDocument to layout the text
    QTextDocument document;
    QString html = QString("<strong>%1</strong>").arg(title);

    if (!author.isEmpty()) {
        QString authorCaption = i18nc("Caption to wallpaper preview, %1 author name",
                              "by %1", author);

        html += QString("<br /><span style=\"font-size: %1pt;\">%2</span>")
                .arg(KGlobalSettings::smallestReadableFont().pointSize())
                .arg(authorCaption);
    }

    if (!resolution.isEmpty()) {
        html += QString("<br /><span style=\"font-size: %1pt;\">%2</span>")
                .arg(KGlobalSettings::smallestReadableFont().pointSize())
                .arg(resolution);
    }

    //Set the text color according to the item state
    QPalette::ColorGroup cg = QPalette::Active;
    if (!(option.state & QStyle::State_Enabled)) {
        cg = QPalette::Disabled;
    } else if (!(option.state & QStyle::State_Active)) {
        cg = QPalette::Inactive;
    }

    QColor color;
    if (option.state & QStyle::State_Selected) {
        color = QApplication::palette().brush(cg, QPalette::HighlightedText).color();
    } else {
        color = QApplication::palette().brush(cg, QPalette::Text).color();
    }

    html = QString("<div style=\"color: %1\" align=\"center\">%2</div>").arg(color.name()).arg(html);

    document.setHtml(html);

    //Calculate positioning
    int x = option.rect.left() + MARGIN;

    //Enable word-wrap
    document.setTextWidth(m_maxWidth);

    //Center text on the row
    int y = option.rect.top() + m_maxHeight + MARGIN * 2; //qMax(0 ,(int)((option.rect.height() - document.size().height()) / 2));

    //Draw text
    painter->save();
    painter->translate(x, y);
    document.drawContents(painter, QRect(QPoint(0, 0), option.rect.size() - QSize(0, m_maxHeight + MARGIN * 2)));
    painter->restore();
}

QSize BackgroundDelegate::sizeHint(const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    Q_UNUSED(option)
    const QString title = index.model()->data(index, Qt::DisplayRole).toString();
    const QString author = index.model()->data(index, AuthorRole).toString();
    const int fontSize = KGlobalSettings::smallestReadableFont().pointSize();

    //Generate a sample complete entry (with the real title) to calculate sizes
    QTextDocument document;
    QString html = QString("<strong>%1</strong><br />").arg(title);
    if (!author.isEmpty()) {
        html += QString("<span style=\"font-size: %1pt;\">by %2</span><br />").arg(fontSize).arg(author);
    }
    html += QString("<span style=\"font-size: %1pt;\">1600x1200</span>").arg(fontSize);

    document.setHtml(html);
    document.setTextWidth(m_maxWidth);

    QSize s(m_maxWidth + MARGIN * 2, 
            m_maxHeight + MARGIN * 3 + (int)(document.size().height()));
    return s;
}

