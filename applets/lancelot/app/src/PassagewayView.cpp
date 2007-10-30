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

#include "PassagewayView.h"

namespace Lancelot
{

PassagewayView::PassagewayView(QString name, ActionListViewModel * entranceModel,
    PassagewayViewModel * atlasModel, QGraphicsItem * parent)
    : Panel(name, parent), m_entranceModel(entranceModel), m_atlasModel(atlasModel)
{
    setLayout(m_layout = new Plasma::NodeLayout());

    for (int i = 0; i < 3; ++i) {
        m_listViews.append(new ActionListView(name + "::List_" + QString::number(i), NULL, this));
        m_listViewPanels.append(new Panel(name + "::List_" + QString::number(i), this));
        m_listViewPanels.last()->setWidget(m_listViews.last());
    }

    m_layout->addItem(
        m_listViewPanels.at(0),
        Plasma::NodeLayout::NodeCoordinate(0, 0, 32, 0),
        Plasma::NodeLayout::NodeCoordinate(0.5, 1.0, 0, 0)
    );
    m_layout->addItem(
        m_listViewPanels.at(1),
        Plasma::NodeLayout::NodeCoordinate(0.5, 0, 0, 0),
        Plasma::NodeLayout::NodeCoordinate(1.0, 1.0, 0, 0)
    );

    m_listViewPanels.at(0)->setTitle("Entrance");
    m_listViewPanels.at(1)->setTitle("Atlas");

    m_path.append(Step());
    m_path.last().title = "Entrance";
    m_path.last().model = entranceModel;

    m_path.append(Step());
    m_path.last().title = "Atlas";
    m_path.last().model = atlasModel;

}

PassagewayView::~PassagewayView()
{
}

// Entrance
void PassagewayView::setEntranceModel(ActionListViewModel * model) { m_entranceModel = model; }
void PassagewayView::setEntranceTitle(const QString & title)       { m_entranceTitle = title; }
void PassagewayView::setEntranceIcon(KIcon * icon)                 { m_entranceIcon = icon; }

ActionListViewModel * PassagewayView::entranceModel() { return m_entranceModel; }
QString PassagewayView::entranceTitle()               { return m_entranceTitle; }
KIcon * PassagewayView::entranceIcon()                { return m_entranceIcon; }

// Atlas
void PassagewayView::setAtlasModel(PassagewayViewModel * model) { m_atlasModel = model; }
void PassagewayView::setAtlasTitle(const QString & title)       { m_atlasTitle = title; }
void PassagewayView::setAtlasIcon(KIcon * icon)                 { m_atlasIcon = icon; }

PassagewayViewModel * PassagewayView::atlasModel() { return m_atlasModel; }
QString PassagewayView::atlasTitle()               { return m_atlasTitle; }
KIcon * PassagewayView::atlasIcon()                { return m_atlasIcon; }


}
