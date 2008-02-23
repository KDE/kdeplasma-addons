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

#ifndef PASSAGEWAYVIEWMODELS_H_
#define PASSAGEWAYVIEWMODELS_H_

#include "ActionListViewModels.h"

namespace Lancelot
{

class PassagewayViewModel: public ActionListViewModel {
public:
    PassagewayViewModel();
    virtual ~PassagewayViewModel();
    virtual PassagewayViewModel * child(int index) = 0;
    virtual QString modelTitle() const = 0;
    virtual KIcon * modelIcon()  const = 0;
};

class DummyPassagewayViewModel: public PassagewayViewModel {
private:
    int m_size;
    int m_level;
    QString m_title;
    DummyPassagewayViewModel * m_child;
    KIcon * m_icon;
    
public:
    DummyPassagewayViewModel(QString title, int size, int level)
        : Lancelot::PassagewayViewModel(),
          m_size(size), m_title(title), m_level(level),
          m_icon(new KIcon("lancelot")), m_child(NULL)
    {
         if (size > 3) {
             m_child = new DummyPassagewayViewModel(title, size - 1, level + 1);
         }
    }
    
    virtual ~DummyPassagewayViewModel()
    {
        delete m_child;
    }

    PassagewayViewModel * child(int index)
    {
        return m_child;
    }

    QString modelTitle() const {
        return m_title;
    }

    KIcon * modelIcon() const {
        return NULL;
    }

    QString title(int index) const {
        return m_title + " " + QString::number(index) + " " + QString::number(m_level) + QString((index < size())?"":"err");
    }

    QString description(int index) const {
        return "Description " + QString::number(index);
    }
    
    KIcon * icon(int index) const { Q_UNUSED(index); return m_icon; }
    int size() const { return m_size; }

    void activated(int index) { Q_UNUSED(index); }
};

}

#endif /*PASSAGEWAYVIEWMODELS_H_*/
