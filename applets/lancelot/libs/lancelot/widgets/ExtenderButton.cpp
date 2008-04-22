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

#include "ExtenderButton.h"

#include <QRectF>
#include <KDebug>
#include <KIcon>

#define ACTIVATION_TIME 300

namespace Lancelot {

// Inner class ExtenderObject
class ExtenderObject : public BasicWidget {
public:
    ExtenderObject(QString name, Plasma::Svg * icon,
            ExtenderButton * parent = 0)
      : BasicWidget(name, icon, "", "", parent),
        m_parent(parent)
    {
        setInnerOrientation(Qt::Vertical);
        setAlignment(Qt::AlignCenter);
    }

    void hoverEnterEvent(QGraphicsSceneHoverEvent * event)
    {
        BasicWidget::hoverEnterEvent(event);
        startTimer();
    }

    void hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
    {
        BasicWidget::hoverLeaveEvent(event);
        stopTimer();
    }

    static QBasicTimer timer;

    void startTimer()
    {
        kDebug() << "Starting timer";
        timer.start(ACTIVATION_TIME, this);
    }

    void stopTimer()
    {
        kDebug() << "Stopping timer";
        timer.stop();
    }

    void timerEvent(QTimerEvent * event)
    {
        kDebug() << "timer fired";
        if (event->timerId() == timer.timerId()) {
            kDebug() << "and it is the right one";
            emit m_parent->activated();
        }
        QObject::timerEvent(event);
    }

private:
    ExtenderButton * m_parent;
};

// ExtenderButton
class ExtenderButton::Private {
public:
    Private(ExtenderButton * parent)
      : q(parent),
        extender(NULL),
        extenderPosition(NoExtender),
        activationMethod(ClickActivate)
    {
        if (!extenderIconSvg) {
            extenderIconSvg = new Plasma::Svg("lancelot/extender-button-icon");
            //extenderIconSvg->setContentType(Plasma::Svg::ImageSet);
            extenderIconSvg->setContainsMultipleImages(true);
        }

        extender = new ExtenderObject(q->name() + "::Extender", extenderIconSvg, q);
        extender->setVisible(false);

        extender->setIconSize(QSize(16, 16));
    }

    ~Private()
    {
        delete extender;
    }

    void relayoutExtender()
    {
        if (!extender) return;
        QRectF geometry = QRectF(QPointF(0, 0), QSizeF(w, h));  // q->geometry();
        kDebug() << "Parent geometry " << geometry;
        switch (extenderPosition) {
        case TopExtender:
            geometry.setHeight(EXTENDER_SIZE);
            geometry.moveTop(- EXTENDER_SIZE);
            break;
        case BottomExtender:
            geometry.moveTop(geometry.bottom());
            geometry.setHeight(EXTENDER_SIZE);
            break;
        case LeftExtender:
            geometry.setWidth(EXTENDER_SIZE);
            geometry.moveLeft(- EXTENDER_SIZE);
            break;
        case RightExtender:
            geometry.moveLeft(geometry.right());
            geometry.setWidth(EXTENDER_SIZE);
            break;
        case NoExtender:
            break;
        }
        extender->setGeometry(geometry);

        qreal left, top, right, bottom;
        q->getContentsMargins(&left, &top, &right, &bottom);
        kDebug() << left << " " << top << " " << right << " " << bottom;
        kDebug() << "Procd geometry " << geometry;
    }

    ExtenderButton * q;

    ExtenderObject * extender;
    ExtenderPosition extenderPosition;
    ActivationMethod activationMethod;

