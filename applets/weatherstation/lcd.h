/*
 *   Copyright (C) 2007, 2008 Petri Damsten <damu@iki.fi>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#ifndef LCD_HEADER
#define LCD_HEADER

#include <QGraphicsWidget>

/**
 * @class LCD
 *
 * @short Provides a widget to display lcd like svgs in plasma.
 *
 * Widget can turn on/off svg elements individually and in groups.
 * It has helper functions to handle seven segment numbers in element groups.
 * Elements that should be always drawn must be groupped under name 'background'
 *
 * Element groups are name [maingroup:]groupname:elementname.
 * Seven segment numbers are special groups named numbername:digit:segment
 * e.g Element for decimal point in first digit in number named speed is speed:0:DP
 * Segment names: http://en.wikipedia.org/wiki/Seven-segment_display
 */
class LCD : public QGraphicsWidget
{
    Q_OBJECT
    Q_PROPERTY(QString svg READ svg WRITE setSvg)
public:

    /**
     * Constructor
     * @param parent the QGraphicsItem this meter is parented to.
     */
    explicit LCD(QGraphicsItem *parent = 0);

    /**
     * Destructor
     */
    virtual ~LCD();

    /**
     * @return path of the svg widget has
     */
    QString svg() const;

    /**
     * Set svg for the widget
     * @param svg path of svg
     */
    void setSvg(const QString &svg);

    /**
     * Set seven segment number (multiple digits in a group)
     * @param name name of the digit group
     * @param number number string e.g 'h 2.3', numbers and
     * upper/lower case ascii characters are supported
     */
    void setNumber(const QString &name, const QString& number);

    /**
     * Set seven segment digit
     * @param name name of the digit
     * @param digit digit char
     * @param dot whether to show dot
     */
    void setDigit(const QString &name, QChar digit, bool dot = false);

    /**
     * Set group elements on/off
     * @param name name of the element group
     * @param on list of group elements that should be on (rest of the elements are off)
     */
    void setGroup(const QString &name, const QStringList& on);

    /**
     * Get group elements
     * @param name name of the element group
     * @return list of group elements
     */
    QStringList groupItems(const QString &name);

    /**
     * Set svg element off
     * @param name name of the element
     */
    void setItemOff(const QString &name);

    /**
     * Set svg element on
     * @param name name of the element
     */
    void setItemOn(const QString &name);

    /**
     * Set svg element clickable (clicked is emitted)
     * @param name name of the element
     */
    void setItemClickable(const QString &name, bool clickable);

    /**
     * Set text label for the meter
     * @param name name of the element.
     * @param text text for the label.
     */
    void setLabel(const QString &name, const QString &text);

    /**
     * @param name name of the element
     * @return text label for the meter
     */
    QString label(const QString &name) const;

    /**
     * @return lcd as pixmap
     */
    QPixmap toPixmap();

    /**
     * Clear all items
     */
    void clear();

    /**
     * Reimplemented from QGraphicsWidget
     */
    void paint(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget);

signals:
    void clicked(const QString &name);

protected:
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent* event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event);
    virtual QSizeF sizeHint(Qt::SizeHint which, const QSizeF& constraint = QSizeF()) const;

private:
    class Private;
    Private * const d;
};

#endif
