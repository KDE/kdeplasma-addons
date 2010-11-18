/*
 *   Copyright (C) 2007, 2008, 2009, 2010 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include "BasicWidget.h"
#include "Global.h"

#include <QApplication>
#include <QtGui/QPainter>

#include <KGlobalSettings>

#include <Plasma/PaintUtils>

#include <cmath>

#include <lancelot/lancelot.h>

#define WIDGET_PADDING 8

#define max(A, B) ((A) >= (B)) ? (A) : (B)

namespace Lancelot
{

class BasicWidget::Private {
    public:
    Private(BasicWidget * parent, QString title = QString(), QString description = QString())
      : icon(QIcon()), iconSize(32, 32),
        innerOrientation(Qt::Horizontal), alignment(Qt::AlignCenter),
        title(title), description(description)
    {
        init(parent);
    }

    Private(BasicWidget * parent, QIcon icon, QString title, QString description)
      : icon(icon), iconSize(32, 32),
        innerOrientation(Qt::Horizontal), alignment(Qt::AlignCenter),
        title(title), description(description)
    {
        init(parent);
    }

    Private(BasicWidget * parent, const Plasma::Svg & icon, QString title, QString description)
      : icon(QIcon()), iconSize(32, 32),
        innerOrientation(Qt::Horizontal), alignment(Qt::AlignCenter),
        title(title), description(description)
    {
        iconInSvg.setImagePath(icon.imagePath());
        init(parent);
    }

    void init(BasicWidget * parent)
    {
        parent->setAcceptsHoverEvents(true);
        // parent->resize(140, 38);
        parent->setGroupByName("BasicWidget");
    }

    int shortcutPosition(QString & text)
    {
        Q_UNUSED(text);

        int index = 0;
        while ((index = text.indexOf('&', index)) != -1) {
            if (index == text.size() - 1) {
                return -1;
            }

            if (text.at(index + 1) != '&') {
                return index + 1;
            }

            index++;
            text.remove(index, 1);
        }
        return -1;
    }

    void rotatePainterForIcon(QPainter * painter, qreal angle, QRect & iconRect)
    {
        // TODO: Make this work for other angles
        if (angle != 90 && angle != -90) return;

        painter->rotate(angle);

        iconRect.moveLeft(iconRect.width() / 2);
        iconRect.moveTop(WIDGET_PADDING - iconRect.height() * 1.5);
    }

    QIcon icon;
    Plasma::Svg iconInSvg;
    QSize iconSize;
    Qt::Orientation innerOrientation;

    Qt::Alignment alignment;

    QString title;
    QString description;
};

BasicWidget::BasicWidget(QGraphicsItem * parent)
  : Widget(),
    d(new Private(this))
{
    setParentItem(parent);
}

BasicWidget::BasicWidget(QString title, QString description,
        QGraphicsItem * parent)
  : Widget(),
    d(new Private(this, title, description))
{
    setParentItem(parent);
}

BasicWidget::BasicWidget(QIcon icon, QString title,
        QString description, QGraphicsItem * parent)
  : Widget(),
    d(new Private(this, icon, title, description))
{
    setParentItem(parent);
}

BasicWidget::BasicWidget(const Plasma::Svg & icon, QString title,
        QString description, QGraphicsItem * parent)
  : Widget(),
    d(new Private(this, icon, title, description))
{
    setParentItem(parent);
}

BasicWidget::~BasicWidget()
{
    delete d;
}

void BasicWidget::paint(QPainter * painter,
        const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    Q_UNUSED(widget);
    Q_UNUSED(option);

    paintBackground(painter);
    paintForeground(painter);
}

// macro for setting the left coordinate of items
// relative to the parent and with alignment
// taken into consideration
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

void BasicWidget::paintForeground(QPainter * painter)
{
    bool rtl = QApplication::isRightToLeft();

    QPainter * _painter = painter;

    QPixmap foreground(size().toSize().width(), size().toSize().height());
    foreground.fill(Qt::transparent);

    // Replacing painter with QImage painter
    QPainter fpainter(&foreground);
    painter = &fpainter;

    QColor fgColor;
    if (!isEnabled()) {
        fgColor = group()->foregroundColor()->disabled;
    } else if (isHovered()) {
        fgColor = group()->foregroundColor()->active;
    } else {
        fgColor = group()->foregroundColor()->normal;
    }
    painter->setPen(QPen(fgColor));

    QFont titleFont = painter->font();
    QFont descriptionFont = KGlobalSettings::smallestReadableFont();

    QRectF widgetRect     = QRectF(0, 0, size().width() - 2 * WIDGET_PADDING, size().height() - 2 * WIDGET_PADDING);
    QRectF iconRect;

    if (!d->icon.isNull() || d->iconInSvg.isValid()) {
        iconRect = QRectF(QPointF(), d->iconSize);
        if (iconRect.width() > geometry().width()) {
            iconRect.setWidth(geometry().width());
        }
        if (iconRect.height() > geometry().height()) {
            iconRect.setHeight(geometry().height());
        }
    }

    // painter->setFont(titleFont)); // NOT NEEDED
    QRectF titleRect        = painter->boundingRect(widgetRect,
            Qt::AlignLeft | Qt::AlignTop | Qt::TextSingleLine, d->title);

    painter->setFont(descriptionFont);
    QRectF descriptionRect  = painter->boundingRect(widgetRect,
            Qt::AlignLeft | Qt::AlignTop | Qt::TextSingleLine, d->description);

    if (d->innerOrientation == Qt::Vertical || (d->title.isEmpty() && d->description.isEmpty())) {

        // Modified setLeft macro for icon since we can not cut it if it's larger than needed
        // setLeft(iconRect, widgetRect, d->alignment);

        if (d->alignment & Qt::AlignHCenter) {
            iconRect.moveLeft(WIDGET_PADDING + (widgetRect.width() - iconRect.width()) / 2);
        } else if (d->alignment & Qt::AlignRight) {
            iconRect.moveLeft(WIDGET_PADDING + widgetRect.width() - iconRect.width());
        }

        setLeft(titleRect, widgetRect, d->alignment);
        setLeft(descriptionRect, widgetRect, d->alignment);

        qreal top = WIDGET_PADDING, height =
            iconRect.height() + titleRect.height() + descriptionRect.height();

        if ((!d->icon.isNull() || d->iconInSvg.isValid()) && !(d->title.isEmpty() && d->description.isEmpty()))
            height += WIDGET_PADDING;

        if (d->alignment & Qt::AlignVCenter)
            top = (widgetRect.height() - height) / 2 + WIDGET_PADDING;
        if (d->alignment & Qt::AlignBottom)
            top = widgetRect.height() - height + WIDGET_PADDING;

        if (!d->icon.isNull() || d->iconInSvg.isValid()) { // using real painter...

            iconRect.moveTop(top);
            QRect rect(QPoint(lround(iconRect.left()), lround(iconRect.top())), d->iconSize);

            d->rotatePainterForIcon(_painter, -rotation(), rect);

            if (!d->icon.isNull()) {
                d->icon.paint(_painter, rect);
            } else {
                d->iconInSvg.resize(rect.size());
                d->iconInSvg.paint(_painter, rect.left(), rect.top(), isHovered()?"active":"inactive");
            }
            top += d->iconSize.height() + WIDGET_PADDING;

            d->rotatePainterForIcon(_painter, rotation(), rect);
        }

        if (!d->title.isEmpty()) {
            titleRect.moveTop(top);
            painter->setFont(titleFont);
            // painter->drawText(titleRect,
            //        Qt::AlignLeft | Qt::AlignTop | Qt::TextSingleLine | Qt::ElideRight, d->title);
            drawText(painter, titleRect,
                   Qt::AlignLeft | Qt::AlignTop | Qt::TextSingleLine | Qt::ElideRight, d->title, true);
            top += titleRect.height();
        }

        if (!d->description.isEmpty()) {
            descriptionRect.moveTop(top);

            painter->setFont(descriptionFont);
            // painter->drawText(descriptionRect,
            //         Qt::AlignLeft | Qt::AlignTop | Qt::TextSingleLine | Qt::ElideRight, d->description);
            drawText(painter, descriptionRect,
                    Qt::AlignLeft | Qt::AlignTop | Qt::TextSingleLine | Qt::ElideRight, d->description, false);
        }
    } else {
        // Horizontal layout
        qreal /*left = WIDGET_PADDING,*/ width =
            iconRect.width() + fmaxf(titleRect.width(), descriptionRect.width()) +
            WIDGET_PADDING;

        if (d->alignment & Qt::AlignTop) {
            iconRect.moveTop(WIDGET_PADDING);
            titleRect.moveTop(WIDGET_PADDING);
            descriptionRect.moveTop(titleRect.bottom());
        } else if (d->alignment & (Qt::AlignVCenter | Qt::AlignBottom)) {
            iconRect.moveTop(WIDGET_PADDING +
                    ((d->alignment & Qt::AlignVCenter) ? 0.5 : 1) * (widgetRect.height() - iconRect.height()));
            titleRect.moveTop(WIDGET_PADDING +
                    ((d->alignment & Qt::AlignVCenter) ? 0.5 : 1) * (widgetRect.height() -
                        ((d->description.isEmpty())?0:descriptionRect.height()) - titleRect.height()));
            descriptionRect.moveTop(titleRect.bottom());
        }

        if ((widgetRect.width() < width) || (d->alignment & Qt::AlignLeft)) {
            if (rtl) {
                iconRect.moveRight(widgetRect.right() + WIDGET_PADDING);
                titleRect.setWidth(widgetRect.width() - ((!d->icon.isNull() || d->iconInSvg.isValid()) ? iconRect.width() + WIDGET_PADDING : 0));
                descriptionRect.setWidth(titleRect.width());
            } else {
                iconRect.moveLeft(WIDGET_PADDING);
                titleRect.setWidth(widgetRect.width() - ((!d->icon.isNull() || d->iconInSvg.isValid()) ? iconRect.width() + WIDGET_PADDING : 0));
                descriptionRect.setWidth(titleRect.width());
            }
        } else if (d->alignment & Qt::AlignHCenter) {
            if (rtl) {
                iconRect.moveRight(WIDGET_PADDING + (widgetRect.width() + width) / 2);
            } else {
                iconRect.moveLeft(WIDGET_PADDING + (widgetRect.width() - width) / 2);
            }
        } else {
            if (rtl) {
                iconRect.moveRight(widgetRect.width() - WIDGET_PADDING - (widgetRect.width() - width));
            } else {
                iconRect.moveLeft(WIDGET_PADDING + (widgetRect.width() - width));
            }
        }

        if (rtl) {
            titleRect.moveRight(- WIDGET_PADDING + iconRect.left());
            descriptionRect.moveRight(- WIDGET_PADDING + iconRect.left());
        } else {
            titleRect.moveLeft(WIDGET_PADDING + iconRect.right());
            descriptionRect.moveLeft(WIDGET_PADDING + iconRect.right());
        }

        if (!d->icon.isNull() || d->iconInSvg.isValid()) {  // using real painter...
            QRect rect(QPoint(lround(iconRect.left()), lround(iconRect.top())), d->iconSize);

            d->rotatePainterForIcon(_painter, -rotation(), rect);

            if (!d->icon.isNull()) {
                QIcon::Mode mode;
                if (!isEnabled()) {
                    mode = QIcon::Disabled;
                } else if (isHovered()) {
                    mode = QIcon::Active;
                } else {
                    mode = QIcon::Normal;
                }

                d->icon.paint(_painter, rect, Qt::AlignCenter, mode, QIcon::Off);
            } else {
                d->iconInSvg.resize(d->iconSize);
                d->iconInSvg.paint(_painter, rect.left(), rect.top(), isHovered()?"active":"inactive"); //TODO: add disabled state
            }

            d->rotatePainterForIcon(_painter, rotation(), rect);

        }

        if (!d->title.isEmpty()) {
            painter->setFont(titleFont);
            // painter->drawText(titleRect,
            //         Qt::AlignLeft | Qt::AlignTop | Qt::TextSingleLine, d->title);
            drawText(painter, titleRect,
                    Qt::AlignLeft | Qt::AlignTop | Qt::TextSingleLine, d->title, true);
        }

        if (!d->description.isEmpty()) {
            if (!isHovered()) {
                QPen pen = painter->pen();
                QColor clr = painter->pen().color();
                clr.setAlphaF(0.3);
                painter->setPen(QPen(clr));
            }
            painter->setFont(descriptionFont);
            // painter->drawText(descriptionRect,
            //         Qt::AlignLeft | Qt::AlignTop | Qt::TextSingleLine, d->description);
            drawText(painter, descriptionRect,
                    Qt::AlignLeft | Qt::AlignTop | Qt::TextSingleLine, d->description, false);
        }
    }

    QLinearGradient gradient;
    if (QApplication::isRightToLeft()) {
        gradient = QLinearGradient(
                QPointF(WIDGET_PADDING, 0),
                QPointF(WIDGET_PADDING + 20, 0)
                );
        gradient.setColorAt(0, Qt::transparent);
        gradient.setColorAt(1, Qt::black);
    } else {
        gradient = QLinearGradient(
                QPointF(size().width() - WIDGET_PADDING - 20, 0),
                QPointF(size().width() - WIDGET_PADDING, 0)
                );
        gradient.setColorAt(1, Qt::transparent);
        gradient.setColorAt(0, Qt::black);
    }
    painter->setCompositionMode(QPainter::CompositionMode_DestinationIn);
    painter->fillRect(
            0, 0, (int)ceil(size().width()), (int)ceil(size().height()),
            gradient);
    //

    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
    _painter->setCompositionMode(QPainter::CompositionMode_SourceOver);

    _painter->drawPixmap(0, 0, foreground);
}

