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

#ifndef EXTENDERBUTTON_H_
#define EXTENDERBUTTON_H_

#include <QtGui>
#include <QtCore>
#include <plasma/widgets/widget.h>
#include "BaseWidget.h"

#ifdef SUPER
#undef SUPER
#endif

#define SUPER BaseWidget
#define EXTENDER_SIZE 20

namespace Lancelot
{

class ExtenderButton : public SUPER
{
    Q_OBJECT
public:
    enum ExtenderPosition { NO = 0, RIGHT = 1, LEFT = 2, TOP = 3, BOTTOM = 4 };
    enum ActivationMethod { HOVER = 0, CLICK = 1, EXTENDER = 2 };

    class Extender : public SUPER {
    public:
        Extender(Plasma::Svg * icon, QGraphicsItem * parent = 0) : SUPER(icon, "", "", parent) {
            m_svgElementPrefix = "extender_";
        };
        friend class ExtenderButton;
        
    };

    ExtenderButton(QString title = QString(), QString description = QString(), QGraphicsItem * parent = 0);
    ExtenderButton(QIcon * icon, QString title = QString(), QString description = QString(), QGraphicsItem * parent = 0);
    ExtenderButton(Plasma::Svg * icon, QString title = QString(), QString description = QString(), QGraphicsItem * parent = 0);

    virtual ~ExtenderButton();

    virtual QRectF boundingRect () const;
	
    virtual void hoverEnterEvent ( QGraphicsSceneHoverEvent * event );
    virtual void hoverLeaveEvent ( QGraphicsSceneHoverEvent * event );
    
    void setExtenderPosition(ExtenderPosition position);
    ExtenderPosition extenderPosition();
    
    void setActivationMethod(ActivationMethod method);
    ActivationMethod activationMethod();
    
    void setGeometry (const QRectF & geometry);
    
protected:
    void mousePressEvent ( QGraphicsSceneMouseEvent * event );
    
private:
    void relayoutExtender();
    void init();
    
    Extender * m_extender;
    ExtenderPosition m_extenderPosition;
    ActivationMethod m_activationMethod;
    
    static Plasma::Svg * m_extenderIconSvg;
    static Plasma::Svg * m_extenderButtonSvg;
    static int * m_extendersCount;

Q_SIGNALS:
    void activated();
    
    
	
};

}

#endif /*EXTENDERBUTTON_H_*/
