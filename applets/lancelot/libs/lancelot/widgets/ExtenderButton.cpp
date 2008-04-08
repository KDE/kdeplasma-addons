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
class ExtenderObject : public BaseActionWidget {
public:
    ExtenderObject(QString name, Plasma::Svg * icon, ExtenderButton * parent = 0)
      : BaseActionWidget(name, icon, "", "", parent), m_parent(parent)
    {
        setInnerOrientation(Qt::Vertical);
        setAlignment(Qt::AlignCenter);
    }

    void hoverEnterEvent(QGraphicsSceneHoverEvent * event)
    {
        BaseActionWidget::hoverEnterEvent(event);
        startTimer();
    }

    void hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
    {
        BaseActionWidget::hoverLeaveEvent(event);
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
      : q(parent), extender(NULL), extenderPosition(NoExtender), activationMethod(ClickActivate)
    {
        if (!extenderIconSvg) {
            extenderIconSvg = new Plasma::Svg("lancelot/extender-button-icon");
            extenderIconSvg->setContentType(Plasma::Svg::ImageSet);
        }

        extender = new ExtenderObject(q->name() + "::Extender", extenderIconSvg, q);
        extender->setVisible(false);

        extender->setIconSize(QSize(16, 16));

        //connect(extender, SIGNAL(mouseHoverEnter()), this, SLOT(startTimer()));
        //connect(extender, SIGNAL(mouseHoverLeave()), this, SLOT(stopTimer()));
    }

    ~Private()
    {
        delete extender;
    }

    void relayoutExtender()
    {
        if (!extender) return;
        switch (extenderPosition) {
        case TopExtender:
            extender->setPos(0, - EXTENDER_SIZE);
            extender->resize(q->size().width(), EXTENDER_SIZE);
            break;
        case BottomExtender:
            extender->setPos(0, q->size().height());
            extender->resize(q->size().width(), EXTENDER_SIZE);
            break;
        case LeftExtender:
            extender->setPos(- EXTENDER_SIZE, 0);
            extender->resize(EXTENDER_SIZE, q->size().height());
            break;
        case RightExtender:
            extender->setPos(q->size().width(), 0);
            extender->resize(EXTENDER_SIZE, q->size().height());
            break;
        case NoExtender:
            break;
        }
    }

    ExtenderButton * q;

    ExtenderObject * extender;
    ExtenderPosition extenderPosition;
    ActivationMethod activationMethod;

    static Plasma::Svg * extenderIconSvg;
};

Plasma::Svg * ExtenderButton::Private::extenderIconSvg = NULL;
QBasicTimer ExtenderObject::timer = QBasicTimer();

//ExtenderButton::Private::ExtenderButtonTimer * ExtenderButton::Private::ExtenderButtonTimer::m_instance = NULL;

ExtenderButton::ExtenderButton(QString name, QString title, QString description,
        QGraphicsItem * parent) :
            BaseActionWidget(name, title, description, parent), d(new Private(this))
{
    setGroupByName("ExtenderButton");
}

ExtenderButton::ExtenderButton(QString name, QIcon * icon, QString title,
        QString description, QGraphicsItem * parent) :
            BaseActionWidget(name, icon, title, description, parent), d(new Private(this))
{
    setGroupByName("ExtenderButton");
}

ExtenderButton::ExtenderButton(QString name, Plasma::Svg * icon, QString title,
        QString description, QGraphicsItem * parent) :
            BaseActionWidget(name, icon, title, description, parent), d(new Private(this))
{
    setGroupByName("ExtenderButton");
}

void ExtenderButton::setGroup(WidgetGroup * g)
{
    Widget::setGroup(g);
    d->extender->setGroupByName(
            group()->name() + "-Extender"
            );
}

void ExtenderButton::groupUpdated()
{
    BaseActionWidget::groupUpdated();
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
    BaseActionWidget::hoverEnterEvent(event);
}

void ExtenderButton::hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
{
    d->extender->setVisible(false);
    d->extender->stopTimer();
    BaseActionWidget::hoverLeaveEvent(event);
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
    BaseActionWidget::mousePressEvent(event);
}

void ExtenderButton::setGeometry(const QRectF & geometry)
{
    BaseActionWidget::setGeometry(geometry);
    d->relayoutExtender();
}

} // namespace Lancelot

