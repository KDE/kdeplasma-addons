/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *   Copyright (C) 2007 Robert Knight <robertknight@gmail.com>
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

#ifndef LANCELOT_MODELS_SYSTEMSERVICES_H_
#define LANCELOT_MODELS_SYSTEMSERVICES_H_

#include "BaseModel.h"

namespace Lancelot {
namespace Models {

class SystemServices : public BaseModel {
    Q_OBJECT
public:
    SystemServices();
    virtual ~SystemServices();

protected:
    void load();

};

}
}

#endif /* LANCELOT_MODELS_SYSTEMSERVICES_H_ */
