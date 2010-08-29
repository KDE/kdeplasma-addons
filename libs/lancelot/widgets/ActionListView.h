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

#ifndef LANCELOT_ACTION_LIST_VIEW_H
#define LANCELOT_ACTION_LIST_VIEW_H

#include <QtGui/QIcon>

#include <lancelot/lancelot.h>
#include <lancelot/lancelot_export.h>
#include <lancelot/models/ActionListModel.h>
#include <lancelot/widgets/CustomListView.h>

namespace Lancelot
{

class LANCELOT_EXPORT ActionListView : public Lancelot::CustomListView
{
    Q_OBJECT

    Q_PROPERTY ( int extenderPosition READ extenderPosition WRITE setExtenderPosition )
    Q_PROPERTY ( bool categoriesActivable READ areCategoriesActivable WRITE setCategoriesActivable )
    Q_PROPERTY ( bool showsExtendersOutside READ showsExtendersOutside WRITE setShowsExtendersOutside )
    Q_PROPERTY ( int selectedIndex READ selectedIndex)

    // Q_PROPERTY ( int activationMethod READ activationMethod WRITE setActivationMethod )

    // @puck L_WIDGET
    // @puck L_INCLUDE(lancelot/widgets/ActionListView.h)

public:
    enum ItemDisplayMode {
        Standard = 0,
        DescriptionFirst = 1,
        SingleLineNameFirst = 2,
        SingleLineDescriptionFirst = 3,
        OnlyName = 4,
        OnlyDescription = 5
    };

    ActionListView(QGraphicsItem * parent = 0);
    explicit ActionListView(ActionListModel * model, QGraphicsItem * parent = 0);
    virtual ~ActionListView();

    void setModel(ActionListModel * model);
    ActionListModel * model() const;

    void setExtenderPosition(int position);
    int extenderPosition() const;

    void setDisplayMode(ItemDisplayMode mode);
    ItemDisplayMode displayMode() const;
    // void setActivationMethod(int method);
    // int activationMethod() const;

    void setItemsGroup(Group * group = NULL);
    void setItemsGroupByName(const QString & group);
    Group * itemsGroup() const;

    void setCategoriesGroup(Group * group = NULL);
    void setCategoriesGroupByName(const QString & group);
    Group * categoriesGroup() const;

    void setCategoriesActivable(bool value);
    bool areCategoriesActivable() const;

    int selectedIndex() const;

    void setItemHeight(int height, Qt::SizeHint which);
    void setCategoryHeight(int height, Qt::SizeHint which);

    int itemHeight(Qt::SizeHint which) const;
    int categoryHeight(Qt::SizeHint which) const;

    void setItemIconSize(QSize size);
    void setCategoryIconSize(QSize size);

    QSize itemIconSize() const;
    QSize categoryIconSize() const;

    void setShowsExtendersOutside(bool value);
    bool showsExtendersOutside() const;

    L_Override void keyPressEvent(QKeyEvent * event);

public Q_SLOTS:
    void clearSelection();
    void initialSelection();

    void hide();
    void show();

protected:
    L_Override bool sceneEventFilter(QGraphicsItem * watched,
            QEvent * event);
    L_Override bool sceneEvent(QEvent * event);
    L_Override QSizeF sizeHint(Qt::SizeHint which,
            const QSizeF & constraint = QSizeF()) const;
    L_Override void resizeEvent(
            QGraphicsSceneResizeEvent * event);
    L_Override void hoverLeaveEvent(
            QGraphicsSceneHoverEvent * event);

Q_SIGNALS:
    void activated(int index);

private:
    class Private;
    Private * const d;
};

} // namespace Lancelot

#endif /* LANCELOT_ACTION_LIST_VIEW_H */

