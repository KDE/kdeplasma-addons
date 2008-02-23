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

#ifndef PASSAGEWAYVIEW_H_
#define PASSAGEWAYVIEW_H_

#include "Widget.h"
#include <KIcon>

#include "Panel.h"
#include "ActionListView.h"
#include "ActionListViewModels.h"
#include "PassagewayView.h"
#include "PassagewayViewModels.h"

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
class PassagewayView : public Lancelot::Panel
{
    Q_OBJECT
private:
    class Step;
    
public:
	PassagewayView(QString name, ActionListViewModel * entranceModel = 0,
	        PassagewayViewModel * atlasModel = 0, QGraphicsItem * parent = 0);
	virtual ~PassagewayView();

	// Entrance
    void setEntranceModel(ActionListViewModel * model);
    void setEntranceTitle(const QString & title);
    void setEntranceIcon(KIcon * icon);

	// Atlas
	void setAtlasModel(PassagewayViewModel * model);
	void setAtlasTitle(const QString & title);
	void setAtlasIcon(KIcon * icon);

private Q_SLOTS:
    void listItemActivated(int index);
    
private:
    void back(int steps);
    void next(Step newStep);
	
private:
    Plasma::NodeLayout * m_layout;

    // Other
    class Step {
    public:
        Step(QString t, KIcon * i, ActionListViewModel * m)
            : title(t), icon(i), model(m) {};
        QString title;
        KIcon * icon;
        ActionListViewModel * model;
    };
    
    QList < Step * > m_path;
    
    QList < ExtenderButton * > m_buttons;
    Plasma::BoxLayout * m_buttonsLayout;
    Plasma::LayoutAnimator * m_buttonsAnimator;
    
    QList < ActionListView * > m_lists;
    Plasma::BoxLayout * m_listsLayout;
    Plasma::LayoutAnimator * m_listsAnimator;
    
};

}

#endif /*PASSAGEWAYVIEW_H_*/
