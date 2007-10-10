/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "ExtenderButton.h"

#include <QRectF>
#include <KDebug>
#include <KIcon>

namespace Lancelot {

Plasma::Svg * ExtenderButton::m_extenderButtonSvg= NULL;
Plasma::Svg * ExtenderButton::m_extenderIconSvg= NULL;
int * ExtenderButton::m_extendersCount = 0;

ExtenderButton::ExtenderButton(QString name, QString title, QString description,
        QGraphicsItem * parent) :
    SUPER(name, title, description, parent), m_extender(NULL),
            m_extenderPosition(NO), m_activationMethod(CLICK)
{
    init();
}

ExtenderButton::ExtenderButton(QString name, QIcon * icon, QString title,
        QString description, QGraphicsItem * parent) :
    SUPER(name, icon, title, description, parent), m_extender(NULL),
            m_extenderPosition(NO), m_activationMethod(CLICK)
{
    init();
}

ExtenderButton::ExtenderButton(QString name, Plasma::Svg * icon, QString title,
        QString description, QGraphicsItem * parent) :
    SUPER(name, icon, title, description, parent), m_extender(NULL),
            m_extenderPosition(NO), m_activationMethod(CLICK)
{
    init();
}

void ExtenderButton::init()
{
    kDebug() << "ExtenderButton::init()\n";
    if (!m_extenderButtonSvg) {
        m_extenderButtonSvg = new Plasma::Svg("lancelot/extender_button");
        m_extenderButtonSvg->setContentType(Plasma::Svg::ImageSet);

        m_extenderIconSvg = new Plasma::Svg("lancelot/extender_button_icon");
        m_extenderIconSvg->setContentType(Plasma::Svg::ImageSet);

        ++m_extendersCount;
    }

    m_svg = m_extenderButtonSvg;

    m_extender = new Extender(name() + "::Extender", m_extenderIconSvg, this);
    m_extender->setVisible(false);
    m_extender->m_svg = m_svg;
    m_extender->m_iconSize = QSize(16, 16);

    connect(m_extender, SIGNAL(mouseHoverEnter()), this, SIGNAL(activated()));
}

ExtenderButton::~ExtenderButton()
{
    kDebug() << "ExtenderButton::~ExtenderButton()\n";

    if (--m_extendersCount == 0) {
        delete m_extenderButtonSvg;
        delete m_extenderIconSvg;
    }
    delete m_extender;
}

QRectF ExtenderButton::boundingRect() const
{
    if (!m_hover || m_extenderPosition == NO)
        return QRectF(0, 0, size().width(), size().height());
    switch (m_extenderPosition) {
    case TOP:
        return QRectF(0, - EXTENDER_SIZE, size().width(), size().height() + EXTENDER_SIZE);
    case BOTTOM:
        return QRectF(0, 0, size().width(), size().height() + EXTENDER_SIZE);
    case LEFT:
        return QRectF(- EXTENDER_SIZE, 0, size().width() + EXTENDER_SIZE, size().height());
    case RIGHT:
        return QRectF(0, 0, size().width() + EXTENDER_SIZE, size().height());
    case NO:
        return QRectF(0, 0, size().width(), size().height());
    }
    return QRectF();
}

void ExtenderButton::hoverEnterEvent(QGraphicsSceneHoverEvent * event)
{
    if (m_extenderPosition != NO)
        m_extender->setVisible(true);
    else if (m_activationMethod == HOVER)
emit         activated();
    SUPER::hoverEnterEvent(event);
}

void ExtenderButton::hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
{
    m_extender->setVisible(false);
    SUPER::hoverLeaveEvent(event);
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
    switch (m_extenderPosition) {
    case TOP:
        m_extender->setPos(0, - EXTENDER_SIZE);
        m_extender->resize(size().width(), EXTENDER_SIZE);
        m_svgElementSufix = "_top";
        break;
    case BOTTOM:
        m_extender->setPos(0, size().height());
        m_extender->resize(size().width(), EXTENDER_SIZE);
        m_svgElementSufix = "_bottom";
        break;
    case LEFT:
        m_extender->setPos(- EXTENDER_SIZE, 0);
        m_extender->resize(EXTENDER_SIZE, size().height());
        m_svgElementSufix = "_left";
        break;
    case RIGHT:
        m_extender->setPos(size().width(), 0);
        m_extender->resize(EXTENDER_SIZE, size().height());
        m_svgElementSufix = "_right";
        break;
    case NO:
        break;
    }
    m_extender->m_svgElementSufix = m_svgElementSufix;
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
    if (m_activationMethod != EXTENDER)
        setExtenderPosition(ExtenderButton::NO);
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
    SUPER::setGeometry(geometry);
    relayoutExtender();
}

} // namespace Lancelot
