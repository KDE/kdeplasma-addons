/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free 
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef PASSAGEWAYVIEW_H_
#define PASSAGEWAYVIEW_H_

#include "Widget.h"
#include <KIcon>

#include "Panel.h"
#include "ActionListView.h"
#include "ActionListViewModels.h"
#include "PassagewayView.h"
#include "PassagewayViewModels.h"
#include <plasma/widgets/nodelayout.h>

namespace Lancelot
{

/**
 * Class for non-click tree-browsing with a list of fast-access items
 * Entrance - the list of fast-access items
 * Atlas - all items
 */
class PassagewayView : public Lancelot::Panel
{
public:
	PassagewayView(QString name, ActionListViewModel * entranceModel = 0, 
	        PassagewayViewModel * atlasModel = 0, QGraphicsItem * parent = 0);
	virtual ~PassagewayView();

	// Entrance
    void setEntranceModel(ActionListViewModel * model);
    void setEntranceTitle(const QString & title);
    void setEntranceIcon(KIcon * icon);
    
    ActionListViewModel * entranceModel();
    QString entranceTitle();
    KIcon * entranceIcon();
    
	// Atlas
	void setAtlasModel(PassagewayViewModel * model);
	void setAtlasTitle(const QString & title);
	void setAtlasIcon(KIcon * icon);
    
    PassagewayViewModel * atlasModel();
    QString atlasTitle();
    KIcon * atlasIcon();

private:
    
    Plasma::NodeLayout * m_layout;
    
    // Entrance
    ActionListViewModel * m_entranceModel;
    QString m_entranceTitle;
    KIcon * m_entranceIcon;
    
    // Atlas
    PassagewayViewModel * m_atlasModel;
    QString m_atlasTitle;
    KIcon * m_atlasIcon;
    
    // Other
    class Milestone {
        QString title;
        KIcon * icon;
        ActionListViewModel * model;
    };
    QList < Milestone > m_path;
    
    QList < ActionListView * > m_listViews;
};

}

#endif /*PASSAGEWAYVIEW_H_*/
