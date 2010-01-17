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

#ifndef LANCELOT_STANDARD_ACTION_LIST_MODEL_H
#define LANCELOT_STANDARD_ACTION_LIST_MODEL_H

#include <QtCore/QVariant>

#include <lancelot/lancelot_export.h>
#include <lancelot/models/ActionListModel.h>

namespace Lancelot
{

/**
 * A basic implementation of ActionListModel. Doesn't provide mimeData
 * for drag and drop.
 */
class LANCELOT_EXPORT StandardActionListModel: public ActionListModel {
    Q_OBJECT
protected:
    /**
     * This class represents an item in the list model.
     */
    class LANCELOT_EXPORT Item { //krazy:exclude=dpointer
    public:
        explicit Item(QString itemTitle = QString(),
             QString itemDescription = QString(),
             QIcon itemIcon = QIcon(),
             QVariant itemData = QVariant())
          : title(itemTitle), description(itemDescription), icon(itemIcon), data(itemData) {};

        QString title;
        QString description;
        QIcon icon;
        QVariant data;
    };

public:
    /**
     * Creates a new instance of StandardActionListModel
     */
    StandardActionListModel();

    /**
     * Destroys this StandardActionListModel
     */
    virtual ~StandardActionListModel();

    L_Override QString title(int index) const;
    L_Override QString description(int index) const;
    L_Override QIcon icon(int index) const;
    L_Override bool isCategory(int index) const;

    L_Override int size() const;

    /**
     * Adds a new item into the model
     * @param item new item
     */
    void add(const Item & item);

    /**
     * Adds a new item into the model
     * @param title the title for the new item
     * @param description the description of the new item
     * @param icon the icon for the new item
     * @param data data for the new item. Not shown to user
     */
    void add(const QString & title, const QString & description, QIcon icon, const QVariant & data);

    /**
     * Adds a new item into the model
     * @param where where to insert
     * @param item new item
     */
    void insert(int where, const Item & item);

    /**
     * Adds a new item into the model
     * @param where where to insert
     * @param title the title for the new item
     * @param description the description of the new item
     * @param icon the icon for the new item
     * @param data data for the new item. Not shown to user
     */
    void insert(int where, const QString & title, const QString & description, QIcon icon, const QVariant & data);

    /**
     * Replaces existing item at specified index with a new one
     * @param index index of the item to be replaced
     * @param item new item
     */
    void set(int index, const Item & item);

    /**
     * Replaces existing item at specified index with a new one
     * @param index index of the item to be replaced
     * @param title the title for the new item
     * @param description the description of the new item
     * @param icon the icon for the new item
     * @param data data for the new item. Not shown to user
     */
    void set(int index, const QString & title, const QString & description, QIcon icon, const QVariant & data);

    /**
     * Removes an item
     * @param index index of the item to remove
     */
    void removeAt(int index);

    /**
     * @returns the specified item
     * @param index index of the item to return
     */
    const Item & itemAt(int index) const;

    /**
     * Clears all items
     */
    void clear();

protected:
    /**
     * Sets whether emit signals should be inhibited
     * @param value value
     */
    void setEmitInhibited(bool value);

    /**
     * @returns whether emit signals are inhibited
     */
    bool emitInhibited() const;


private:
    class Private;
    Private * const d;

};

} // namespace Lancelot

#endif /* LANCELOT_STANDARD_ACTION_LIST_MODEL_H */

