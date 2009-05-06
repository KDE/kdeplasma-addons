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
#include <QIcon>
#include <QTimer>
#include <QAction>
#include <QGraphicsScene>
#include <QGraphicsView>

#define ACTIVATION_TIME 300
#define EXTENDER_Z_VALUE 100000.0

namespace Lancelot {

// Inner class ExtenderObject
class ExtenderObject : public BasicWidget {
public:
    ExtenderObject(const Plasma::Svg & icon,
            ExtenderButton * parent)
      : BasicWidget(icon, "", "")
    {
        setParentItem(parent);
        setInnerOrientation(Qt::Vertical);
        setAlignment(Qt::AlignCenter);
        setZValue(EXTENDER_Z_VALUE);
    }

    // Needed because of a Qt bug - making it public
    // so that ExtenderButton can invoke it directly
    void hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
    {
        BasicWidget::hoverLeaveEvent(event);
    }

    void paint(QPainter * painter,
            const QStyleOptionGraphicsItem * option, QWidget * widget)
    {
        Q_UNUSED(option);
        Q_UNUSED(widget);

        if (Plasma::FrameSvg * svg = group()->backgroundSvg()) {
            svg->setEnabledBorders(borders);
        }

        BasicWidget::paint(painter, option, widget);
    }

public:
    Plasma::FrameSvg::EnabledBorders borders;

};

// ExtenderButton
class ExtenderButton::Private {
public:
    Private(ExtenderButton * parent)
      : q(parent),
        shortcut(NULL),
        extenderPosition(NoExtender),
        activationMethod(ClickActivate),
        extender(NULL),
        checkable(false),
        checked(false)
    {
        if (!extenderIconSvg.isValid()) {
            extenderIconSvg.setImagePath("lancelot/extender-button-icon");
        }

        extender = new ExtenderObject(extenderIconSvg, parent);
        extender->setVisible(false);

        extender->setIconSize(QSize(16, 16));

        timer.setInterval(ACTIVATION_TIME);
        timer.setSingleShot(true);
        connect(
                extender, SIGNAL(mouseHoverEnter()),
                &timer, SLOT(start())
               );
        connect(
                extender, SIGNAL(mouseHoverLeave()),
                &timer, SLOT(stop())
               );
        connect(
                &timer, SIGNAL(timeout()),
                parent, SLOT(activate())
               );

    }

    ~Private()
    {
        delete extender;
    }

    void relayoutExtender()
    {
        if (!extender) return;
        QRectF geometry = QRectF(QPointF(0, 0), q->size());

        extender->borders = Plasma::FrameSvg::AllBorders;
        borders = Plasma::FrameSvg::AllBorders;

        switch (extenderPosition) {
        case TopExtender:
            geometry.setHeight(EXTENDER_SIZE);
            geometry.moveTop(- EXTENDER_SIZE);
            borders &= ~ Plasma::FrameSvg::TopBorder;
            extender->borders &= ~ Plasma::FrameSvg::BottomBorder;
            break;
        case BottomExtender:
            geometry.moveTop(geometry.bottom());
            geometry.setHeight(EXTENDER_SIZE);
            borders &= ~ Plasma::FrameSvg::BottomBorder;
            extender->borders &= ~ Plasma::FrameSvg::TopBorder;
            break;
        case LeftExtender:
            geometry.setWidth(EXTENDER_SIZE);
            geometry.moveLeft(- EXTENDER_SIZE);
            borders &= ~ Plasma::FrameSvg::LeftBorder;
            extender->borders &= ~ Plasma::FrameSvg::RightBorder;
            break;
        case RightExtender:
            geometry.moveLeft(geometry.right());
            geometry.setWidth(EXTENDER_SIZE);
            borders &= ~ Plasma::FrameSvg::RightBorder;
            extender->borders &= ~ Plasma::FrameSvg::LeftBorder;
            break;
        case NoExtender:
            break;
        }
        extender->setGeometry(geometry);
        extender->setPreferredSize(geometry.size());

        // qreal left, top, right, bottom;
        // q->getContentsMargins(&left, &top, &right, &bottom);
    }

    ExtenderButton * q;
    QAction * shortcut;

    ExtenderPosition extenderPosition;
    ActivationMethod activationMethod;

