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

#ifndef LANCELOT_WIDGET_H_
#define LANCELOT_WIDGET_H_

#include <lancelot/lancelot_export.h>
#include <lancelot/Global.h>

#include <QtGui>
#include <QGraphicsWidget>

namespace Lancelot
{

/**
 * Base class for Widgets that want to use Lancelot framework
 *
 * @author Ivan Cukic
 */
class LANCELOT_EXPORT Widget : public QGraphicsWidget
{
    Q_OBJECT
    Q_PROPERTY ( QString name READ name WRITE setName )
public:
    /**
     * Creates a new Lancelot::Widget
     * @param name the internal name of the widget
     * @param parent parent item
     */
    Widget(QString name = "", QGraphicsItem * parent = 0);

    /**
     * Destroys Lancelot::Widget
     */
    virtual ~Widget();

    /**
     * Sets this widget's group by group name.
     */
    virtual void setGroupByName(const QString & groupName);

    /**
     * Sets this widget's group.
     */
    virtual void setGroup(WidgetGroup * group = NULL);

    /**
     * Returns this widget's group.
     */
    WidgetGroup * group();

    /**
     * Returns the Lancelot::Instance to which this object
     * belongs.
     */
    Instance * instance();

    /**
     * Returns the name of the widget.
     */
    QString name() const;

    /**
     * Sets the name of the widget
     */
    void setName(QString name);

    /**
     * Returns whether the mouse cursor is hovering the widget
     */
    bool isHovered() const;

    Override virtual void setGeometry(const QRectF & rect);
    Override virtual void setGeometry(qreal x, qreal y, qreal w, qreal h);

Q_SIGNALS:
    /**
     * This signal is emitted when the mouse cursor enters the widget
     */
    void mouseHoverEnter();

    /**
     * This signal is emitted when the mouse cursor leaves the widget
     */
    void mouseHoverLeave();

protected:
    /**
     * This function is invoked when the group containing this
     * widget is updated. Reimplement if you need any additional
     * properties in your widget which need to be set using
     * the groups mechanism.
     */
    virtual void groupUpdated();

    /**
     * Paints the widget background using the widget's SVG. Element name is
     * calculated according to widget's state
     */
    void paintBackground(QPainter * painter);

    /**
     * Paints the widget background using the specified element from widget's SVG
     */
    void paintBackground(QPainter * painter, const QString & element);

    /**
     * Reimplement this function if you want to be notified
     * when a geometry changes. This is not the same function
     * as QGraphicsWidget::updateGeometry()
     */
    virtual void geometryUpdated();

    Override virtual void hoverEnterEvent(QGraphicsSceneHoverEvent * event);
    Override virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent * event);
    Override virtual void paint(QPainter * painter,
            const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

private:
    class Private;
    Private * d;

    friend class WidgetGroup;
    friend class Global;
};

} // namespace Lancelot

#endif /* LANCELOT_WIDGET_H_ */