void BasicWidget::setIconSize(QSize size)
{
    d->iconSize = size;
    update();
    updateGeometry();
}

QSize BasicWidget::iconSize() const
{
    return d->iconSize;
}

void BasicWidget::setIcon(QIcon icon)
{
    d->icon = icon;
    update();
    updateGeometry();
}

QIcon BasicWidget::icon() const
{
    return d->icon;
}

void BasicWidget::setIconInSvg(const Plasma::Svg & icon)
{
    d->iconInSvg.setImagePath(icon.imagePath());
    update();
    updateGeometry();
}

Plasma::Svg & BasicWidget::iconInSvg() const
{
    return d->iconInSvg;
}

void BasicWidget::setTitle(const QString & value)
{
    d->title = value;

    QString title(value);
    int pos = d->shortcutPosition(title);
    if (pos > -1) {
        setShortcutKey(title.at(pos));
    }

    update();
    updateGeometry();
}

void BasicWidget::setShortcutKey(const QString & key)
{
  Q_UNUSED(key);
}

QString BasicWidget::title() const
{
    return d->title;
}

void BasicWidget::setDescription(const QString & description)
{
    d->description = description;
    update();
    updateGeometry();
}

QString BasicWidget::description() const
{
    return d->description;
}

void BasicWidget::setInnerOrientation(Qt::Orientation position) {
    d->innerOrientation = position;
    update();
    updateGeometry();
}