    static Plasma::Svg extenderIconSvg;
    Plasma::FrameSvg::EnabledBorders borders;

    ExtenderObject * extender;
    QTimer timer;
    bool checkable : 1;
    bool checked : 1;

};

Plasma::Svg ExtenderButton::Private::extenderIconSvg;

ExtenderButton::ExtenderButton(QGraphicsItem * parent)
  : BasicWidget(parent),
    d(new Private(this))
{
    setGroupByName("ExtenderButton");
    connect(this, SIGNAL(clicked()), this, SLOT(activate()));
    L_WIDGET_SET_INITIALIZED;
}

ExtenderButton::ExtenderButton(QString title,
        QString description, QGraphicsItem * parent)
  : BasicWidget(title, description, parent),
    d(new Private(this))
{
    setGroupByName("ExtenderButton");
    connect(this, SIGNAL(clicked()), this, SLOT(activate()));
    L_WIDGET_SET_INITIALIZED;
}

ExtenderButton::ExtenderButton(QIcon icon, QString title,
        QString description, QGraphicsItem * parent)
  : BasicWidget(icon, title, description, parent),
    d(new Private(this))
{
    setGroupByName("ExtenderButton");
    connect(this, SIGNAL(clicked()), this, SLOT(activate()));
    L_WIDGET_SET_INITIALIZED;
}

ExtenderButton::ExtenderButton(const Plasma::Svg & icon, QString title,
        QString description, QGraphicsItem * parent)
  : BasicWidget(icon, title, description, parent),
    d(new Private(this))
{
    setGroupByName("ExtenderButton");
    connect(this, SIGNAL(clicked()), this, SLOT(activate()));
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
        d->timer.start();
    }
    BasicWidget::hoverEnterEvent(event);
}

void ExtenderButton::hideEvent(QHideEvent * event)
{
    d->extender->setVisible(false);
    d->timer.stop();
    d->extender->hoverLeaveEvent(NULL);
    BasicWidget::hideEvent(event);
}

void ExtenderButton::hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
{
    d->extender->setVisible(false);
    d->timer.stop();
    d->extender->hoverLeaveEvent(event);
    BasicWidget::hoverLeaveEvent(event);
}

void ExtenderButton::setExtenderPosition(ExtenderPosition position)
{
    if (d->extenderPosition == position)
        return;
    d->extenderPosition = position;
    d->relayoutExtender();
}

ExtenderPosition ExtenderButton::extenderPosition() const
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

ActivationMethod ExtenderButton::activationMethod() const
{
    return d->activationMethod;
}

void ExtenderButton::activate()
{
    toggle();
    d->timer.stop();
    d->extender->hide();
    // Qt bug... - element is hidden but doesn't receive hoverLeaveEvent
    hoverLeaveEvent(0);
    //d->extender->hoverLeaveEvent(0);
    update();
    emit activated();
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

    if (Plasma::FrameSvg * svg = group()->backgroundSvg()) {
        if (isHovered() && (d->extenderPosition != NoExtender)) {
            svg->setEnabledBorders(d->borders);
        } else {
            svg->setEnabledBorders(Plasma::FrameSvg::AllBorders);
        }
    }

    if (d->checked && !isDown()) {
        paintBackground(painter, "checked");
    } else {
        paintBackground(painter);
    }
    paintForeground(painter);
}

void ExtenderButton::setChecked(bool checked)
{
    if (!d->checkable) return;
    if (d->checked == checked) return;
    d->checked = checked;
    update();
    emit toggled(d->checked);
}

bool ExtenderButton::isChecked() const
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
    if (d->checkable == checkable) return;
    d->checkable = checkable;
    if (!checkable) {
        d->checked = false;
    }
    update();
}

bool ExtenderButton::isCheckable() const
{
    return d->checkable;
}

void ExtenderButton::setShortcutKey(const QString & key)
{
    if (key.isEmpty()) {
        delete d->shortcut;
        d->shortcut = NULL;
    } else if (scene() && scene()->views().size()) {
        if (!d->shortcut) {
            d->shortcut = new QAction(this);
            scene()->views().at(0)->addAction(d->shortcut);
        }

        d->shortcut->setShortcut("Alt+" + key);
        connect(d->shortcut, SIGNAL(triggered()),
                this, SIGNAL(activated()));
    }
}

} // namespace Lancelot

