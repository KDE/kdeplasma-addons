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

#ifndef LANCELOT_CARD_LAYOUT_H
#define LANCELOT_CARD_LAYOUT_H

#include <QtCore/QMap>
#include <QtCore/QObject>

#include <QtGui/QGraphicsLayout>
#include <QtGui/QGraphicsLayoutItem>
#include <QtGui/QGraphicsWidget>

#include <cmath>

#include <lancelot/lancelot_export.h>

namespace Lancelot
{

/**
 * CardLayout positions its children like a stack of cards.
 *
 * Every child takes the whole area available to CardLayout, but
 * only one child is shown at a time.
 *
 * @note Only QGraphicsWidget based items can be hidden.
 *
 * @author Ivan Cukic
 *
 */

class LANCELOT_EXPORT CardLayout : public QObject, public QGraphicsLayout
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsLayout)

public:

    /**
     * Creates a new Lancelot::CardLayout
     * @param parent parent layout item
     */
    explicit CardLayout(QGraphicsLayoutItem * parent = 0);

    /**
     * Destroys this Lancelot::CardLayout
     */
    virtual ~CardLayout();

    /**
     * Adds a QGraphicsLayoutItem to this layout.
     * @note Items added using this method will be always shown
     * @param item item to add
     */
    void addItem(QGraphicsLayoutItem * item);

    /**
     * Adds a QGraphicsWidget.
     * @param item item to add
     * @param id used to identify the item
     */
    void addItem(QGraphicsWidget * widget, const QString & id);

public Q_SLOTS:
    /**
     * Shows the item specified by id, and hides all other
     * items
     */
    void showCard(const QString & id);

    /**
     * Hides all items that can be hidden
     */
    void hideAll();

public:
    L_Override void setGeometry(const QRectF & rect);
    L_Override int count() const;
    L_Override QGraphicsLayoutItem * itemAt(int i) const;
    L_Override void removeAt(int index);
    L_Override QSizeF sizeHint(Qt::SizeHint which,
            const QSizeF & constraint = QSizeF()) const;

private:
    class Private;
    Private * const d;
};

} // namespace Lancelot

#endif /* LANCELOT_CARD_LAYOUT_H */

