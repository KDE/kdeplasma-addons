/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
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

Plasma::Svg * ExtenderButton::m_extenderIconSvg = NULL;
/*Plasma::Svg * ExtenderButton::m_extenderButtonSvg = NULL;
int ExtenderButton::m_extendersCount = 0;*/

ExtenderButtonTimer * ExtenderButtonTimer::instance() {
    if (!m_instance) {
        m_instance = new ExtenderButtonTimer();
        m_instance->m_timer.setInterval(ACTIVATION_TIME);
        connect(&(m_instance->m_timer), SIGNAL(timeout()),
                m_instance, SLOT(fire()));
    }
    return m_instance;
}

void ExtenderButtonTimer::startTimer(ExtenderButton * owner) {
    if (!owner) return;
    m_owner = owner;
    m_timer.start();
}

void ExtenderButtonTimer::stopTimer() {
    m_timer.stop();
}

void ExtenderButtonTimer::fire() {
    m_owner->timerFired();
}

ExtenderButtonTimer::ExtenderButtonTimer() : QObject() {}
ExtenderButtonTimer::~ExtenderButtonTimer() {}


ExtenderButtonTimer * ExtenderButtonTimer::m_instance = NULL;

ExtenderButton::ExtenderButton(QString name, QString title, QString description,
        QGraphicsItem * parent) :
            BaseActionWidget(name, title, description, parent), m_extender(NULL),
            m_extenderPosition(No), m_activationMethod(Click)
{
    init();
}

ExtenderButton::ExtenderButton(QString name, QIcon * icon, QString title,
        QString description, QGraphicsItem * parent) :
            BaseActionWidget(name, icon, title, description, parent), m_extender(NULL),
            m_extenderPosition(No), m_activationMethod(Click)
{
    init();
}

ExtenderButton::ExtenderButton(QString name, Plasma::Svg * icon, QString title,
        QString description, QGraphicsItem * parent) :
            BaseActionWidget(name, icon, title, description, parent), m_extender(NULL),
            m_extenderPosition(No), m_activationMethod(Click)
{
    init();
}

void ExtenderButton::init()
{
    if (!m_extenderIconSvg) {
        m_extenderIconSvg = new Plasma::Svg("lancelot/extender-button-icon");
        m_extenderIconSvg->setContentType(Plasma::Svg::ImageSet);
    }

    m_extender = new ExtenderObject(name() + "::Extender", m_extenderIconSvg, this);
    m_extender->setVisible(false);
    
    Widget::setGroup("ExtenderButton");
    
    m_extender->m_iconSize = QSize(16, 16);

    //connect(m_extender, SIGNAL(mouseHoverEnter()), this, SIGNAL(activated()));
    connect(m_extender, SIGNAL(mouseHoverEnter()), this, SLOT(startTimer()));
    connect(m_extender, SIGNAL(mouseHoverLeave()), this, SLOT(stopTimer()));
    
    

}

void ExtenderButton::setGroup(WidgetGroup * group)
{
    Widget::setGroup(group);
    m_extender->setGroup(m_group->name() + "-Extender");
}

ExtenderButton::~ExtenderButton()
{
    /*if (--m_extendersCount == 0) {
        delete m_extenderButtonSvg;
        delete m_extenderIconSvg;
        m_extenderButtonSvg = 0;
        m_extenderIconSvg = 0;
    }*/
    delete m_extender;
}

void ExtenderButton::timerFired() {
    emit activated();
}

void ExtenderButton::startTimer() {
    ExtenderButtonTimer::instance()->startTimer(this);
}

void ExtenderButton::stopTimer() {
    ExtenderButtonTimer::instance()->stopTimer();
}

QRectF ExtenderButton::boundingRect() const
{
    if (!m_hover || m_extenderPosition == No)
        return QRectF(0, 0, size().width(), size().height());
    switch (m_extenderPosition) {
    case Top:
        return QRectF(0, - EXTENDER_SIZE, size().width(), size().height() + EXTENDER_SIZE);
    case Bottom:
        return QRectF(0, 0, size().width(), size().height() + EXTENDER_SIZE);
    case Left:
        return QRectF(- EXTENDER_SIZE, 0, size().width() + EXTENDER_SIZE, size().height());
    case Right:
        return QRectF(0, 0, size().width() + EXTENDER_SIZE, size().height());
    case No:
        return QRectF(0, 0, size().width(), size().height());
    }
    return QRectF();
}

void ExtenderButton::hoverEnterEvent(QGraphicsSceneHoverEvent * event)
{
    if (m_extenderPosition != No) {
        m_extender->setVisible(true);
    } else if (m_activationMethod == Hover) {
        //emit activated();
        startTimer();
    }
    BaseActionWidget::hoverEnterEvent(event);
}

void ExtenderButton::hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
{
    m_extender->setVisible(false);
    stopTimer();
    BaseActionWidget::hoverLeaveEvent(event);
}

void ExtenderButton::setExtenderPosition(
        ExtenderButton::ExtenderPosition position)
{
    if (m_extenderPosition == position)
        return;
    m_extenderPosition = position;
    relayoutExtender();
}

void ExtenderButton::relayoutExtender()
{
    if (!m_extender) return;
    switch (m_extenderPosition) {
    case Top:
        m_extender->setPos(0, - EXTENDER_SIZE);
        m_extender->resize(size().width(), EXTENDER_SIZE);
        //m_svgElementSufix = "_top";
        break;
    case Bottom:
        m_extender->setPos(0, size().height());
        m_extender->resize(size().width(), EXTENDER_SIZE);
        //m_svgElementSufix = "_bottom";
        break;
    case Left:
        m_extender->setPos(- EXTENDER_SIZE, 0);
        m_extender->resize(EXTENDER_SIZE, size().height());
        //m_svgElementSufix = "_left";
        break;
    case Right:
        m_extender->setPos(size().width(), 0);
        m_extender->resize(EXTENDER_SIZE, size().height());
        //m_svgElementSufix = "_right";
        break;
    case No:
        break;
    }
    //m_extender->m_svgElementSufix = m_svgElementSufix;
}

ExtenderButton::ExtenderPosition ExtenderButton::extenderPosition()
{
    return m_extenderPosition;
}

void ExtenderButton::setActivationMethod(ExtenderButton::ActivationMethod method)
{
    if (m_activationMethod == method)
        return;
    m_activationMethod = method;
    if (m_activationMethod != Extender)
        setExtenderPosition(ExtenderButton::No);
}

ExtenderButton::ActivationMethod ExtenderButton::activationMethod()
{
    return m_activationMethod;
}

void ExtenderButton::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    Q_UNUSED(event);
    emit activated();
}

void ExtenderButton::setGeometry(const QRectF & geometry)
{
    BaseActionWidget::setGeometry(geometry);
    relayoutExtender();
}

} // namespace Lancelot
