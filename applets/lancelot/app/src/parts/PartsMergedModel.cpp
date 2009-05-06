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

#include "PartsMergedModel.h"
#include <KIcon>
#include <KLocalizedString>

namespace Models {

PartsMergedModel::PartsMergedModel()
    : BaseMergedModel()
{
}

PartsMergedModel::~PartsMergedModel()
{
}

bool PartsMergedModel::hasModelContextActions(int index) const
{
    Q_UNUSED(index);
    return true;
}

void PartsMergedModel::setModelContextActions(int index, Lancelot::PopupMenu * menu)
{
    Q_UNUSED(index);
    menu->addAction(KIcon("list-remove"), i18n("Remove this"))
        ->setData(QVariant(0));
}

void PartsMergedModel::modelContextActivate(int index, QAction * context)
{
    if (!context) {
        return;
    }

    if (context->data().toInt() == 0) {
        emit removeModelRequested(index);
    }
}

void PartsMergedModel::setModelDropActions(int index, Qt::DropActions & actions,
        Qt::DropAction & defaultAction)
{
    Q_UNUSED(index);
    if (modelCount() <= 1) {
        actions = Qt::CopyAction;
        defaultAction = Qt::CopyAction;
    } else {
        actions = Qt::CopyAction | Qt::MoveAction;
        defaultAction = Qt::MoveAction;
    }
}

void PartsMergedModel::modelDataDropped(int index, Qt::DropAction action)
{
    if (action == Qt::MoveAction) {
        emit removeModelRequested(index);
    }
}

} // namespace Models
