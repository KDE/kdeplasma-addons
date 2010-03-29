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

#ifndef LANCELOT_EXTENDER_BUTTON_H
#define LANCELOT_EXTENDER_BUTTON_H

#include <lancelot/lancelot.h>
#include <lancelot/lancelot_export.h>
#include <lancelot/widgets/BasicWidget.h>
#include <lancelot/widgets/Widget.h>

#define EXTENDER_SIZE 20

namespace Lancelot
{

/**
 * Button widget with special activation options beside
 * clicking - hover and extender activation
 *
 * @author Ivan Cukic
 */
class LANCELOT_EXPORT ExtenderButton : public Lancelot::BasicWidget
{
    Q_OBJECT

    /* type: ExtenderPosition */
    Q_PROPERTY ( int extenderPosition READ extenderPosition WRITE setExtenderPosition )
    /* type: ActivationMethod */
    Q_PROPERTY ( int activationMethod READ activationMethod WRITE setActivationMethod )
    Q_PROPERTY ( bool checkable READ isCheckable WRITE setCheckable )
    Q_PROPERTY ( bool checked READ isChecked WRITE setChecked )

    // @puck L_WIDGET
    // @puck L_INCLUDE(lancelot/widgets/ExtenderButton.h lancelot/lancelot.h)

public:
    /**
     * Creates a new Lancelot::ExtenderButton
     * @param parent parent item
     */
    ExtenderButton(QGraphicsItem * parent = 0);

    /**
     * Creates a new Lancelot::ExtenderButton
     * @param title the title of the widget
     * @param description the description of the widget
     * @param parent parent item
     */
    explicit ExtenderButton(QString title, QString description = QString(),
            QGraphicsItem * parent = 0);

    /**
     * Creates a new Lancelot::BasicWidget
     * @param icon the icon for the widget
     * @param title the title of the widget
     * @param description the description of the widget
     * @param parent parent item
     */
    explicit ExtenderButton(QIcon icon, QString title = QString(),
            QString description = QString(), QGraphicsItem * parent = 0);

    /**
     * Creates a new Lancelot::BasicWidget
     * @param icon Svg with active, inactive and disabled states
     * @param title the title of the widget
     * @param description the description of the widget
     * @param parent parent item
     */
    explicit ExtenderButton(const Plasma::Svg & icon, QString title = QString(),
            QString description = QString(), QGraphicsItem * parent = 0);

    /**
     * Destroys Lancelot::ExtenderButton
     */
    virtual ~ExtenderButton();

    /**
     * Sets the position of the extender
     * @param position new position
     */
    void setExtenderPosition(int position);

    /**
     * @returns the extender position
     */
    int extenderPosition() const;

    /**
     * Sets the activation method of the ExtenderButton
     * @param method new activation method
     */
    void setActivationMethod(int method);

    /**
     * @returns activation method
     */
    int activationMethod() const;

    /**
     * Makes the button checkable when set to true
     * @param checkable checkable
     */
    void setCheckable(bool checkable);

    /**
     * @returns whether the button is checkable
     */
    bool isCheckable() const;

    /**
     * @returns whether the button is checked
     */
    bool isChecked() const;

    L_Override void setGroup(Group * group = NULL);

    L_Override QRectF boundingRect() const;

    L_Override void paint(QPainter * painter,
            const QStyleOptionGraphicsItem * option, QWidget * widget = 0);
    L_Override void setShortcutKey(const QString & key);

public Q_SLOTS:
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

Q_SIGNALS:
    /**
     * Emitted when the button is activated
     * @param checked true if the button is checked
     */
    void activated();

    /**
     * Emitted when the state of a checkable button is changed
     * @param checked the new state
     */
    void toggled(bool checked);

protected:
    L_Override void hoverEnterEvent(QGraphicsSceneHoverEvent * event);
    L_Override void hoverLeaveEvent(QGraphicsSceneHoverEvent * event);
    L_Override void resizeEvent(QGraphicsSceneResizeEvent * event);
    L_Override void hideEvent(QHideEvent * event);

protected Q_SLOTS:
    void activate();

private:
    class Private;
    Private * const d;

    friend class ExtenderObject;
};

} // namespace Lancelot

#endif /* LANCELOT_EXTENDER_BUTTON_H */

