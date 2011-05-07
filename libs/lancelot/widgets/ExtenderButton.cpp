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

#include "ExtenderButton.h"

#include <QAction>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QIcon>
#include <QRectF>
#include <QTimer>
#include <QPointer>

#include <KDebug>

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
        iconInSvg().setUsingRenderingCache(true);

        iconInSvg().setContainsMultipleImages(true);

        frameCount = 0;
        while (iconInSvg().hasElement("frame" + QString::number(frameCount))) {
            frameCount++;
        }
        frameCount--;

        setParentItem(parent);
        setInnerOrientation(Qt::Vertical);
        setAlignment(Qt::AlignCenter);
        setZValue(EXTENDER_Z_VALUE);

        animate = !Global::self()->config("Animation", "disableAnimations", false)
                && Global::self()->config("Animation", "extenderActivationFeedback", true);
    }

    L_Override void hoverEnterEvent(QGraphicsSceneHoverEvent * event)
    {
        timer.start(ACTIVATION_TIME / frameCount, this);

        BasicWidget::hoverEnterEvent(event);
    }

    L_Override void hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
    {
        timer.stop();
        frame = 0;

        BasicWidget::hoverLeaveEvent(event);
    }

    void paint(QPainter * painter,
            const QStyleOptionGraphicsItem * option, QWidget * widget)
    {
        Q_UNUSED(option);
        Q_UNUSED(widget);

        // BasicWidget::paint(painter, option, widget);
        if (iconInSvg().isValid()) {
            QRectF iconRect = QRectF(QPointF(), iconSize());
            QSizeF sizeDiff = size() - iconRect.size();

            iconRect.setTopLeft(QPointF(sizeDiff.width() / 2, sizeDiff.height() / 2));
            if (animate) {
                iconInSvg().paint(painter,
                    iconRect.left(),
                    iconRect.top(),
                    "frame" + QString::number(frame));
            } else {
                iconInSvg().paint(painter,
                    iconRect.left(),
                    iconRect.top(),
                    "frame" + QString::number(
                        (frame > 0) ? frameCount : 0
                    ));

            }
        }
    }

    L_Override void showEvent(QShowEvent * event)
    {
        frame = 0;
    }

    L_Override void timerEvent(QTimerEvent * event)
    {
        if (event->timerId() == timer.timerId()) {
            frame ++;

            if (frame > frameCount) {
                timer.stop();
                frame = 0;

                emit clicked();

            } else {
                if (animate || frame == 1) {
                    update();
                }
            }
        }

        BasicWidget::timerEvent(event);
    }

    int frameCount;
    bool animate;

public:
    // Plasma::FrameSvg::EnabledBorders borders;

    QBasicTimer timer;
    int frame;

    friend class ExtenderButton;
};

// ExtenderButton
class ExtenderButton::Private {
public:
    Private(ExtenderButton * parent)
      : q(parent),
        shortcut(NULL),
        extenderPosition(NoExtender),
        activationMethod(ClickActivate),
        checkable(false),
        checked(false)
    {
        if (!extenderIconSvg.isValid()) {
            extenderIconSvg.setImagePath("lancelot/extender-button-icon");
        }

        if (!timer) {
            timer = new QTimer();
            timer->setInterval(ACTIVATION_TIME);
            timer->setSingleShot(true);
        }

    }

    void releaseExtender() {
        ExtenderObject * ext = extender(false);
        if (ext) {
            ext->setVisible(false);
            ext->hoverLeaveEvent(NULL);
            ext->setParentItem(NULL);
        }

        disconnectTimer();
    }

    void disconnectTimer() {
        timer->stop();
        disconnect(timer, SIGNAL(timeout()), 0, 0);
    }

    void connectTimer() {
        disconnectTimer();

        connect(
                timer, SIGNAL(timeout()),
                q, SLOT(activate())
               );
    }

    ExtenderObject * extender(bool take = true) {
        ExtenderObject * result = 0;

        if (!m_extenders.contains(q->scene()) ||
                !m_extenders[q->scene()]) {
            result = new ExtenderObject(extenderIconSvg, NULL);

            result->setVisible(false);
            result->setIconSize(QSize(16, 16));

            m_extenders[q->scene()] = result;
        } else {
            result = m_extenders[q->scene()];
        }

        if (result->parentItem() != q) {
            if (!take) {
                return NULL;
            }

            result->setParentItem(q);
            result->setGroupByName(
                    q->group()->name() + "-Extender"
                    );

            connectTimer();

            disconnect(result, SIGNAL(clicked()), 0, 0);
            connect(result, SIGNAL(clicked()),
                    q, SLOT(activate()));
            relayoutExtender();
        }

        return result;
    }

    ~Private()
    {
    }

    void relayoutExtender()
    {
        if (!m_extenders.contains(q->scene())
            || !m_extenders[q->scene()]
            || q != m_extenders[q->scene()]->parentItem()) return;
        QRectF geometry = QRectF(QPointF(0, 0), q->size());

        // extender()->borders = Plasma::FrameSvg::AllBorders;
        borders = Plasma::FrameSvg::AllBorders;

        switch (extenderPosition) {
        case TopExtender:
            geometry.setHeight(EXTENDER_SIZE);
            geometry.moveTop(- EXTENDER_SIZE);
            // borders &= ~ Plasma::FrameSvg::TopBorder;
            // extender()->borders &= ~ Plasma::FrameSvg::BottomBorder;
            break;
        case BottomExtender:
            geometry.moveTop(geometry.bottom());
            geometry.setHeight(EXTENDER_SIZE);
            // borders &= ~ Plasma::FrameSvg::BottomBorder;
            // extender()->borders &= ~ Plasma::FrameSvg::TopBorder;
            break;
        case LeftExtender:
            geometry.setWidth(EXTENDER_SIZE);
            geometry.moveLeft(- EXTENDER_SIZE);
            // borders &= ~ Plasma::FrameSvg::LeftBorder;
            // extender()->borders &= ~ Plasma::FrameSvg::RightBorder;
            break;
        case RightExtender:
            geometry.moveLeft(geometry.right());
            geometry.setWidth(EXTENDER_SIZE);
            // borders &= ~ Plasma::FrameSvg::RightBorder;
            // extender()->borders &= ~ Plasma::FrameSvg::LeftBorder;
            break;
        case NoExtender:
            break;
        }
        extender()->setGeometry(geometry);
        extender()->setPreferredSize(geometry.size());

        // qreal left, top, right, bottom;
        // q->getContentsMargins(&left, &top, &right, &bottom);
    }

