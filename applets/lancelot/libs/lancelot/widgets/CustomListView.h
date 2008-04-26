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

#ifndef LANCELOT_CUSTOM_LIST_VIEW_H_
#define LANCELOT_CUSTOM_LIST_VIEW_H_

#include <lancelot/lancelot_export.h>

#include <QtGui>
#include <QtCore>
#include <KDebug>
#include <KIcon>

#include <lancelot/widgets/Widget.h>
#include <lancelot/widgets/ExtenderButton.h>
#include <lancelot/widgets/ActionListViewModels.h>

namespace Lancelot
{

class LANCELOT_EXPORT CustomListViewItem: public Widget
{
    Q_OBJECT
public:
    virtual void setItemSelected(bool selected) = 0;
    virtual bool isItemSelected() const = 0;
    virtual ~CustomListViewItem() {};

public Q_SIGNALS:
    void itemActivated();

};

class LANCELOT_EXPORT CustomListViewItemFactory
{
public:
    virtual CustomListViewItem * itemForIndex(int index) = 0;
    virtual void freeItem(CustomListViewItem * item) = 0;
    virtual ~CustomListViewItemFactory() {};
};

class LANCELOT_EXPORT CustomListView: public Widget
{
    Q_OBJECT

public:
    CustomListView(QString name, ActionListViewModel * model = 0, QGraphicsItem * parent = 0);
    virtual ~CustomListView();

    void setModel(ActionListViewModel * model);
    ActionListViewModel * model() const;

    void setItemFactory(CustomListViewItemFactory * factory);
    CustomListViewItemFactory * itemFactory() const;

    void updateGeometry();

    void wheelEvent ( QGraphicsSceneWheelEvent * event );

Q_SIGNALS:
    void activated(int index);

protected slots:
    void scrollTimer();
    void itemActivated(int index);

    void modelUpdated();
    void modelItemInserted(int index);
    void modelItemDeleted(int index);
    void modelItemAltered(int index);

private:
    class Private;
    Private * d;
};

} // namespace Lancelot

#endif /* LANCELOT_CUSTOM_LIST_VIEW_H_ */