Qt::Orientation BasicWidget::innerOrientation() const
{
    return d->innerOrientation;
}

void BasicWidget::setAlignment(Qt::Alignment alignment)
{
    d->alignment = alignment;
    update();
}

Qt::Alignment BasicWidget::alignment() const
{
    return d->alignment;
}

QSizeF BasicWidget::sizeHint(Qt::SizeHint which, const QSizeF & constraint) const
{
    QSizeF result = QSizeF();

    switch (which) {
        case Qt::MinimumSize:
            result = d->iconSize;
            break;
        case Qt::MaximumSize:
            result = MAX_WIDGET_SIZE;
            break;
        default:
            // Do we need a more precise sizeHint?
            // result = d->iconSize + QSizeF(2 * WIDGET_PADDING, 2 * WIDGET_PADDING);
            result = d->iconSize;
            QFontMetrics titleMetrics = QFontMetrics(font());
            QFontMetrics desctiprionMetrics =
                    QFontMetrics(KGlobalSettings::smallestReadableFont());
            QSizeF textSize = QSizeF(
                    qMax(
                        titleMetrics.width(d->title),
                        desctiprionMetrics.width(d->description)
                    ),
                    (titleMetrics.height()) +
                    (d->description.isEmpty()?0:desctiprionMetrics.height())
                );

            if (d->innerOrientation == Qt::Horizontal) {
                result.rwidth() += textSize.width();

                if (result.height() < textSize.height()) {
                    result.setHeight(textSize.height());
                }
            } else {
                result.rheight() += textSize.height();

                if (result.width() < textSize.width()) {
                    result.setWidth(textSize.width());
                }
            }
            result += Widget::sizeHint(which, constraint) +
                QSizeF(3 * WIDGET_PADDING, 2 * WIDGET_PADDING);
    }

    if (constraint.isValid()) {
        result = result.boundedTo(constraint);
    }

    return result;
}

