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

class CaptionedList: public Panel {
public:
    CaptionedList(QString name, QIcon * icon, QString title = QString(), QGraphicsItem * parent = 0)
        : Panel(name, icon, title, parent), m_list(NULL)
    {
        m_list = new ActionListView(name + "::List", 0, this);
        setWidget(m_list);
    }
    
    ActionListView * list()
    {
        return m_list;
    }

private:
    ActionListView * m_list;
    
};

PassagewayView::PassagewayView(QString name, ActionListViewModel * entranceModel,
    PassagewayViewModel * atlasModel, QGraphicsItem * parent)
    : Panel(name, parent)
{
    setLayout(m_layout = new Plasma::NodeLayout());

    for (int i = 0; i < 2; ++i) {
        m_list.append(new CaptionedList(name + "::" + QString::number(i), NULL, "", this));
        m_list.last()->list()->setExtenderPosition(((i == 0)?(Lancelot::ExtenderButton::Left):(Lancelot::ExtenderButton::Right)));

    }

    m_layout->addItem(
        m_list.at(0),
        Plasma::NodeLayout::NodeCoordinate(0, 0, 32, 0),
        Plasma::NodeLayout::NodeCoordinate(0.5, 1.0, 0, 0)
    );
    m_layout->addItem(
        m_list.at(1),
        Plasma::NodeLayout::NodeCoordinate(0.5, 0, 0, 0),
        Plasma::NodeLayout::NodeCoordinate(1.0, 1.0, 0, 0)
    );

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
void PassagewayView::setEntranceModel(ActionListViewModel * model)
{
    m_list.at(0)->list()->setModel(model);
}

void PassagewayView::setEntranceTitle(const QString & title)
{
    m_list.at(0)->setTitle(title);
}

void PassagewayView::setEntranceIcon(KIcon * icon)
{
    m_list.at(0)->setIcon(icon);
}

// ActionListViewModel * PassagewayView::entranceModel() { return m_entranceModel; }
// QString PassagewayView::entranceTitle()               { return m_entranceTitle; }
// KIcon * PassagewayView::entranceIcon()                { return m_entranceIcon; }

// Atlas
void PassagewayView::setAtlasModel(PassagewayViewModel * model)
{
    m_list.at(1)->list()->setModel(model);
}

void PassagewayView::setAtlasTitle(const QString & title)
{
    m_list.at(1)->setTitle(title);
}

void PassagewayView::setAtlasIcon(KIcon * icon)
{
    m_list.at(1)->setIcon(icon);
}

// PassagewayViewModel * PassagewayView::atlasModel() { return m_atlasModel; }
// QString PassagewayView::atlasTitle()               { return m_atlasTitle; }
// KIcon * PassagewayView::atlasIcon()                { return m_atlasIcon; }


}
