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

#ifndef LANCELOT_BASEACTIONWIDGET_H_
#define LANCELOT_BASEACTIONWIDGET_H_

#include "../lancelot_export.h"

#include <QtGui>
#include <QtCore>
#include <plasma/svg.h>
#include "Widget.h"

namespace Lancelot
{

/**
 * Provides a common appeareance with an icon, title
 * and a description. %Layout can be vertical or
 * horizontal.
 *
 * @author Ivan Cukic
 *
 * The icon can be a QIcon or contained in a Plasma::Svg
 */
class LANCELOT_EXPORT BasicWidget: public Widget {
    Q_OBJECT
    Q_PROPERTY ( QIcon icon READ icon WRITE setIcon )
    Q_PROPERTY ( QSize iconSize READ iconSize WRITE setIconSize )

    Q_PROPERTY ( QString name READ name WRITE setName )
    Q_PROPERTY ( QString title READ title WRITE setTitle )
    Q_PROPERTY ( QString description READ description WRITE setDescription )
    Q_PROPERTY ( Qt::Orientation innerOrientation READ innerOrientation WRITE setInnerOrientation )

public:
    /**
     * Creates a new Lancelot::BasicWidget
     * @param name the internal name of the widget
     * @param title the title of the widget
     * @param description the description of the widget
     * @param parent parent item
     */
    BasicWidget(QString name = QString(), QString title = QString(),
            QString description = QString(), QGraphicsItem * parent = 0);

    /**
     * Creates a new Lancelot::BasicWidget
     * @param name the internal name of the widget
     * @param icon the icon for the widget
     * @param title the title of the widget
     * @param description the description of the widget
     * @param parent parent item
     */
    BasicWidget(QString name, QIcon icon, QString title = QString(),
            QString description = QString(), QGraphicsItem * parent = 0);

    /**
     * Creates a new Lancelot::BasicWidget
     * @param name the internal name of the widget
     * @param icon Svg with active, inactive and disabled states
     * @param title the title of the widget
     * @param description the description of the widget
     * @param parent parent item
     */
    BasicWidget(QString name, Plasma::Svg * icon, QString title = QString(),
            QString description = QString(), QGraphicsItem * parent = 0);

    /**
     * Destroys Lancelot::BasicWidget
     */
    virtual ~BasicWidget();

    /**
     * @returns icon of this Lancelot::BasicWidget
     */
    QIcon icon() const;

    /**
     * Sets icon of this Lancelot::BasicWidget
     */
    void setIcon(QIcon icon);

    /**
     * @returns icon of this Lancelot::BasicWidget
     */
    Plasma::Svg * iconInSvg() const;

    /**
     * Sets icon of this Lancelot::BasicWidget
     */
    void setIconInSvg(Plasma::Svg * svg);

    /**
     * @returns icon size of this Lancelot::BasicWidget
     */
    QSize iconSize() const;

    /**
     * Sets icon size of this Lancelot::BasicWidget
     */
    void setIconSize(QSize size);

    /**
     * @returns inner orientation of this Lancelot::BasicWidget
     */
    Qt::Orientation innerOrientation() const;

    /**
     * Sets the inner orientation of this Lancelot::BasicWidget
     */
    void setInnerOrientation(Qt::Orientation orientation);

    /**
     * @returns alignment of this Lancelot::BasicWidget
     */
    Qt::Alignment alignment() const;

    /**
     * Sets alignment of this Lancelot::BasicWidget
     */
    void setAlignment(Qt::Alignment alignment);

    /**
     * @returns title of this Lancelot::BasicWidget
     */
    QString title() const;

    /**
     * Sets title of this Lancelot::BasicWidget
     */
    void setTitle(const QString & title);

    /**
     * @returns description of this Lancelot::BasicWidget
     */
    QString description() const;

    /**
     * Sets description of this Lancelot::BasicWidget
     */
    void setDescription(const QString & description);

protected:
    Override virtual void paint(QPainter * painter,
            const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

    /**
     * Common implementation of icon/text/description
     * rendering
     */
    void paintForeground(QPainter * painter);

private:
    class Private;
    Private * d;
};

} // namespace Lancelot

#endif /*LANCELOT_BASEACTIONWIDGET_H_*/
