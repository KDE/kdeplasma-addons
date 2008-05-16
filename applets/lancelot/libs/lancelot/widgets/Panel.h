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

#ifndef LANCELOT_PANEL_H_
#define LANCELOT_PANEL_H_

#include <lancelot/lancelot_export.h>
#include <plasma/plasma.h>

#include <lancelot/widgets/Widget.h>

#include <QIcon>

namespace Lancelot
{

/**
 * A container widget with optional title-bar
 *
 * @author Ivan Cukic
 */
class LANCELOT_EXPORT Panel: public Lancelot::Widget
{
    Q_OBJECT

    Q_PROPERTY ( QIcon icon READ icon WRITE setIcon )
    Q_PROPERTY ( QSize iconSize READ iconSize WRITE setIconSize )
    Q_PROPERTY ( QString title READ title WRITE setTitle ) L_EXTRA ( i18n )

    L_WIDGET
    L_INCLUDE(lancelot/widgets/Panel.h QIcon QSize QString)

public:
    /**
     * Creates a new Lancelot::Widget
     * @param parent parent item
     */
    Panel(QGraphicsItem * parent = 0);

    /**
     * Creates a new Lancelot::Panel
     * @param icon the icon for the widget
     * @param title the title of the widget
     * @param parent parent item
     */
    Panel(QIcon icon, QString title = QString(),
            QGraphicsItem * parent = 0);

    /**
     * Creates a new Lancelot::Widget
     * @param title the title of the widget
     * @param parent parent item
     */
    Panel(QString title, QGraphicsItem * parent = 0);

    /**
     * Destroys Lancelot::Panel
     */
    virtual ~Panel();

    /**
     * Sets title of this Lancelot::BasicWidget
     * @param title new title
     */
    void setTitle(const QString & title);

    /**
     * @returns title of this Lancelot::BasicWidget
     */
    QString title() const;

    /**
     * Sets icon of this Lancelot::BasicWidget
     * @param icon new icon
     */
    void setIcon(QIcon icon);

    /**
     * @returns icon of this Lancelot::BasicWidget
     */
    QIcon icon() const;

    /**
     * Sets icon size of this Lancelot::BasicWidget
     * @param size new icon size
     */
    void setIconSize(QSize size);

    /**
     * @returns icon size of this Lancelot::BasicWidget
     */
    QSize iconSize() const;

    /**
     * Sets the layout item for the components in case the panel
     * should contain more than one item at a time
     * @param layoutItem the contained layout item
     */
    void setLayoutItem(QGraphicsLayoutItem * layoutItem);

    /**
     * @returns the layout item
     */
    QGraphicsLayoutItem * layoutItem();

    /**
     * Sets the background image for the panel
     * @param imagePath location of Svg to use as background
     */
    void setBackground(const QString & imagePath);

    /**
     * Clears the background
     */
    void clearBackground();

    /**
     * @param edge edge
     * @returns size of the specified edge
     */
    qreal borderSize(Plasma::MarginEdge edge);

    L_Override virtual void setGeometry(qreal x, qreal y, qreal w, qreal h);
    L_Override virtual void setGeometry(const QRectF & geometry);
    L_Override virtual void setGroup(WidgetGroup * group = NULL);

protected:
    L_Override virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);
    L_Override virtual QSizeF sizeHint(Qt::SizeHint which,
            const QSizeF & constraint = QSizeF()) const;

private:
    class Private;
    Private * const d;
};

} // namespace Lancelot

#endif /* LANCELOT_PANEL_H_*/

