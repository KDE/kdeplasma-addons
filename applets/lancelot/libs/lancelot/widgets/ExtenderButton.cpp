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
    ExtenderObject(Plasma::Svg * icon,
            ExtenderButton * parent = 0)
      : BasicWidget(icon, "", "", parent),
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
        timer.start(ACTIVATION_TIME, this);
    }

    void stopTimer()
    {
        timer.stop();
    }

    void timerEvent(QTimerEvent * event)
    {
        if (event->timerId() == timer.timerId()) {
            stopTimer();
            emit m_parent->activated();
            m_parent->toggle();
            hide();
            // Qt bug... - element is hidden but doesn't receive hoverLeaveEvent
            hoverLeaveEvent(0);
            m_parent->hoverLeaveEvent(0);
        }
        QObject::timerEvent(event);
    }

    void paint(QPainter * painter,
            const QStyleOptionGraphicsItem * option, QWidget * widget)
    {
        if (Plasma::PanelSvg * svg = group()->backgroundSvg()) {
            svg->setEnabledBorders(borders);
        }
        BasicWidget::paint(painter, option, widget);
    }

    Plasma::PanelSvg::EnabledBorders borders;

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
        activationMethod(ClickActivate),
        down(false),
        checkable(false),
        checked(false)
    {
        if (!extenderIconSvg) {
            extenderIconSvg = new Plasma::Svg(q);
            extenderIconSvg->setImagePath("lancelot/extender-button-icon");
        }

        extender = new ExtenderObject(extenderIconSvg, q);
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
        QRectF geometry = QRectF(QPointF(0, 0), q->size());

        extender->borders = Plasma::PanelSvg::AllBorders;
        borders = Plasma::PanelSvg::AllBorders;

        switch (extenderPosition) {
        case TopExtender:
            geometry.setHeight(EXTENDER_SIZE);
            geometry.moveTop(- EXTENDER_SIZE);
            borders &= ~ Plasma::PanelSvg::TopBorder;
            extender->borders &= ~ Plasma::PanelSvg::BottomBorder;
            break;
        case BottomExtender:
            geometry.moveTop(geometry.bottom());
            geometry.setHeight(EXTENDER_SIZE);
            borders &= ~ Plasma::PanelSvg::BottomBorder;
            extender->borders &= ~ Plasma::PanelSvg::TopBorder;
            break;
        case LeftExtender:
            geometry.setWidth(EXTENDER_SIZE);
            geometry.moveLeft(- EXTENDER_SIZE);
            borders &= ~ Plasma::PanelSvg::LeftBorder;
            extender->borders &= ~ Plasma::PanelSvg::RightBorder;
            break;
        case RightExtender:
            geometry.moveLeft(geometry.right());
            geometry.setWidth(EXTENDER_SIZE);
            borders &= ~ Plasma::PanelSvg::RightBorder;
            extender->borders &= ~ Plasma::PanelSvg::LeftBorder;
            break;
        case NoExtender:
            break;
        }
        extender->setGeometry(geometry);

        qreal left, top, right, bottom;
        q->getContentsMargins(&left, &top, &right, &bottom);
    }

    ExtenderButton * q;

    ExtenderObject * extender;
    ExtenderPosition extenderPosition;
    ActivationMethod activationMethod;

    static Plasma::Svg * extenderIconSvg;
    Plasma::PanelSvg::EnabledBorders borders;

    bool down : 1;
    bool checkable : 1;
    bool checked : 1;

};

Plasma::Svg * ExtenderButton::Private::extenderIconSvg = NULL;
QBasicTimer ExtenderObject::timer = QBasicTimer();

ExtenderButton::ExtenderButton(QGraphicsItem * parent)
  : BasicWidget(parent),
    d(new Private(this))
{
    setGroupByName("ExtenderButton");
    L_WIDGET_SET_INITIALIZED;
}

ExtenderButton::ExtenderButton(QString title,
        QString description, QGraphicsItem * parent)
  : BasicWidget(title, description, parent),
    d(new Private(this))
{
    setGroupByName("ExtenderButton");
    L_WIDGET_SET_INITIALIZED;
}

ExtenderButton::ExtenderButton(QIcon icon, QString title,
        QString description, QGraphicsItem * parent)
  : BasicWidget(icon, title, description, parent),
    d(new Private(this))
{
    setGroupByName("ExtenderButton");
    L_WIDGET_SET_INITIALIZED;
}

ExtenderButton::ExtenderButton(Plasma::Svg * icon, QString title,
        QString description, QGraphicsItem * parent)
  : BasicWidget(icon, title, description, parent),
    d(new Private(this))
{
    setGroupByName("ExtenderButton");
    L_WIDGET_SET_INITIALIZED;
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
    if (group()->hasProperty("ActivationMethod")) {
        setActivationMethod((ActivationMethod)(group()->property("ActivationMethod").toInt()));
    }
}

ExtenderButton::~ExtenderButton()
{
    delete d;
}

QRectF ExtenderButton::boundingRect() const
{
    return QRectF(0, 0, size().width(), size().height());
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
        d->down = true;
        update();
        event->accept();
        emit pressed();
    } else {
        BasicWidget::mousePressEvent(event);
    }
}

void ExtenderButton::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
    if (d->down && (event->button() == Qt::LeftButton)) {
        d->down = false;
        emit activated();
        toggle();
        update();
        emit clicked();
        emit released();
    } else {
        BasicWidget::mouseReleaseEvent(event);
    }
}

void ExtenderButton::geometryUpdated()
{
    BasicWidget::geometryUpdated();
    d->relayoutExtender();
}

void ExtenderButton::paint(QPainter * painter,
        const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (Plasma::PanelSvg * svg = group()->backgroundSvg()) {
        if (isHovered() && (d->extenderPosition != NoExtender)) {
            svg->setEnabledBorders(d->borders);
        } else {
            svg->setEnabledBorders(Plasma::PanelSvg::AllBorders);
        }
    }

    if (d->down) {
        paintBackground(painter, "down");
    } else if (d->checked) {
        paintBackground(painter, "checked");
    } else {
        paintBackground(painter);
    }
    paintForeground(painter);
}

void ExtenderButton::setChecked(bool checked)
{
    if (!d->checkable) return;
    d->checked = checked;
    update();
    emit toggled(d->checked);
}

bool ExtenderButton::isChecked()
{
    return d->checked;
}

void ExtenderButton::toggle()
{
    if (!d->checkable) return;
    d->checked = !d->checked;
    update();
    emit toggled(d->checked);
}

void ExtenderButton::setCheckable(bool checkable)
{
    d->checkable = checkable;
    if (!checkable) {
        d->checked = false;
    }
    update();
}

bool ExtenderButton::isCheckable()
{
    return d->checkable;
}

void ExtenderButton::setDown(bool down)
{
    d->down = down;
    update();
}

bool ExtenderButton::isDown()
{
    return d->down;
}

} // namespace Lancelot

