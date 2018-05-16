/***************************************************************************
 *   Copyright (C) 2015 Marco Martin <mart@kde.org>                        *
 *   Copyright (C) 2016 David Edmundson <davidedmundson@kde.org>           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef GROUPINGCONTAINMENT_H
#define GROUPINGCONTAINMENT_H

#include <Plasma/Containment>

class QQuickItem;

class GroupingContainment : public Plasma::Containment
{
    Q_OBJECT
public:
    explicit GroupingContainment( QObject *parent, const QVariantList &args );
    ~GroupingContainment() override;

    void init() override;

    //Creates an applet
    Q_INVOKABLE void newTask(const QString &task);

    //cleans all instances of a given applet
    void cleanupTask(const QString &task);

    /**
     * Given an AppletInterface pointer, shows a proper context menu for it
     */
    Q_INVOKABLE void showPlasmoidMenu(QQuickItem *appletInterface, int x, int y);

    /**
     * Find out global coordinates for a popup given local MouseArea
     * coordinates
     */
    Q_INVOKABLE QPointF popupPosition(QQuickItem* visualParent, int x, int y);

    /**
     * Reparent the item "before" with the same parent as the item "after",
     * then restack it before it, using QQuickITem::stackBefore.
     * used to quickly reorder icons in the systray (or hidden popup)
     * @see QQuickITem::stackBefore
     */
    Q_INVOKABLE void reorderItemBefore(QQuickItem* before, QQuickItem* after);

    /**
     * Reparent the item "after" with the same parent as the item "before",
     * then restack it after it, using QQuickITem::stackAfter.
     * used to quickly reorder icons in the systray (or hidden popup)
     * @see QQuickITem::stackAfter
     */
    Q_INVOKABLE void reorderItemAfter(QQuickItem* after, QQuickItem* before);
};

#endif