    ExtenderButton * q;
    QAction * shortcut;

    ExtenderPosition extenderPosition;
    ActivationMethod activationMethod;

    Plasma::FrameSvg::EnabledBorders borders;

    static Plasma::Svg extenderIconSvg;
    static QHash < QGraphicsScene *, QPointer < ExtenderObject > > m_extenders;
    static QTimer * timer;

    bool checkable : 1;
    bool checked : 1;

};

Plasma::Svg ExtenderButton::Private::extenderIconSvg;
QHash < QGraphicsScene *, QPointer < ExtenderObject > > ExtenderButton::Private::m_extenders;
QTimer * ExtenderButton::Private::timer = 0;

ExtenderButton::ExtenderButton(QGraphicsItem * parent)
  : BasicWidget(parent),
    d(new Private(this))
{
    setGroupByName("ExtenderButton");
    connect(this, SIGNAL(clicked()), this, SLOT(activate()));
}

ExtenderButton::ExtenderButton(QString title,
        QString description, QGraphicsItem * parent)
  : BasicWidget(title, description, parent),
    d(new Private(this))
{
    setGroupByName("ExtenderButton");
    connect(this, SIGNAL(clicked()), this, SLOT(activate()));

    setTitle(title);
}

ExtenderButton::ExtenderButton(QIcon icon, QString title,
        QString description, QGraphicsItem * parent)
  : BasicWidget(icon, title, description, parent),
    d(new Private(this))
{
    setGroupByName("ExtenderButton");
    connect(this, SIGNAL(clicked()), this, SLOT(activate()));

    setTitle(title);
}

ExtenderButton::ExtenderButton(const Plasma::Svg & icon, QString title,
        QString description, QGraphicsItem * parent)
  : BasicWidget(icon, title, description, parent),
    d(new Private(this))
{
    setGroupByName("ExtenderButton");
    connect(this, SIGNAL(clicked()), this, SLOT(activate()));

    setTitle(title);
}

void ExtenderButton::setGroup(Group * g)
{
    BasicWidget::setGroup(g);
}

ExtenderButton::~ExtenderButton()
{
    if (Private::m_extenders.contains(scene())
        && Private::m_extenders[scene()]->parentItem() == this) {
        Private::m_extenders[scene()]->setParentItem(0);
    }

    delete d;
}

QRectF ExtenderButton::boundingRect() const
{
    return QRectF(0, 0, size().width(), size().height());
}

void ExtenderButton::hoverEnterEvent(QGraphicsSceneHoverEvent * event)
{
    if (d->extenderPosition != NoExtender) {
        d->extender()->setVisible(true);
    } else if (d->activationMethod == HoverActivate) {
        d->connectTimer();
        d->timer->start();
    }
    BasicWidget::hoverEnterEvent(event);
}

void ExtenderButton::hideEvent(QHideEvent * event)
{
    BasicWidget::hideEvent(event);
    d->releaseExtender();
}

void ExtenderButton::hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
{
    BasicWidget::hoverLeaveEvent(event);
    d->releaseExtender();
}

void ExtenderButton::setExtenderPosition(int position)
{
    if (d->extenderPosition == position)
        return;
    d->extenderPosition = (ExtenderPosition) position;
    d->relayoutExtender();
}

int ExtenderButton::extenderPosition() const
{
    return d->extenderPosition;
}

void ExtenderButton::setActivationMethod(int method)
{
    if (d->activationMethod == method)
        return;
    d->activationMethod = (ActivationMethod) method;
    if (d->activationMethod != ExtenderActivate)
        setExtenderPosition(NoExtender);
}

int ExtenderButton::activationMethod() const
{
    return d->activationMethod;
}

void ExtenderButton::activate()
{
    toggle();

    hoverLeaveEvent(0);
    d->releaseExtender();

    update();

    emit activated();
}

void ExtenderButton::resizeEvent(QGraphicsSceneResizeEvent * event)
{
    BasicWidget::resizeEvent(event);
    d->relayoutExtender();
}

void ExtenderButton::paint(QPainter * painter,
        const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // if (Plasma::FrameSvg * svg = group()->backgroundSvg()) {
    //     if (isHovered() && (d->extenderPosition != NoExtender)) {
    //         svg->setEnabledBorders(d->borders);
    //     } else {
    //         svg->setEnabledBorders(Plasma::FrameSvg::AllBorders);
    //     }
    // }

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
    kDebug() << title() << " set the key:" << key;

    if (key.isEmpty()) {
        delete d->shortcut;
        d->shortcut = NULL;
    } else if (scene() && scene()->views().size()) {
        if (!d->shortcut) {
            d->shortcut = new QAction(this);
            scene()->views().at(0)->addAction(d->shortcut);
        }

        d->shortcut->setShortcut(QString(QLatin1String("Alt+")+ key));
        connect(d->shortcut, SIGNAL(triggered()),
                this, SIGNAL(activated()));
    }
}

} // namespace Lancelot