    static Plasma::Svg * extenderIconSvg;
    qreal w, h;
};

Plasma::Svg * ExtenderButton::Private::extenderIconSvg = NULL;
QBasicTimer ExtenderObject::timer = QBasicTimer();

ExtenderButton::ExtenderButton(QString name, QString title,
        QString description, QGraphicsItem * parent)
  : BasicWidget(name, title, description, parent),
    d(new Private(this))
{
    setGroupByName("ExtenderButton");
}

ExtenderButton::ExtenderButton(QString name, QIcon icon, QString title,
        QString description, QGraphicsItem * parent)
  : BasicWidget(name, icon, title, description, parent),
    d(new Private(this))
{
    setGroupByName("ExtenderButton");
}

ExtenderButton::ExtenderButton(QString name, Plasma::Svg * icon, QString title,
        QString description, QGraphicsItem * parent)
  : BasicWidget(name, icon, title, description, parent),
    d(new Private(this))
{
    setGroupByName("ExtenderButton");
}

void ExtenderButton::setGroup(WidgetGroup * g)
{
    BasicWidget::setGroup(g);
    d->extender->setGroupByName(
            group()->name() + "-Extender"
            );
}

void ExtenderButton::groupUpdated()
{
    BasicWidget::groupUpdated();
    if (group()->hasProperty("ExtenderPosition")) {
        setExtenderPosition((ExtenderPosition)(group()->property("ExtenderPosition").toInt()));
    }
}

ExtenderButton::~ExtenderButton()
{
    delete d;
}

QRectF ExtenderButton::boundingRect() const
{
    if (!isHovered() || d->extenderPosition == NoExtender)
        return QRectF(0, 0, size().width(), size().height());
    switch (d->extenderPosition) {
    case TopExtender:
        return QRectF(0, - EXTENDER_SIZE, size().width(), size().height() + EXTENDER_SIZE);
    case BottomExtender:
        return QRectF(0, 0, size().width(), size().height() + EXTENDER_SIZE);
    case LeftExtender:
        return QRectF(- EXTENDER_SIZE, 0, size().width() + EXTENDER_SIZE, size().height());
    case RightExtender:
        return QRectF(0, 0, size().width() + EXTENDER_SIZE, size().height());
    case NoExtender:
        return QRectF(0, 0, size().width(), size().height());
    }
    return QRectF();
}

void ExtenderButton::hoverEnterEvent(QGraphicsSceneHoverEvent * event)
{
    if (d->extenderPosition != NoExtender) {
        d->extender->setVisible(true);
    } else if (d->activationMethod == HoverActivate) {
        d->extender->startTimer();
    }
    BasicWidget::hoverEnterEvent(event);
}

void ExtenderButton::hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
{
    d->extender->setVisible(false);
    d->extender->stopTimer();
    BasicWidget::hoverLeaveEvent(event);
}

void ExtenderButton::setExtenderPosition(ExtenderPosition position)
{
    if (d->extenderPosition == position)
        return;
    d->extenderPosition = position;
    d->relayoutExtender();
}

ExtenderPosition ExtenderButton::extenderPosition()
{
    return d->extenderPosition;
}

void ExtenderButton::setActivationMethod(ActivationMethod method)
{
    if (d->activationMethod == method)
        return;
    d->activationMethod = method;
    if (d->activationMethod != ExtenderActivate)
        setExtenderPosition(NoExtender);
}

ActivationMethod ExtenderButton::activationMethod()
{
    return d->activationMethod;
}

void ExtenderButton::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    if (event->button() == Qt::LeftButton) {
        emit activated();
    } else {
        event->ignore();
    }
    BasicWidget::mousePressEvent(event);
}

void ExtenderButton::setGeometry(qreal x, qreal y, qreal w, qreal h)
{
    setGeometry(QRectF(x, y, w, h));
}

void ExtenderButton::setGeometry(const QRectF & rect)
{
    BasicWidget::setGeometry(rect);

    // TODO: Qt Bug returning wrong size() bypass
    d->w = rect.width();
    d->h = rect.height();
    d->relayoutExtender();
    kDebug() << rect;
}

// void ExtenderButton::updateGeometry()
// {
//     BasicWidget::updateGeometry();
//     d->relayoutExtender();
// }

} // namespace Lancelot

