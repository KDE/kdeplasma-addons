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

#ifndef LANCELOTAPP_MODELS_BASEMERGEDMODEL_H_
#define LANCELOTAPP_MODELS_BASEMERGEDMODEL_H_

#include <lancelot/models/ActionListViewModels.h>

namespace Models {

class BaseMergedModel: public Lancelot::MergedActionListViewModel {
public:
    BaseMergedModel();
    ~BaseMergedModel();

    void addModel(const QString & id, QIcon icon,
            const QString & title, ActionListViewModel * model);
    L_Override virtual QMimeData * modelMimeData(int index) const;
    L_Override virtual void setModelDropActions(int index,
            Qt::DropActions & actions, Qt::DropAction & defaultAction);

private:
    QList < QString > m_modelIDs;
};

} // namespace Models

#endif // LANCELOTAPP_MODELS_BASEMERGEDMODEL_H_
