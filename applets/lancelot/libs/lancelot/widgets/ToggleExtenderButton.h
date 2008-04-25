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

#ifndef LANCELOT_TOGGLE_EXTENDER_BUTTON_H_
#define LANCELOT_TOGGLE_EXTENDER_BUTTON_H_

#include <lancelot/lancelot_export.h>

#include "ExtenderButton.h"

namespace Lancelot
{

/**
 * Toggle button widget with special activation (toggling) options
 * beside clicking - hover and extender activation
 *
 * @author Ivan Cukic
 */
class LANCELOT_EXPORT ToggleExtenderButton : public Lancelot::ExtenderButton {
    Q_OBJECT
    Q_PROPERTY ( bool pressed READ isPressed WRITE setPressed )
public:
    /**
     * Creates a new Lancelot::ToggleExtenderButton
     * @param name the internal name of the widget
     * @param title the title of the widget
     * @param description the description of the widget
     * @param parent parent item
     */
    ToggleExtenderButton(QString name = QString(), QString title = QString(),
            QString description = QString(), QGraphicsItem * parent = 0);

    /**
     * Creates a new Lancelot::BasicWidget
     * @param name the internal name of the widget
     * @param icon the icon for the widget
     * @param title the title of the widget
     * @param description the description of the widget
     * @param parent parent item
     */
    ToggleExtenderButton(QString name, QIcon icon, QString title = QString(),
            QString description = QString(), QGraphicsItem * parent = 0);

    /**
     * Creates a new Lancelot::BasicWidget
     * @param name the internal name of the widget
     * @param icon Svg with active, inactive and disabled states
     * @param title the title of the widget
     * @param description the description of the widget
     * @param parent parent item
     */
    ToggleExtenderButton(QString name, Plasma::Svg * icon, QString title = QString(),
            QString description = QString(), QGraphicsItem * parent = 0);

    /**
     * Destroys Lancelot::ToggleExtenderButton
     */
    virtual ~ToggleExtenderButton();

    /**
     * @returns whether the button is in pressed state
     */
    bool isPressed() const;

    /**
     * Sets whether the button is in pressed state
     */
    void setPressed(bool pressed = true);

public slots:
    /**
     * Toggles the pressed state
     */
    void toggle();

Q_SIGNALS:
    /**
     * This signal is sent when the pressed state is changed
     * @param pressed the new state
     */
    void toggled(bool pressed);

protected:
    Override virtual void paint(QPainter * painter,
            const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

private:
    class Private;
    Private * d;
};

} // namespace Lancelot

#endif /* LANCELOT_TOGGLE_EXTENDER_BUTTON_H_ */

