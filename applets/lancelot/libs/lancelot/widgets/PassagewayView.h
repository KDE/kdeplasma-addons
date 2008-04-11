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

// TODO: Convert to dptr

#ifndef PASSAGEWAYVIEW_H_
#define PASSAGEWAYVIEW_H_

#include "../lancelot.h"
#include "../lancelot_export.h"

#include "Widget.h"
#include <KIcon>

#include "Panel.h"
#include "ActionListView.h"
#include "PassagewayView.h"
#include "../layouts/ColumnLayout.h"

#include "../models/PassagewayViewModels.h"
#include "../models/ActionListViewModels.h"

#include <plasma/layouts/nodelayout.h>
#include <plasma/layouts/boxlayout.h>
#include <plasma/layouts/layoutanimator.h>

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
public:
    PassagewayView(QString name, PassagewayViewModel * entranceModel = 0,
            PassagewayViewModel * atlasModel = 0, QGraphicsItem * parent = 0);
    virtual ~PassagewayView();

    // Entrance
    void setEntranceModel(PassagewayViewModel * model);
    void setEntranceTitle(const QString & title);
    void setEntranceIcon(KIcon * icon);

    // Atlas
    void setAtlasModel(PassagewayViewModel * model);
    void setAtlasTitle(const QString & title);
    void setAtlasIcon(KIcon * icon);

protected Q_SLOTS:
    virtual void listItemActivated(int index);
    virtual void pathButtonActivated();

private:
    class Private;
    Private * d;
};

}

#endif /*PASSAGEWAYVIEW_H_*/
