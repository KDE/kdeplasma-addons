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

#include "personwatchlist.h"

#include "utils.h"


PersonWatchList::PersonWatchList(Plasma::DataEngine* engine, QObject* parent)
    : QObject(parent),
      m_list(engine)
{
    connect(&m_list, SIGNAL(keysAdded(QSet<QString>)), SLOT(slotKeysAdded(QSet<QString>)));
    connect(&m_list, SIGNAL(keysRemoved(QSet<QString>)), SLOT(slotKeysRemoved(QSet<QString>)));
}


bool PersonWatchList::contains(const QString& id) const {
    return m_list.contains(personAddPrefix(id));
}


void PersonWatchList::setSource(const QString& source) {
    m_list.setQuery(source);
}


void PersonWatchList::slotKeysAdded(const QSet<QString>& keys) {
    foreach(const QString& key, keys) {
        QString person = personRemovePrefix(key);
        if (!person.isEmpty()) {
            emit personAdded(person);
        }
    }
}


void PersonWatchList::slotKeysRemoved(const QSet<QString>& keys) {
    foreach(const QString& key, keys) {
        QString person = personRemovePrefix(key);
        if (!person.isEmpty()) {
            emit personRemoved(person);
        }
    }
}


#include "personwatchlist.moc"
