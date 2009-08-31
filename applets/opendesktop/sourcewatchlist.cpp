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

#include "sourcewatchlist.h"


using namespace Plasma;

SourceWatchList::SourceWatchList(DataEngine* engine, QObject* parent)
    : QObject(parent),
      m_engine(engine)
{
}


bool SourceWatchList::contains(const QString& key) const
{
    return m_keys.contains(key);
}


QString SourceWatchList::query() const
{
    return m_query;
}


void SourceWatchList::setQuery(const QString& query)
{
    if (query != m_query) {
        m_engine->disconnectSource(m_query, this);
        m_query = query;
        m_engine->connectSource(m_query, this);
    }
}


void SourceWatchList::dataUpdated(const QString& source, const Plasma::DataEngine::Data& data)
{
    Q_UNUSED(source)

    QSet<QString> keys = QSet<QString>::fromList(data.keys());
    foreach(const QString& key, QSet<QString>(m_keys).subtract(keys)) {
        emit keyRemoved(key);
    }
    foreach(const QString& key, QSet<QString>(keys).subtract(m_keys)) {
        emit keyAdded(key);
    }
    m_keys = keys;
}


#include "sourcewatchlist.moc"
