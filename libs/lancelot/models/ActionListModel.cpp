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

#include "ActionListModel.h"

namespace Lancelot {

// ActionListModel

ActionListModel::ActionListModel()
{
}

ActionListModel::~ActionListModel()
{
}

QString ActionListModel::description(int index) const
{
    Q_UNUSED(index);
    return QString();
}

QIcon ActionListModel::icon(int index) const
{
    Q_UNUSED(index);
    return QIcon();
}

QMimeData * ActionListModel::mimeData(int index) const
{
    Q_UNUSED(index);
    return NULL;
}

bool ActionListModel::dataDropAvailable(int where, const QMimeData * mimeData)
{
    Q_UNUSED(where);
    Q_UNUSED(mimeData);
    return false;
}

void ActionListModel::dataDropped(int where, const QMimeData * mimeData)
{
    Q_UNUSED(where);
    Q_UNUSED(mimeData);
}

void ActionListModel::dataDragFinished(int index, Qt::DropAction action)
{
    Q_UNUSED(index);
    Q_UNUSED(action);
}

void ActionListModel::setDropActions(int index, Qt::DropActions & actions, Qt::DropAction & defaultAction)
{
    Q_UNUSED(index);
    actions = Qt::IgnoreAction;
    defaultAction = Qt::IgnoreAction;
}

bool ActionListModel::isCategory(int index) const
{
    Q_UNUSED(index);
    return false;
}

void ActionListModel::activated(int index)
{
    activate(index);
    emit itemActivated(index);
}

bool ActionListModel::hasContextActions(int index) const
{
    Q_UNUSED(index);
    return false;
}

void ActionListModel::setContextActions(int index, Lancelot::PopupMenu * menu)
{
    Q_UNUSED(index);
    Q_UNUSED(menu);
}

void ActionListModel::contextActivate(int index, QAction * context)
{
    Q_UNUSED(index);
    Q_UNUSED(context);
}

void ActionListModel::activate(int index)
{
    Q_UNUSED(index);
}

QString ActionListModel::selfTitle() const
{
    return QString();
}

QString ActionListModel::selfShortTitle() const
{
    return QString();
}

QIcon ActionListModel::selfIcon() const
{
    return QIcon();
}

QMimeData * ActionListModel::selfMimeData() const
{
    return NULL;
}

} // namespace Lancelot

#include "ActionListModel.moc"

