/*
    QAction * chosenAction;
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

#include <QtGui/QIcon>

#include <lancelot/widgets/CustomListView.h>
#include <lancelot/models/ActionListModel.h>

namespace Lancelot
{

class LANCELOT_EXPORT ActionListView : public CustomListView
{
    Q_OBJECT
    L_WIDGET
    L_INCLUDE(lancelot/widgets/ActionListView.h)

public:
    ActionListView(QGraphicsItem * parent = 0);
    explicit ActionListView(ActionListModel * model, QGraphicsItem * parent = 0);
    virtual ~ActionListView();

    void setModel(ActionListModel * model);
    ActionListModel * model() const;

    void setExtenderPosition(ExtenderPosition position);
    ExtenderPosition extenderPosition() const;

    void setItemsGroup(WidgetGroup * group = NULL);
    void setItemsGroupByName(const QString & group);
    WidgetGroup * itemsGroup() const;

    void setCategoriesGroup(WidgetGroup * group = NULL);
    void setCategoriesGroupByName(const QString & group);
    WidgetGroup * categoriesGroup() const;

    void setCategoriesActivable(bool value);
    bool areCategoriesActivable() const;

    int selectedIndex() const;

    void setItemHeight(int height, Qt::SizeHint which);
    void setCategoryHeight(int height, Qt::SizeHint which);
    void setItemIconSize(QSize size);
    void setCategoryIconSize(QSize size);

    void setShowsExtendersOutside(bool value);
    bool showsExtendersOutside() const;

    L_Override void groupUpdated();
    L_Override void keyPressEvent(QKeyEvent * event);

public Q_SLOTS:
    void clearSelection();
    void initialSelection();

Q_SIGNALS:
    void activated(int index);

private:
    class Private;
    Private * const d;
};

} // namespace Lancelot

#endif /* LANCELOT_ACTION_LIST_VIEW_H */

