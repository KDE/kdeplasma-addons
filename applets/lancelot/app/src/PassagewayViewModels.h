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

#ifndef PASSAGEWAYVIEWMODELS_H_
#define PASSAGEWAYVIEWMODELS_H_

#include "ActionListViewModels.h"

namespace Lancelot
{

class PassagewayViewModel: public ActionListViewModel {
public:
    PassagewayViewModel() {};
    virtual ~PassagewayViewModel() {};

    PassagewayViewModel * child(int index) { Q_UNUSED(index); return NULL; }

};

}

#endif /*PASSAGEWAYVIEWMODELS_H_*/
