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
    : Panel(name, parent), m_layout(NULL), m_buttonsLayout(NULL), m_listsLayout(NULL)
{
    setLayout(m_layout = new Plasma::NodeLayout());

    m_layout->addItem(
        m_buttonsLayout = new Plasma::BoxLayout(Plasma::BoxLayout::LeftToRight),
        Plasma::NodeLayout::NodeCoordinate(0, 0, 0, 0),
        Plasma::NodeLayout::NodeCoordinate(1, 0, 0, 32)
    );
    m_buttonsLayout->setMargin(0);

    m_buttonsAnimator = new Plasma::LayoutAnimator(this);
    m_buttonsAnimator->setAutoDeleteOnRemoval(true);
    m_buttonsAnimator->setEffect(Plasma::LayoutAnimator::InsertedState, Plasma::LayoutAnimator::FadeEffect);
    m_buttonsAnimator->setEffect(Plasma::LayoutAnimator::StandardState, Plasma::LayoutAnimator::MoveEffect);
    m_buttonsAnimator->setEffect(Plasma::LayoutAnimator::RemovedState,  Plasma::LayoutAnimator::FadeEffect);
    m_buttonsAnimator->setTimeLine(new QTimeLine(300, this));

    m_buttonsLayout->setAnimator(m_buttonsAnimator);

    m_layout->addItem(
        m_listsLayout = new Plasma::BoxLayout(Plasma::BoxLayout::LeftToRight),
        Plasma::NodeLayout::NodeCoordinate(0, 0, 0, 32),
        Plasma::NodeLayout::NodeCoordinate(1, 1, 0, 0)
    );
    m_listsLayout->setMargin(0);

    m_listsAnimator = new Plasma::LayoutAnimator(this);
    m_listsAnimator->setAutoDeleteOnRemoval(false);
    m_listsAnimator->setEffect(Plasma::LayoutAnimator::InsertedState, Plasma::LayoutAnimator::FadeEffect);
    m_listsAnimator->setEffect(Plasma::LayoutAnimator::StandardState, Plasma::LayoutAnimator::MoveEffect);
    m_listsAnimator->setEffect(Plasma::LayoutAnimator::RemovedState,  Plasma::LayoutAnimator::FadeEffect);
    m_listsAnimator->setTimeLine(new QTimeLine(300, this));

    m_buttonsLayout->setAnimator(m_buttonsAnimator);

    next(Step("", NULL, entranceModel));
    next(Step("", NULL, atlasModel));
}

void PassagewayView::listItemActivated(int index)
{
    next(Step("Test", new KIcon("lancelot"), NULL));
}

PassagewayView::~PassagewayView()
{
    delete m_buttonsLayout;
    delete m_listsLayout;
    delete m_layout;

    foreach(ExtenderButton * button, m_buttons) {
        delete button;
    }
    foreach(ActionListView * list, m_lists) {
        delete list;
    }
    foreach(Step * step, m_path) {
        delete step;
    }
}

void PassagewayView::back(int steps)
{

}

void PassagewayView::next(Step newStep)
{
    ExtenderButton * button = new ExtenderButton(m_name + "::button", newStep.icon, newStep.title, "", this);
    ActionListView * list   = new ActionListView(m_name + "::list", newStep.model, this);

    button->setIconSize(QSize(24, 24));
    button->setAlignment(Qt::AlignLeft);
    
    m_buttons.append(button);
    m_lists.append(list);
    m_path.append(new Step(newStep));

    m_buttonsLayout->addItem(button);
    m_listsLayout->addItem(list);
    
    if (m_lists.size() > 2) {
        m_listsLayout->takeAt(0);
    }
    
    connect(
        list, SIGNAL(activated(int)),
        this, SLOT(listItemActivated(int))
    );
    
}

// Entrance
void PassagewayView::setEntranceModel(ActionListViewModel * model)
{
    if (m_lists.size() < 2) return;
    m_lists.at(0)->setModel(model);
}

void PassagewayView::setEntranceTitle(const QString & title)
{
    if (m_lists.size() < 2) return;
    m_path.at(0)->title = title;
    m_buttons.at(0)->setTitle(title);
}

void PassagewayView::setEntranceIcon(KIcon * icon)
{
    if (m_lists.size() < 2) return;
    m_path.at(0)->icon = icon;
    m_buttons.at(0)->setIcon(icon);
}

// Atlas
void PassagewayView::setAtlasModel(PassagewayViewModel * model)
{
    if (m_lists.size() < 2) return;
    m_lists.at(1)->setModel(model);
}

void PassagewayView::setAtlasTitle(const QString & title)
{
    if (m_lists.size() < 2) return;
    m_path.at(1)->title = title;
    m_buttons.at(1)->setTitle(title);
}

void PassagewayView::setAtlasIcon(KIcon * icon)
{
    if (m_lists.size() < 2) return;
    m_path.at(1)->icon = icon;
    m_buttons.at(1)->setIcon(icon);
}

}
