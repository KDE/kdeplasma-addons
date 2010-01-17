/*
 *   Copyright (C) 2007, 2008, 2009, 2010 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include "BaseMergedModel.h"
#include "Serializator.h"

#include <KUrl>
#include <KTemporaryFile>

namespace Lancelot {
namespace Models {

class BaseMergedModel::Private {
public:
    QList < QString > modelIDs;
};

BaseMergedModel::BaseMergedModel()
    : Lancelot::MergedActionListModel(),
      d(new Private())
{

}

BaseMergedModel::~BaseMergedModel()
{
    delete d;
}

void BaseMergedModel::addModel(const QString & id, ActionListModel * model)
{
    addModel(id, model->selfIcon(), model->selfTitle(), model);
}

void BaseMergedModel::addModel(const QString & id, QIcon icon,
        const QString & title, ActionListModel * model)
{
    d->modelIDs.append(id);
    Lancelot::MergedActionListModel::addModel(icon, title, model);
}

QMimeData * BaseMergedModel::modelMimeData(int index) const
{
    if ((index < 0) || (index >= d->modelIDs.size())) {
        return NULL;
    }

    QMap < QString , QString > map;
    map["version"] = "1.0";
    map["type"]    = "list";
    map["model"]   = d->modelIDs.at(index);

    QMimeData * data = new QMimeData();
    data->setData("text/x-lancelotpart", Serializator::serialize(map).toAscii());
    return data;
}

void BaseMergedModel::setModelDropActions(int index, Qt::DropActions & actions,
        Qt::DropAction & defaultAction)
{
    Q_UNUSED(index);
    actions = Qt::CopyAction;
    defaultAction = Qt::CopyAction;
}

} // namespace Models
} // namespace Lancelot
