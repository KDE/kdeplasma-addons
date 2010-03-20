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

#ifndef LANCELOT_WIDGET_H
#define LANCELOT_WIDGET_H

#include <QtGui/QGraphicsWidget>

#include <lancelot/lancelot_export.h>
#include <lancelot/lancelot.h>
#include <lancelot/Global.h>

namespace Lancelot
{

/**
 * Base class for Widgets that want to use Lancelot framework
 *
 * @author Ivan Cukic
 */
class LANCELOT_EXPORT Widget : public QGraphicsWidget {
    Q_OBJECT

    Q_PROPERTY(QString group READ groupName WRITE setGroupByName)
    Q_PROPERTY(bool down READ isDown WRITE setDown)

    // @puck L_WIDGET
    // @puck L_INCLUDE(lancelot/widgets/Widget.h QString)

public:
    /**
     * Creates a new Lancelot::Widget
     * @param parent parent item
     */
    Widget(QGraphicsItem * parent = 0);

    /**
     * Destroys Lancelot::Widget
     */
    virtual ~Widget();

    /**
     * Sets this widget's group by group name.
     * @param groupName name of the group
     */
    virtual void setGroupByName(const QString & groupName);

    /**
     * @returns this widget's group's name.
     */
    QString groupName() const;

    /**
     * Sets this widget's group.
     * @param group new group
     */
    virtual void setGroup(Group * group = NULL);

    /**
     * Returns this widget's group.
     */
    Group * group() const;

    /**
     * Returns whether the mouse cursor is hovering the widget
     */
    bool isHovered() const;

    /**
     * @returns whether the button is down
     */
    bool isDown() const;

protected:
    L_Override QSizeF sizeHint(Qt::SizeHint which,
            const QSizeF & constraint = QSizeF()) const;

    void setPaintBackwardsWhenRTL(bool value);

Q_SIGNALS:
    /**
     * This signal is emitted when the mouse cursor enters the widget
     */
    void mouseHoverEnter();

    /**
     * This signal is emitted when the mouse cursor leaves the widget
     */
    void mouseHoverLeave();

    /**
     * Emitted when the button is clicked.
     * You should use the activated() signal instead if you want to
     * support other activation methods beside clicking.
     * @param checked true if the button is checked
     */
    void clicked();

    /**
     * This signal is emitted when the button is pressed down
     */
    void pressed();

    /**
     * This signal is emitted when the button is released
     */
    void released();

protected:
    /**
     * Paints the widget background using the widget's SVG. Element name is
     * calculated according to widget's state
     * @param painter painter to paint on
     */
    void paintBackground(QPainter * painter);

    /**
     * Paints the widget background using the specified element from widget's SVG
     * @param painter painter to paint on
     * @param element element to paint
     */
    void paintBackground(QPainter * painter, const QString & element);

    /**
     * Sets whether the widget is hovered. This function exists
     * only as a hack if you need a way to artificially force
     * this value.
     */
    void setHovered(bool value);

    /**
     * Sets whether the widget is down (pressed). This function exists
     * only as a hack if you need a way to artificially force
     * this value.
     */
    void setDown(bool value);

    L_Override void hoverEnterEvent(QGraphicsSceneHoverEvent * event);
    L_Override void hoverLeaveEvent(QGraphicsSceneHoverEvent * event);

    L_Override void mousePressEvent(QGraphicsSceneMouseEvent * event);
    L_Override void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);
    L_Override void hideEvent(QHideEvent * event);

    L_Override void paint(QPainter * painter,
            const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

private:
    class Private;
    Private * const d;
};

} // namespace Lancelot

#endif /* LANCELOT_WIDGET_H */

