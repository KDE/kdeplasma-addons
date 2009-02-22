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

#ifndef LANCELOT_PASSAGEWAY_VIEW_H
#define LANCELOT_PASSAGEWAY_VIEW_H

#include <lancelot/lancelot.h>
#include <lancelot/lancelot_export.h>

#include <lancelot/widgets/Widget.h>
#include <QtGui/QIcon>

#include <lancelot/widgets/Panel.h>
#include <lancelot/widgets/ActionListView.h>
#include <lancelot/widgets/PassagewayView.h>
#include <lancelot/layouts/ColumnLayout.h>

#include <lancelot/models/ActionTreeModel.h>
#include <lancelot/models/ActionListModel.h>

#include <lancelot/layouts/NodeLayout.h>
#include <QtGui/QGraphicsLinearLayout>

namespace Lancelot
{

/**
 * Class for non-click tree-browsing with a list of fast-access items
 * Entrance - the list of fast-access items
 * Atlas - all items
 */
class LANCELOT_EXPORT PassagewayView : public Lancelot::Panel
{
    Q_OBJECT

    L_WIDGET
    L_INCLUDE(lancelot/widgets/PassagewayView.h)

public:
    PassagewayView(QGraphicsItem * parent = 0);
    explicit PassagewayView(ActionTreeModel * entranceModel,
            ActionTreeModel * atlasModel = 0, QGraphicsItem * parent = 0);
    virtual ~PassagewayView();

    // Entrance
    void setEntranceModel(ActionTreeModel * model);
    void setEntranceTitle(const QString & title);
    void setEntranceIcon(QIcon icon);

    // Atlas
    void setAtlasModel(ActionTreeModel * model);
    void setAtlasTitle(const QString & title);
    void setAtlasIcon(QIcon icon);

    void setActivationMethod(ActivationMethod value);
    ActivationMethod activationMethod() const;

    void setColumnLimit(int limit);
    void reset();

    void clearSelection();

    L_Override void setGroup(WidgetGroup * group = NULL);
    L_Override void groupUpdated();
    L_Override void keyPressEvent(QKeyEvent * event);

protected Q_SLOTS:
    virtual void listItemActivated(int itemIndex, int listIndex = -1);
    virtual void pathButtonActivated();

private:
    class Private;
    Private * const d;
};

} // namespace Lancelot

#endif /* LANCELOT_PASSAGEWAY_VIEW_H */

