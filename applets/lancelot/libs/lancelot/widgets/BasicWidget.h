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

#ifndef LANCELOT_BASIC_WIDGET_H_
#define LANCELOT_BASIC_WIDGET_H_

#include <lancelot/lancelot_export.h>
#include <lancelot/lancelot.h>

#include <QtGui>
#include <QtCore>
#include <plasma/svg.h>

#include <lancelot/widgets/Widget.h>

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
    Q_PROPERTY ( QString title READ title WRITE setTitle )
    Q_PROPERTY ( QString description READ description WRITE setDescription )
    Q_PROPERTY ( Qt::Orientation innerOrientation READ innerOrientation WRITE setInnerOrientation )

    L_WIDGET

public:
    /**
     * Creates a new Lancelot::BasicWidget
     * @param title the title of the widget
     * @param description the description of the widget
     * @param parent parent item
     */
    BasicWidget(QString title = QString(),
            QString description = QString(), QGraphicsItem * parent = 0);

    /**
     * Creates a new Lancelot::BasicWidget
     * @param icon the icon for the widget
     * @param title the title of the widget
     * @param description the description of the widget
     * @param parent parent item
     */
    BasicWidget(QIcon icon, QString title = QString(),
            QString description = QString(), QGraphicsItem * parent = 0);

    /**
     * Creates a new Lancelot::BasicWidget
     * @param icon Svg with active, inactive and disabled states
     * @param title the title of the widget
     * @param description the description of the widget
     * @param parent parent item
     */
    BasicWidget(Plasma::Svg * icon, QString title = QString(),
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
     * @param icon new icon
     */
    void setIcon(QIcon icon);

    /**
     * @returns icon of this Lancelot::BasicWidget
     */
    Plasma::Svg * iconInSvg() const;

    /**
     * Sets icon of this Lancelot::BasicWidget
     * @param svg new icon
     */
    void setIconInSvg(Plasma::Svg * svg);

    /**
     * @returns icon size of this Lancelot::BasicWidget
     */
    QSize iconSize() const;

    /**
     * Sets icon size of this Lancelot::BasicWidget
     * @param size new icon size
     */
    void setIconSize(QSize size);

    /**
     * @returns inner orientation of this Lancelot::BasicWidget
     */
    Qt::Orientation innerOrientation() const;

    /**
     * Sets the inner orientation of this Lancelot::BasicWidget
     * @param orientation new inner orientation
     */
    void setInnerOrientation(Qt::Orientation orientation);

    /**
     * @returns alignment of this Lancelot::BasicWidget
     */
    Qt::Alignment alignment() const;

    /**
     * Sets alignment of this Lancelot::BasicWidget
     * @param alignment new alignment
     */
    void setAlignment(Qt::Alignment alignment);

    /**
     * @returns title of this Lancelot::BasicWidget
     */
    QString title() const;

    /**
     * Sets title of this Lancelot::BasicWidget
     * @param title new title
     */
    void setTitle(const QString & title);

    /**
     * @returns description of this Lancelot::BasicWidget
     */
    QString description() const;

    /**
     * Sets description of this Lancelot::BasicWidget
     * @return description new description
     */
    void setDescription(const QString & description);

protected:
    /**
     * Common implementation of icon/text/description
     * rendering
     */
    void paintForeground(QPainter * painter);

    Override virtual void paint(QPainter * painter,
            const QStyleOptionGraphicsItem * option, QWidget * widget = 0);
    Override virtual QSizeF sizeHint(Qt::SizeHint which,
            const QSizeF & constraint = QSizeF()) const;

private:
    class Private;
    Private * const d;
};

} // namespace Lancelot

#endif /* LANCELOT_BASIC_WIDGET_H_ */

