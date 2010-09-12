/*
 *   Copyright (C) 2009 by Ana Cecília Martins <anaceciliamb@gmail.com>
 *   Copyright (C) 2010 by Chani Armitage <chani@kde.org>
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

/**
 * Taken from kdebase/workspace/libs/plasmagenericshell
 */

#include "abstracticon.h"

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

namespace Plasma
{

AbstractIcon::AbstractIcon(QGraphicsItem *parent)
    : QGraphicsWidget(parent),
      m_iconHeight(DEFAULT_ICON_SIZE),
      m_maxSize(maximumSize()),
      m_selected(false),
      m_hovered(false)
{
    setCacheMode(DeviceCoordinateCache);
    setAcceptHoverEvents(true);
    m_background = new Plasma::FrameSvg(this);
    m_background->setImagePath("widgets/background");
}

AbstractIcon::~AbstractIcon()
{
}

void AbstractIcon::resizeEvent(QGraphicsSceneResizeEvent *)
{
    m_background->resizeFrame(size());
    qreal l, t, r, b;
    m_background->getMargins(l, t, r, b);
    setContentsMargins(l, t, r, b);
}

QSizeF AbstractIcon::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
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

void AbstractIcon::setIconSize(int height)
{
    m_iconHeight = height;
    updateGeometry();
    update();
}

int AbstractIcon::iconSize() const
{
    return m_iconHeight;
}

void AbstractIcon::setName(const QString &name)
{
    m_name = name;
    updateGeometry();
    update();
}

QString AbstractIcon::name() const
{
    return m_name;
}

void AbstractIcon::collapse()
{
    if (isVisible()) {
        setVisible(false);
        m_maxSize = maximumSize();
        //kDebug() << m_maxSize;
        setMaximumSize(0, 0);
    }
}

void AbstractIcon::expand()
{
    if (!isVisible()) {
        setVisible(true);
        setMaximumSize(m_maxSize);
    }
}

void AbstractIcon::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
    m_hovered = true;
    emit hoverEnter(this);
    QMimeData *data = mimeData();
    if (data && !data->formats().isEmpty()) {
    }
}

void AbstractIcon::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    m_hovered = false;
    emit hoverLeave(this);
}

void AbstractIcon::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton &&
        (event->pos() - event->buttonDownPos(Qt::LeftButton)).toPoint().manhattanLength() > QApplication::startDragDistance()) {
        event->accept();
        QMimeData *data = mimeData();
        if (data && !data->formats().isEmpty()) {
            qDebug() << "Start Dragging";
            emit dragging(this);
            QDrag *drag = new QDrag(event->widget());
            QPixmap p = pixmap(QSize(KIconLoader::SizeLarge, KIconLoader::SizeLarge));
            drag->setPixmap(p);

            drag->setMimeData(mimeData());
            drag->exec();

            setCursor(Qt::OpenHandCursor);
        } else {
            delete data;
        }
    }
}

void AbstractIcon::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsWidget::mouseReleaseEvent(event);
    if (isDraggable()) {
        setCursor(Qt::OpenHandCursor);
    }

    if (boundingRect().contains(event->pos())) {
        emit(clicked(this));
    }
}

void AbstractIcon::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
    if (isDraggable()) {
        setCursor(Qt::ClosedHandCursor);
    }
}

void AbstractIcon::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
    emit doubleClicked(this);
}

void AbstractIcon::setSelected(bool selected)
{
    m_selected = selected;
}

bool AbstractIcon::isSelected() const
{
    return m_selected;
}

bool AbstractIcon::isDraggable() const
{
    return cursor().shape() == Qt::OpenHandCursor ||
           cursor().shape() == Qt::ClosedHandCursor;
}

void AbstractIcon::setDraggable(bool draggable)
{
    if (draggable) {
        setCursor(Qt::OpenHandCursor);
    } else {
        unsetCursor();
    }
}

void AbstractIcon::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
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
        PaintUtils::drawHalo(painter, haloRect);
    }

    painter->drawPixmap(iconRect, pixmap(QSize(m_iconHeight, m_iconHeight)));
    painter->drawText(textRect, flags, m_name);
}

} // namespace Plasma


