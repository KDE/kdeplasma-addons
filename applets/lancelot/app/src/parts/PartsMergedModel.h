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

#ifndef LANCELOTPARTS_MODELS_PARTSMERGEDMODEL_H_
#define LANCELOTPARTS_MODELS_PARTSMERGEDMODEL_H_

#include "../models/BaseMergedModel.h"

namespace Models {

class PartsMergedModel: public BaseMergedModel {
    Q_OBJECT
public:
    PartsMergedModel();
    ~PartsMergedModel();

    L_Override virtual bool hasModelContextActions(int index) const;
    L_Override virtual void setModelContextActions(int index, QMenu * menu);
    L_Override virtual void modelContextActivate(int index, QAction * context);
    L_Override virtual void setModelDropActions(int index,
            Qt::DropActions & actions, Qt::DropAction & defaultAction);
    L_Override virtual void modelDataDropped(int index, Qt::DropAction action);

Q_SIGNALS:
    void removeModelRequested(int index);
};

} // namespace Models

#endif // LANCELOTPARTS_MODELS_PARTSMERGEDMODEL_H_


