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

#ifndef LANCELOT_ACTION_LIST_MODEL_H
#define LANCELOT_ACTION_LIST_MODEL_H

#include <lancelot/lancelot_export.h>
#include <lancelot/widgets/PopupMenu.h>

#include <QtCore/QMimeData>
#include <QtCore/QString>
#include <QtGui/QIcon>
#include <QtGui/QAction>

namespace Lancelot
{

/**
 * This class represents a list data model.
 */
class LANCELOT_EXPORT ActionListModel: public QObject {
    Q_OBJECT
public:
    /**
     * Creates a new instance of ActionListModel
     */
    ActionListModel();

    /**
     * Destroys this ActionListModel
     */
    virtual ~ActionListModel();

    /**
     * @param index index of the item
     * @returns title for the specified item
     */
    virtual QString title(int index) const = 0;

    /**
     * @param index index of the item
     * @returns description of the specified item
     */
    virtual QString description(int index) const;

    /**
     * @param index index of the item
     * @returns icon for the specified item
     */
    virtual QIcon icon(int index) const;

    /**
     * @param index index of the item
     * @returns mime data for the specified item
     */
    virtual QMimeData * mimeData(int index) const;

    /**
     * This function is invoked when a data is dropped
     * @param index index of the dragged item
     * @param action invoked drop action
     */
    virtual void dataDropped(int index, Qt::DropAction action);

    /**
     * @param index index of the item to be dragged
     * @param actions supported drop actions
     * @param defaultAction default drop action
     */
    virtual void setDropActions(int index, Qt::DropActions & actions, Qt::DropAction & defaultAction);

    /**
     * @param index index of the item
     * @returns whether the item represents a category
     */
    virtual bool isCategory(int index) const;

    /**
     * @returns the number of items in model
     */
    virtual int size() const = 0;

    /**
     * @returns whether the specified item has context actions
     * @param index index of the item
     */
    virtual bool hasContextActions(int index) const;

    /**
     * Adds actions ofr the specifies item to menu
     * @param index index of the item
     * @param menu menu to add the actions to
     */
    virtual void setContextActions(int index, Lancelot::PopupMenu * menu);

    /**
     * Method for handling context menu actions
     * @param index of the activated item
     * @param context index of the context action
     */
    virtual void contextActivate(int index, QAction * context);

    virtual QString selfTitle() const;
    virtual QIcon selfIcon() const;
    virtual QMimeData * selfMimeData() const;

public Q_SLOTS:
    /**
     * Activates the specified element
     * @param index of the element that should be activated
     */
    void activated(int index);

Q_SIGNALS:
    /**
     * This signal is emitted when an item is activated
     * @param index of the activated element
     */
    void itemActivated(int index);

    /**
     * This signal is emitted when the model is updated and the update
     * is too complex to explain using itemInserted, itemDeleted and
     * itemAltered methods
     */
    void updated();

    /**
     * This signal is emitted when an item is inserted into the model
     * @param index place where the new item is inserted
     */
    void itemInserted(int index);

    /**
     * This signal is emitted when an item is deleted from the model
     * @param index index of the deleted item
     */
    void itemDeleted(int index);

    /**
     * This signal is emitted when an item is altered
     * @param index index of the altered item
     */
    void itemAltered(int index);

protected:
    /**
     * Models should reimplement this function. It is invoked when
     * an item is activated, before the itemActivated signal is emitted
     * @param index of the item that is activated
     */
    virtual void activate(int index);
};

} // namespace Lancelot

#endif /* LANCELOT_ACTION_LIST_MODEL_H */

