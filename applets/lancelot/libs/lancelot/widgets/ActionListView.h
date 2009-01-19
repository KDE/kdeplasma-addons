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

#ifndef LANCELOT_ACTION_LIST_VIEW_H
#define LANCELOT_ACTION_LIST_VIEW_H

#include <lancelot/lancelot.h>
#include <lancelot/lancelot_export.h>

#include <KDebug>
#include <QIcon>

#include <lancelot/widgets/CustomListView.h>
#include <lancelot/models/ActionListViewModels.h>

namespace Lancelot
{

class LANCELOT_EXPORT ActionListView : public CustomListView
{
    Q_OBJECT
    L_WIDGET
    L_INCLUDE(lancelot/widgets/ActionListView.h)

public:
    ActionListView(QGraphicsItem * parent = 0);
    explicit ActionListView(ActionListViewModel * model, QGraphicsItem * parent = 0);
    virtual ~ActionListView();

    void setModel(ActionListViewModel * model);
    ActionListViewModel * model() const;

    void setExtenderPosition(ExtenderPosition position);
    ExtenderPosition extenderPosition() const;

    void setItemsGroup(WidgetGroup * group = NULL);
    void setItemsGroupByName(const QString & group);
    WidgetGroup * itemsGroup() const;

    void setCategoriesGroup(WidgetGroup * group = NULL);
    void setCategoriesGroupByName(const QString & group);
    WidgetGroup * categoriesGroup() const;

    bool areCategoriesActivable() const;
    void setCategoriesActivable(bool value);

    void clearSelection();
    void initialSelection();
    int selectedIndex() const;

    void setItemHeight(int height, Qt::SizeHint which);
    void setCategoryHeight(int height, Qt::SizeHint which);

    L_Override virtual void groupUpdated();
    L_Override virtual void keyPressEvent(QKeyEvent * event);

Q_SIGNALS:
    void activated(int index);

private:
    class Private;
    Private * const d;
};

} // namespace Lancelot

#endif /* LANCELOT_ACTION_LIST_VIEW_H */

