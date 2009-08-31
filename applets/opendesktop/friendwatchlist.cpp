/*
    This file is part of KDE.

    Copyright (c) 2009 Eckhart Wörner <ewoerner@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#include "friendwatchlist.h"

#include "utils.h"


FriendWatchList::FriendWatchList(Plasma::DataEngine* engine, QObject* parent)
    : QObject(parent),
      m_list(engine)
{
    connect(&m_list, SIGNAL(keyAdded(QString)), SLOT(slotKeyAdded(QString)));
    connect(&m_list, SIGNAL(keyRemoved(QString)), SLOT(slotKeyRemoved(QString)));
}


bool FriendWatchList::contains(const QString& id) const
{
    return m_list.contains(personQuery(id));
}


QString FriendWatchList::relativeTo() const
{
    return m_list.query().remove(0, 8); // Remove the Friends- prefix
}


void FriendWatchList::setRelativeTo(const QString& id)
{
    if (!id.isEmpty()) {
        m_list.setQuery(friendsQuery(id));
    } else {
        m_list.setQuery(QString());
    }
}


void FriendWatchList::slotKeyAdded(const QString& key)
{
    emit friendAdded(QString(key).remove(0, 7)); // Remove the Person- prefix
}


void FriendWatchList::slotKeyRemoved(const QString& key)
{
    emit friendRemoved(QString(key).remove(0, 7)); // Remove the Person- prefix
}


#include "friendwatchlist.moc"