void BasicWidget::drawText(QPainter * painter, const QRectF & rectangle, int flags, const QString & txt, bool shortcutEnabled)
{
    Q_UNUSED(flags);

    if (txt.isEmpty()) {
        return;
    }

    QString text = txt;
    int shortcutPosition = d->shortcutPosition(text);
    if (shortcutPosition > -1 && shortcutEnabled) {
        text = text.remove(shortcutPosition - 1, 1);
    } else {
        shortcutEnabled = false;
    }

    static const int radius = 2;
    if (group()->hasProperty("BlurTextShadow")) {
        QColor textColor = painter->pen().color();
        QColor shadowColor;
        if (textColor.valueF() * textColor.alphaF() > 0.4) {
            shadowColor = Qt::black;
        } else {
            shadowColor = Qt::white;
        }

        QPixmap result = Plasma::PaintUtils::shadowText(
                text, textColor, shadowColor,
                QPoint(0, 0), radius);

        if (group()->hasProperty("TextColorBackground")) {
            QColor bgColor;
            if (!isEnabled()) {
                bgColor = group()->backgroundColor()->disabled;
            } else if (isHovered()) {
                bgColor = group()->backgroundColor()->active;
            } else {
                bgColor = group()->backgroundColor()->normal;
            }
            painter->setRenderHint(QPainter::Antialiasing);
            QRectF frect = QRectF(rectangle.topLeft(), result.size());
            painter->fillPath(
                    Plasma::PaintUtils::roundedRectangle(
                        frect, 2 * radius), QBrush(bgColor)
                    );
        }

        painter->drawPixmap(rectangle.topLeft(), result);

        if (shortcutEnabled) {
            int width = painter->boundingRect(
                    rectangle,
                    Qt::AlignLeft | Qt::AlignTop | Qt::TextSingleLine,
                    text.left(shortcutPosition - 1)).width();
            QPixmap result = Plasma::PaintUtils::shadowText(
                    "_", textColor, shadowColor,
                    QPoint(0, 0), radius);
            painter->drawPixmap(rectangle.topLeft() + QPoint(width, 0), result);
        }
    } else {
        if (group()->hasProperty("TextColorBackground")) {
            QColor bgColor;
            if (!isEnabled()) {
                bgColor = group()->backgroundColor()->disabled;
            } else if (isHovered()) {
                bgColor = group()->backgroundColor()->active;
            } else {
                bgColor = group()->backgroundColor()->normal;
            }
            painter->setRenderHint(QPainter::Antialiasing);
            QRectF frect = painter->boundingRect(rectangle,
                    Qt::AlignLeft | Qt::AlignTop | Qt::TextSingleLine, text);
            frect.adjust(- radius, - radius, radius, radius);
            painter->fillPath(
                    Plasma::PaintUtils::roundedRectangle(
                        frect, 2*radius), QBrush(bgColor)
                    );
        }
        painter->drawText(rectangle,
                Qt::AlignLeft | Qt::AlignTop | Qt::TextSingleLine, text);

        if (shortcutEnabled) {
            int width = painter->boundingRect(
                    rectangle,
                    Qt::AlignLeft | Qt::AlignTop | Qt::TextSingleLine,
                    text.left(shortcutPosition - 1)).width();
            painter->drawText(
                    QRectF(rectangle.topLeft() + QPoint(width, 0), rectangle.size()),
                    QString('_'));
        }
    }
}

} // namespace Lancelot

#include "BasicWidget.moc"

