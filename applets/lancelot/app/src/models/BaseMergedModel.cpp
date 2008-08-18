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

#include "BaseMergedModel.h"
#include "Serializator.h"

#include <KDebug>
#include <KUrl>
#include <KTemporaryFile>

namespace Models {

BaseMergedModel::BaseMergedModel()
    : Lancelot::MergedActionListViewModel()
{

}

BaseMergedModel::~BaseMergedModel()
{
}

void BaseMergedModel::addModel(const QString & id, QIcon icon,
        const QString & title, ActionListViewModel * model)
{
    m_modelIDs.append(id);
    Lancelot::MergedActionListViewModel::addModel(icon, title, model);
}

QMimeData * BaseMergedModel::modelMimeData(int index) const
{
    if ((index < 0) || (index >= m_modelIDs.size())) {
        return NULL;
    }

    QMap < QString , QString > map;
    map["version"] = "1.0";
    map["type"]    = "list";
    map["model"]   = m_modelIDs.at(index);

    KTemporaryFile file;
    file.setAutoRemove(false);
    file.setSuffix(".lancelotpart");
    if (!file.open()) {
        return NULL;
    }

    QTextStream out(&file);
    out << Serializator::serialize(map).toAscii();
    out.flush();

    QMimeData * data = new QMimeData();
    QByteArray urlData = KUrl(file.fileName()).url().toAscii();
    data->setData("text/uri-list", urlData);
    data->setData("text/plain", urlData);
    file.close();

    return data;
}

void BaseMergedModel::setModelDropActions(int index, Qt::DropActions & actions,
        Qt::DropAction & defaultAction)
{
    actions = Qt::CopyAction;
    defaultAction = Qt::CopyAction;
}

} // namespace Models
