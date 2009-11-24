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

#include "ownidwatcher.h"

#include "utils.h"


using namespace Plasma;

OwnIdWatcher::OwnIdWatcher(DataEngine* engine, QObject* parent)
    : QObject(parent),
      m_engine(engine)
{
}


void OwnIdWatcher::dataUpdated(const QString& source, const DataEngine::Data& data)
{
    if (source != m_source) {
        return;
    }
    foreach (const QString& key, data.keys()) {
        QString id = personRemovePrefix(key);
        if (!id.isEmpty()) {
            emit changed(id);
        }
    }
}


void OwnIdWatcher::setProvider(const QString& provider)
{
    if (m_provider != provider) {
        if (!m_source.isEmpty()) {
            m_engine->disconnectSource(m_source, this);
        }
        m_provider = provider;
        emit changed(QString());
        m_source = personSelfQuery(m_provider);
        if (!m_source.isEmpty()) {
            m_engine->connectSource(m_source, this);
        }
    }
}


#include "ownidwatcher.moc"
