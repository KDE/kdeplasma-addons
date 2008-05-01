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

#include <lancelot/widgets/Widget.h>
#include <lancelot/widgets/BasicWidget.h>

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
    Q_PROPERTY ( bool checkable READ isCheckable WRITE setCheckable )
    Q_PROPERTY ( bool checked READ isChecked WRITE setChecked )
    Q_PROPERTY ( bool down READ isDown WRITE setDown )

    L_WIDGET

public:
    /**
     * Creates a new Lancelot::ExtenderButton
     * @param title the title of the widget
     * @param description the description of the widget
     * @param parent parent item
     */
    ExtenderButton(QString title = QString(),
            QString description = QString(), QGraphicsItem * parent = 0);

    /**
     * Creates a new Lancelot::BasicWidget
     * @param icon the icon for the widget
     * @param title the title of the widget
     * @param description the description of the widget
     * @param parent parent item
     */
    ExtenderButton(QIcon icon, QString title = QString(),
            QString description = QString(), QGraphicsItem * parent = 0);

    /**
     * Creates a new Lancelot::BasicWidget
     * @param icon Svg with active, inactive and disabled states
     * @param title the title of the widget
     * @param description the description of the widget
     * @param parent parent item
     */
    ExtenderButton(Plasma::Svg * icon, QString title = QString(),
            QString description = QString(), QGraphicsItem * parent = 0);

    /**
     * Destroys Lancelot::ExtenderButton
     */
    virtual ~ExtenderButton();

    /**
     * Sets the position of the extender
     * @param position new position
     */
    void setExtenderPosition(ExtenderPosition position);

    /**
     * @returns the extender position
     */
    ExtenderPosition extenderPosition();

    /**
     * Sets the activation method of the ExtenderButton
     * @param method new activation method
     */
    void setActivationMethod(ActivationMethod method);

    /**
     * @returns activation method
     */
    ActivationMethod activationMethod();

    /**
     * Makes the button checkable when set to true
     * @param checkable checkable
     */
    void setCheckable(bool checkable);

    /**
     * @returns whether the button is checkable
     */
    bool isCheckable();

    /**
     * @returns whether the button is checked
     */
    bool isChecked();

    /**
     * @returns whether the button is down
     */
    bool isDown();

    Override virtual void geometryUpdated();

    Override virtual void setGroup(WidgetGroup * group = NULL);
    Override virtual void groupUpdated();

    Override virtual QRectF boundingRect() const;

    Override virtual void paint(QPainter * painter,
            const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

public slots:
    /**
     * Sets whether the button is checked.
     * Has no effect if button is not checkable.
     * @param checked checked
     */
    void setChecked(bool checked);

    /**
     * Toggles the checked state.
     * Has no effect if button is not checkable.
     */
    void toggle();

    /**
     * Sets whether the button is down
     * @param down down
     */
    void setDown(bool down);

Q_SIGNALS:
    /**
     * Emitted when the button is activated
     * @param checked true if the button is checked
     */
    void activated(bool checked = false);

    /**
     * Emitted when the state of a checkable button is changed
     * @param checked the new state
     */
    void toggled(bool checked);

    /**
     * Emitted when the button is clicked.
     * You should use the activated() signal instead if you want to
     * support other activation methods beside clicking.
     * @param checked true if the button is checked
     */
    void clicked(bool checked = false);

    /**
     * This signal is emitted when the button is pressed down
     */
    void pressed();

    /**
     * This signal is emitted when the button is released
     */
    void released();

protected:
    Override virtual void mousePressEvent(QGraphicsSceneMouseEvent * event);
    Override virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);
    Override virtual void hoverEnterEvent(QGraphicsSceneHoverEvent * event);
    Override virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent * event);

private:
    class Private;
    Private * const d;

    friend class ExtenderObject;
};

} // namespace Lancelot

#endif /* LANCELOT_EXTENDER_BUTTON_H_ */

