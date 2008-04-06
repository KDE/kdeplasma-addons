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

// TODO: Convert to dptr

#ifndef LANCELOT_EXTENDERBUTTON_H_
#define LANCELOT_EXTENDERBUTTON_H_

#include "../lancelot_export.h"

#include <QtGui>
#include <QtCore>
#include "Widget.h"
#include "BaseActionWidget.h"

#define EXTENDER_SIZE 20

namespace Lancelot
{

class ExtenderButton;

class LANCELOT_EXPORT ExtenderButtonTimer: public QObject {
    Q_OBJECT
public:
    static ExtenderButtonTimer * instance();
    void startTimer(ExtenderButton * owner);
    void stopTimer();

protected slots:
    void fire();

private:
    ExtenderButtonTimer();
    ~ExtenderButtonTimer();

    static ExtenderButtonTimer * m_instance;
    QTimer m_timer;
    ExtenderButton * m_owner;
};


class LANCELOT_EXPORT ExtenderButton : public BaseActionWidget
{
    Q_OBJECT
public:
    enum ExtenderPosition { No = 0, Right = 1, Left = 2, Top = 3, Bottom = 4 };
    enum ActivationMethod { Hover = 0, Click = 1, Extender = 2 };

    class ExtenderObject : public BaseActionWidget {
    public:
        ExtenderObject(QString name, Plasma::Svg * icon, QGraphicsItem * parent = 0) : BaseActionWidget(name, icon, "", "", parent) {
            setInnerOrientation(Vertical);
            setAlignment(Qt::AlignCenter);
        }
        friend class ExtenderButton;

    };

    ExtenderButton(QString name = QString(), QString title = QString(), QString description = QString(), QGraphicsItem * parent = 0);
    ExtenderButton(QString name, QIcon * icon, QString title = QString(), QString description = QString(), QGraphicsItem * parent = 0);
    ExtenderButton(QString name, Plasma::Svg * icon, QString title = QString(), QString description = QString(), QGraphicsItem * parent = 0);

    virtual ~ExtenderButton();

    virtual QRectF boundingRect () const;

    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent * event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent * event);

    void setExtenderPosition(ExtenderPosition position);
    ExtenderPosition extenderPosition();

    void setActivationMethod(ActivationMethod method);
    ActivationMethod activationMethod();

    void setGeometry (const QRectF & geometry);
    void setGroup(WidgetGroup * group = NULL);
    void groupUpdated();

protected:
    void mousePressEvent (QGraphicsSceneMouseEvent * event);

private:
    void relayoutExtender();
    void init();

    ExtenderObject * m_extender;
    ExtenderPosition m_extenderPosition;
    ActivationMethod m_activationMethod;

    static Plasma::Svg * m_extenderIconSvg;
    //static Plasma::Svg * m_extenderButtonSvg;
    //static int m_extendersCount;

    void timerFired();

protected slots:
    void startTimer();
    void stopTimer();

Q_SIGNALS:
    void activated();

    friend class ExtenderButtonTimer;
};

}

#endif /*EXTENDERBUTTON_H_*/
