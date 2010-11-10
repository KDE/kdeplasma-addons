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

#include <QSet>

#include "messagecounter.h"
#include "utils.h"


MessageCounter::MessageCounter(Plasma::DataEngine* engine, QObject* parent)
    : QObject(parent), m_count(0), m_watcher(engine)
{
    m_watcher.setUpdateInterval(10 * 60 * 1000);
    connect(&m_watcher, SIGNAL(keysAdded(QSet<QString>)), SLOT(keysAdded(QSet<QString>)));
    connect(&m_watcher, SIGNAL(keysRemoved(QSet<QString>)), SLOT(keysRemoved(QSet<QString>)));
}


void MessageCounter::keysAdded(const QSet<QString>& keys)
{
    m_count += keys.size();
    emit messageCountChanged(m_count);
}


void MessageCounter::keysRemoved(const QSet<QString>& keys)
{
    m_count -= keys.size();
    emit messageCountChanged(m_count);
}


void MessageCounter::setProvider(const QString& provider)
{
    m_watcher.setQuery(messageListUnreadQuery(provider, "0"));
}


#include "messagecounter.moc"
