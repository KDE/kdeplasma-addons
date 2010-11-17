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

#ifndef LANCELOT_MERGED_ACTION_LIST_MODEL_H
#define LANCELOT_MERGED_ACTION_LIST_MODEL_H

#include <QtCore/QPair>

#include <lancelot/lancelot_export.h>
#include <lancelot/models/ActionListModel.h>

namespace Lancelot
{

/**
 * This class implements a model that merges a list
 * of other models. Titles in the sub-models are represented
 * as categories in the merged one.
 */
class LANCELOT_EXPORT MergedActionListModel: public ActionListModel {
    Q_OBJECT
public:
    /**
     * Creates a new instance of MergedActionListModel
     */
    MergedActionListModel();

    /**
     * Destroys this MergedActionListModel
     */
    virtual ~MergedActionListModel();

    /**
     * Adds a model to the list
     * @param icon icon for the model
     * @param title title of the model
     * @param model model to add
     */
    void addModel(QIcon icon, const QString & title, ActionListModel * model);

    /**
     * Adds a model to the list. Title and icon is
     * loaded from model
     * @param model model to add
     */
    void addModel(ActionListModel * model);

    /**
     * @returns model with the specified index
     * @param index index of the model to return
     */
    ActionListModel * modelAt(int index);

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
     * the models inside the MergedActionListModel by
     * dragging their caption items.
     */
    virtual QMimeData * modelMimeData(int index) const;

    /**
     * This function is invoked when a model is dragged and dropped.
     * Reimplement this function if you want to support dragging
     * the models inside the MergedActionListModel by
     * dragging their caption items.
     */
    virtual void modelDataDropped(int index, Qt::DropAction action);

    /**
     * @param index index of the model to be dragged
     * @param actions supported drop actions
     * @param defaultAction default drop action
     * Reimplement this function if you want to support dragging
     * the models inside the MergedActionListModel by
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

    L_Override QString title(int index) const;
    L_Override QString description(int index) const;
    L_Override QIcon icon(int index) const;
    L_Override QMimeData * mimeData(int index) const;
    L_Override void dataDragFinished(int index, Qt::DropAction action);
    L_Override void setDropActions(int index, Qt::DropActions & actions, Qt::DropAction & defaultAction);
    L_Override bool isCategory(int index) const;
    L_Override int size() const;
    L_Override bool hasContextActions(int index) const;
    L_Override void setContextActions(int index, Lancelot::PopupMenu * menu);
    L_Override void contextActivate(int index, QAction * context);

    L_Override bool dataDropAvailable(int where, const QMimeData * mimeData);
    L_Override void dataDropped(int where, const QMimeData * mimeData);

    /**
     * @returns whether the empty models are hidden
     */
    bool hideEmptyModels() const;

    /**
     * Sets whether the empty models should be hidden
     */
    void setHideEmptyModels(bool hide);

    /**
     * @returns whether the model titles are shown
     */
    bool showModelTitles() const;

    /**
     * Sets whether model titles are shown
     */
    void setShowModelTitles(bool show);

protected:
    L_Override void activate(int index);

protected Q_SLOTS:
    // listen to model changes
    void modelUpdated();
    void modelItemInserted(int index);
    void modelItemDeleted(int index);
    void modelItemAltered(int index);

private:
    class Private;
    Private * const d;

};

} // namespace Lancelot

#endif /* LANCELOT_MERGED_ACTION_LIST_MODEL_H */

