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

#ifndef LANCELOT_ACTION_LIST_VIEW_MODELS_H
#define LANCELOT_ACTION_LIST_VIEW_MODELS_H

#include <lancelot/lancelot_export.h>
#include <lancelot/widgets/PopupMenu.h>

#include <QtCore/QPair>
#include <QtCore/QMimeData>
#include <QtCore/QString>
#include <QtGui/QIcon>
#include <QtCore/QVariant>
// #include <QtGui/QMenu>
#include <QtGui/QAction>

namespace Lancelot
{

/*
 * Notice: Classes in this file are not going to stay ABI nor API compatible,
 * and will possibly be replaced in the future. That is the reason for which
 * the classes are not in d-ptr pattern.
 *
 * TODO: Refactor and convert to d-ptr
 */

/**
 * This class represents data model for ActionListView widgets.
 */
class LANCELOT_EXPORT ActionListViewModel: public QObject {
    Q_OBJECT
public:
    /**
     * Creates a new instance of ActionListViewModel
     */
    ActionListViewModel();

    /**
     * Destroys this ActionListViewModel
     */
    virtual ~ActionListViewModel();

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

/**
 * A basic implementation of ActionListViewModel. Doesn't provide mimeData
 * for drag and drop.
 */
class LANCELOT_EXPORT StandardActionListViewModel: public ActionListViewModel {
    Q_OBJECT
protected:
    /**
     * This class represents an item in the list model.
     */
    class LANCELOT_EXPORT Item {
    public:
        Item(QString itemTitle, QString itemDescription, QIcon itemIcon, QVariant itemData)
          : title(itemTitle), description(itemDescription), icon(itemIcon), data(itemData) {};

        QString title;
        QString description;
        QIcon icon;
        QVariant data;
    };

public:
    /**
     * Creates a new instance of StandardActionListViewModel
     */
    StandardActionListViewModel();

    /**
     * Destroys this StandardActionListViewModel
     */
    virtual ~StandardActionListViewModel();

    L_Override virtual QString title(int index) const;
    L_Override virtual QString description(int index) const;
    L_Override virtual QIcon icon(int index) const;
    L_Override virtual bool isCategory(int index) const;

    L_Override virtual int size() const;

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
    Item & itemAt(int index);

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

    QList < Item > m_items;

private:
    bool m_sendEmits;

};

/**
 * This class implements a model that merges a list
 * of other models. Titles in the sub-models are represented
 * as categories in the merged one.
 */
class LANCELOT_EXPORT MergedActionListViewModel: public ActionListViewModel {
    Q_OBJECT
public:
    /**
     * Creates a new instance of MergedActionListViewModel
     */
    MergedActionListViewModel();

    /**
     * Destroys this MergedActionListViewModel
     */
    virtual ~MergedActionListViewModel();

    /**
     * Adds a model to the list
     * @param icon icon for the model
     * @param title title of the model
     * @param model model to add
     */
    void addModel(QIcon icon, const QString & title, ActionListViewModel * model);

    /**
     * @returns model with the specified index
     * @param index index of the model to return
     */
    ActionListViewModel * modelAt(int index);

    /**
     * Removes a model from the list
     * @param index index of the model to remove
     */
    void removeModel(int index);

    /**
     * @returns number of the models
     */
    int modelCount() const;

    /**
     * @returns mime data for the specified model
     * Reimplement this function if you want to support dragging
     * the models inside the MergedActionListViewModel by
     * dragging their caption items.
     */
    virtual QMimeData * modelMimeData(int index) const;

    /**
     * This function is invoked when a model is dragged and dropped.
     * Reimplement this function if you want to support dragging
     * the models inside the MergedActionListViewModel by
     * dragging their caption items.
     */
    virtual void modelDataDropped(int index, Qt::DropAction action);

    /**
     * @param index index of the model to be dragged
     * @param actions supported drop actions
     * @param defaultAction default drop action
     * Reimplement this function if you want to support dragging
     * the models inside the MergedActionListViewModel by
     * dragging their caption items.
     */
    virtual void setModelDropActions(int index,
            Qt::DropActions & actions, Qt::DropAction & defaultAction);

    /**
     * @returns whether the model has context actions
     * @param index model for which context menu is tested
     * Reimplement this function if you want to support context
     * menus for models when user right clicks the captio item
     */
    virtual bool hasModelContextActions(int index) const;

    /**
     * Sets the actions for the context menu
     * @param index model for which context menu should be set
     * Reimplement this function if you want to support context
     * menus for models when user right clicks the captio item
     */
    virtual void setModelContextActions(int index, Lancelot::PopupMenu * menu);

    /**
     * Activates the context for the specified model
     * @param index model index
     * @param activated action
     * Reimplement this function if you want to support context
     * menus for models when user right clicks the captio item
     */
    virtual void modelContextActivate(int index, QAction * context);

    L_Override virtual QString title(int index) const;
    L_Override virtual QString description(int index) const;
    L_Override virtual QIcon icon(int index) const;
    L_Override virtual QMimeData * mimeData(int index) const;
    L_Override virtual void dataDropped(int index, Qt::DropAction action);
    L_Override virtual void setDropActions(int index, Qt::DropActions & actions, Qt::DropAction & defaultAction);
    L_Override virtual bool isCategory(int index) const;
    L_Override virtual int size() const;
    L_Override virtual bool hasContextActions(int index) const;
    L_Override virtual void setContextActions(int index, Lancelot::PopupMenu * menu);
    L_Override virtual void contextActivate(int index, QAction * context);

    /**
     * @returns whether the empty models are hidden
     */
    bool hideEmptyModels() const;

    /**
     * Sets whether the empty models should be hidden
     */
    void setHideEmptyModels(bool hide);

protected:
    L_Override virtual void activate(int index);

private:
    void toChildCoordinates(int index, int & model, int & modelIndex) const;
    void fromChildCoordinates(int & index, int model, int modelIndex) const;
    QList< ActionListViewModel * > m_models;
    QList< QPair< QString, QIcon > > m_modelsMetadata;
    bool m_hideEmptyModels;

private Q_SLOTS:
    // listen to model changes
    void modelUpdated();
    void modelItemInserted(int index);
    void modelItemDeleted(int index);
    void modelItemAltered(int index);

};

} // namespace Lancelot

#endif /* LANCELOT_ACTION_LIST_VIEW_MODELS_H */

