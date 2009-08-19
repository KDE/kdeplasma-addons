/*
    Copyright 2008 Marco Martin <notmart@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

// Own
#include "weatherdelegate.h"

#include <cmath>
#include <math.h>

// Qt
#include <QFontMetrics>
#include <QIcon>
#include <QModelIndex>
#include <QPainter>
#include <QStyleOptionViewItem>

// KDE
#include <KDebug>
#include <KColorScheme>
#include <KIconLoader>

// plasma
#include <plasma/paintutils.h>
#include <plasma/theme.h>


class WeatherDelegatePrivate
{
public:
    WeatherDelegatePrivate()
    {
    }

    ~WeatherDelegatePrivate() {
    }

};




WeatherDelegate::WeatherDelegate(QObject *parent)
        : QAbstractItemDelegate(parent),
        d(new WeatherDelegatePrivate)
{
}

WeatherDelegate::~WeatherDelegate()
{
    delete d;
}

bool WeatherDelegate::helpEvent(QHelpEvent *event, QAbstractItemView* view, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    QVariant toolTip = index.data(Qt::ToolTipRole);

    // No tooltip data
    if (!toolTip.isValid()) {
        return false;
    }

    if (index.data() != toolTip) {
        return QAbstractItemDelegate::helpEvent(event, view, option, index);
    }

    return QAbstractItemDelegate::helpEvent(event, view, option, index);
}

void WeatherDelegate::paint(QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QFontMetrics fm(option.font);
    const int rows = index.model()->rowCount();
    const int columns = index.model()->columnCount();
    const int radius = 5;
    bool drawLeft = false;
    bool drawRight = false;


    QColor backgroundColor(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
    backgroundColor.setAlphaF((((qreal)index.row() + 1) / (qreal)rows)*0.3);


    QRect backgroundRect(option.rect);

    backgroundRect.setHeight(qMin(int(fm.height() * 1.2), option.rect.height()-4));
    backgroundRect.moveCenter(option.rect.center());

    if (index.column() == 0) {
        drawLeft = true;
    }
    if (index.column() == columns - 1) {
        drawRight = true;
    }

    if (!drawLeft && !drawRight) {
        painter->fillRect(backgroundRect, backgroundColor);
    } else {
        QPainterPath path;
        QRect backgroundRectToClip(backgroundRect);

        if (!drawLeft) {
            backgroundRectToClip.setLeft(backgroundRectToClip.left()-radius);
        }
        if (!drawRight) {
            backgroundRectToClip.setRight(backgroundRectToClip.right()+radius);
        }

        path = Plasma::PaintUtils::roundedRectangle(backgroundRectToClip, radius);


        QPainterPath clipPath;
        clipPath.addRect(backgroundRect);
        path = path.intersected(clipPath);

        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(Qt::NoPen);
        painter->setBrush(backgroundColor);
        painter->drawPath(path);
    }

    QSize decorationSize(option.decorationSize);
    decorationSize = decorationSize.boundedTo(QSize(option.rect.size().width(), option.rect.size().width()));

    if (decorationSize.width() > KIconLoader::SizeSmall && decorationSize.width() < KIconLoader::SizeSmallMedium) {
        decorationSize = QSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
    } else if (decorationSize.width() > KIconLoader::SizeSmallMedium && decorationSize.width() < KIconLoader::SizeMedium) {
        decorationSize = QSize(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium);
    } else if (decorationSize.width() > KIconLoader::SizeMedium && decorationSize.width() < KIconLoader::SizeLarge) {
        decorationSize = QSize(KIconLoader::SizeMedium, KIconLoader::SizeMedium);
    }

    QString titleText = index.data(Qt::DisplayRole).value<QString>();
    QFont titleFont = qvariant_cast<QFont>(index.data(Qt::FontRole)).resolve(option.font);
    QColor titleColor = qvariant_cast<QColor>(index.data(Qt::ForegroundRole));
    QIcon decorationIcon = index.data(Qt::DecorationRole).value<QIcon>();

    Qt::Alignment decorationAlignment = Qt::AlignCenter;
    if (!titleText.isNull()) {
        decorationAlignment |= option.decorationPosition == QStyleOptionViewItem::Left ?                                                                        Qt::AlignLeft : Qt::AlignRight;
    }

    QRect decorationRect = QStyle::alignedRect(option.direction,
                           decorationAlignment,
                           decorationSize,
                           option.rect);

    QRect titleRect = QStyle::alignedRect(option.direction,
                                          option.decorationPosition == QStyleOptionViewItem::Left ?
                                          Qt::AlignRight : Qt::AlignLeft,
                                          option.rect.size(),
                                          option.rect);

    if (!decorationIcon.isNull() && !titleText.isNull()) {
        QRect textRect = fm.boundingRect(titleText);
        textRect.moveCenter(titleRect.center());
        if (option.direction == Qt::LeftToRight) {
            decorationRect.moveRight(textRect.left());
        } else {
            decorationRect.moveLeft(textRect.right());
        }
    }

    decorationIcon.paint(painter, decorationRect, option.decorationAlignment);

    if (titleColor.isValid()) {
        painter->setPen(titleColor);
    } else {
        painter->setPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
    }
    painter->setFont(titleFont);
    painter->drawText(titleRect, Qt::AlignCenter, titleText);
}

QSize WeatherDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    //Q_UNUSED(index)
    QSize size = option.rect.size();

    QFontMetrics metrics(option.font);

    size.setHeight(qMax(option.decorationSize.height(), metrics.height()));

    size.setWidth(option.decorationSize.width() + metrics.width(index.data(Qt::DisplayRole).value<QString>()));

    return size;
}
