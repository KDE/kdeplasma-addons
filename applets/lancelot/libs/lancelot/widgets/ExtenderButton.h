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

#ifndef LANCELOT_EXTENDER_BUTTON_H_
#define LANCELOT_EXTENDER_BUTTON_H_

#include <lancelot/lancelot.h>
#include <lancelot/lancelot_export.h>

#include <QtGui>
#include <QtCore>
#include "Widget.h"
#include "BasicWidget.h"

#define EXTENDER_SIZE 20

namespace Lancelot
{

/**
 * Button widget with special activation options beside
 * clicking - hover and extender activation
 *
 * @author Ivan Cukic
 */
class LANCELOT_EXPORT ExtenderButton : public BasicWidget
{
    Q_OBJECT
    Q_PROPERTY ( ExtenderPosition extenderPosition READ extenderPosition WRITE setExtenderPosition )
    Q_PROPERTY ( ActivationMethod activationMethod READ activationMethod WRITE setActivationMethod )
public:
    /**
     * Creates a new Lancelot::ExtenderButton
     * @param name the internal name of the widget
     * @param title the title of the widget
     * @param description the description of the widget
     * @param parent parent item
     */
    ExtenderButton(QString name = QString(), QString title = QString(),
            QString description = QString(), QGraphicsItem * parent = 0);

    /**
     * Creates a new Lancelot::BasicWidget
     * @param name the internal name of the widget
     * @param icon the icon for the widget
     * @param title the title of the widget
     * @param description the description of the widget
     * @param parent parent item
     */
    ExtenderButton(QString name, QIcon icon, QString title = QString(),
            QString description = QString(), QGraphicsItem * parent = 0);

    /**
     * Creates a new Lancelot::BasicWidget
     * @param name the internal name of the widget
     * @param icon Svg with active, inactive and disabled states
     * @param title the title of the widget
     * @param description the description of the widget
     * @param parent parent item
     */
    ExtenderButton(QString name, Plasma::Svg * icon, QString title = QString(),
            QString description = QString(), QGraphicsItem * parent = 0);

    /**
     * Destroys Lancelot::ExtenderButton
     */
    virtual ~ExtenderButton();

    /**
     * Sets the position of the extender
     */
    void setExtenderPosition(ExtenderPosition position);

    /**
     * @returns the extender position
     */
    ExtenderPosition extenderPosition();

    /**
     * Sets the activation method of the ExtenderButton
     */
    void setActivationMethod(ActivationMethod method);

    /**
     * @returns activation method
     */
    ActivationMethod activationMethod();

    Override virtual void geometryUpdated();

    Override virtual void setGroup(WidgetGroup * group = NULL);
    Override virtual void groupUpdated();

    Override virtual QRectF boundingRect() const;

Q_SIGNALS:
    /**
     * Emitted when the button is activated
     */
    void activated();

protected:
    Override virtual void mousePressEvent(QGraphicsSceneMouseEvent * event);
    Override virtual void hoverEnterEvent(QGraphicsSceneHoverEvent * event);
    Override virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent * event);

private:
    class Private;
    Private * d;

    friend class ExtenderObject;
};

} // namespace Lancelot

#endif /* LANCELOT_EXTENDER_BUTTON_H_ */

