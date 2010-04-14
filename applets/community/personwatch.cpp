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

#include "personwatch.h"

#include "utils.h"


using namespace Plasma;

PersonWatch::PersonWatch(DataEngine* engine, QObject* parent)
    : QObject(parent),
      m_engine(engine)
{
}


DataEngine::Data PersonWatch::data() const
{
    return m_data;
}


void PersonWatch::dataUpdated(const QString& source, const DataEngine::Data& data)
{
    if (source != m_source) {
        return;
    }
    DataEngine::Data ocsData = data.value(personAddPrefix(m_id)).value<DataEngine::Data>();
    if (ocsData != m_data) {
        m_data = ocsData;
        emit updated();
    }
}


void PersonWatch::setId(const QString& id)
{
    setSourceParameter(m_id, id);
}


void PersonWatch::setProvider(const QString& provider)
{
    setSourceParameter(m_provider, provider);
}


void PersonWatch::setSourceParameter(QString& variable, const QString& value)
{
    if (variable != value) {
        if (!m_source.isEmpty()) {
            m_engine->disconnectSource(m_source, this);
        }
        variable = value;
        dataUpdated(m_source, DataEngine::Data());
        m_source = personQuery(m_provider, m_id);
        if (!m_source.isEmpty()) {
            m_engine->connectSource(m_source, this);
        }
    }
}


#include "personwatch.moc"
