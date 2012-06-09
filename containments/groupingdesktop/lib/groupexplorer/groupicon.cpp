/*
 *   Copyright (C) 2009 by Ana Cecília Martins <anaceciliamb@gmail.com>
 *   Copyright (C) 2010 by Chani Armitage <chani@kde.org>
 *   Copyright 2010 by Giulio Camuffo <giuliocamuffo@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library/Lesser General Public License
 *   version 2, or (at your option) any later version, as published by the
 *   Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library/Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "groupicon.h"

#include <QApplication>
#include <QCursor>
#include <QFontMetrics>
#include <QGraphicsSceneMouseEvent>
#include <QStyleOptionGraphicsItem>
#include <QPainter>

#include <KIconLoader>
#include <KIcon>
#include <KGlobalSettings>

#include <Plasma/FrameSvg>
#include <Plasma/Theme>
#include <Plasma/PaintUtils>
#include <QWidget>

#include "abstractgroup.h"

GroupIcon::GroupIcon(const QString &name)
         : QGraphicsWidget(0),
           m_background(new Plasma::FrameSvg(this)),
           m_id(name),
           m_iconHeight(DEFAULT_ICON_SIZE)
{
    setCacheMode(DeviceCoordinateCache);
    setAcceptHoverEvents(true);
    m_background->setImagePath("widgets/background");

    GroupInfo gi = AbstractGroup::groupInfo(name);
    m_name = gi.prettyName();
    m_icon = KIcon(gi.icon());
    setCursor(Qt::OpenHandCursor);
}

GroupIcon::~GroupIcon()
{
}

void GroupIcon::resizeEvent(QGraphicsSceneResizeEvent *)
{
    m_background->resizeFrame(size());
    qreal l, t, r, b;
    m_background->getMargins(l, t, r, b);
    setContentsMargins(l, t, r, b);
}

QSizeF GroupIcon::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    if (which == Qt::MinimumSize || which == Qt::PreferredSize) {
        qreal l, t, r, b;
        getContentsMargins(&l, &t, &r, &b);
        QFontMetrics fm(font());
        const int minHeight = m_iconHeight + 2 + fm.height();
        QSizeF s(qMax(m_iconHeight, fm.width(m_name)) + l + r, minHeight + t + b);
        return s;
    }

    return QGraphicsWidget::sizeHint(which, constraint);
}


void GroupIcon::setIconSize(int height)
{
    m_iconHeight = height;
    updateGeometry();
    update();
}

void GroupIcon::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
    emit hoverEnter(this);
}

void GroupIcon::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton &&
        (event->pos() - event->buttonDownPos(Qt::LeftButton)).toPoint().manhattanLength() > QApplication::startDragDistance()) {
        event->accept();
        QMimeData *data = new QMimeData();
        if (data) {
            data->setData(AbstractGroup::mimeType(), m_id.toAscii());
            if (!data->formats().isEmpty()) {
                QDrag *drag = new QDrag(event->widget());
                QPixmap p = m_icon.pixmap(QSize(KIconLoader::SizeLarge, KIconLoader::SizeLarge));
                drag->setPixmap(p);

                drag->setMimeData(data);
                drag->exec();   // krazy:exclude=crashy

                setCursor(Qt::OpenHandCursor);
            } else {
                delete data;
            }
        }
    }
}

void GroupIcon::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
    if (isDraggable()) {
        setCursor(Qt::OpenHandCursor);
    }
}

void GroupIcon::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
    if (isDraggable()) {
        setCursor(Qt::ClosedHandCursor);
    }
}

bool GroupIcon::isDraggable() const
{
    return cursor().shape() == Qt::OpenHandCursor ||
           cursor().shape() == Qt::ClosedHandCursor;
}

void GroupIcon::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    m_background->paintFrame(painter, option->rect, option->rect);
    const QRectF rect = contentsRect();
    const int width = rect.width();
    const int height = rect.height();

    QRectF textRect(rect.x(), rect.y(), width, height - m_iconHeight - 2);
    QRect iconRect(rect.x() + qMax(0, (width / 2) - (m_iconHeight / 2)), textRect.bottom() + 2, m_iconHeight, m_iconHeight);
    //QRectF textRect(rect.x(), iconRect.bottom() + 2, width, height - iconRect.height() - 2);

    painter->setPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
    painter->setFont(font());

    int flags = Qt::AlignTop;// | Qt::TextWordWrap;
    QFontMetrics fm(font());
    if (fm.width(m_name) < textRect.width()) {
        flags |= Qt::AlignCenter;
    }

    if (!m_name.isEmpty() && qGray(painter->pen().color().rgb()) < 192) {
        const QRectF haloRect = fm.boundingRect(textRect.toAlignedRect(), flags, m_name);
        Plasma::PaintUtils::drawHalo(painter, haloRect);
    }

    painter->drawPixmap(iconRect, m_icon.pixmap(QSize(m_iconHeight, m_iconHeight)));
    painter->drawText(textRect, flags, m_name);
}

#include "groupicon.moc"
