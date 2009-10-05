/*
*   Copyright 2009 Matteo Agostinelli <agostinelli@gmail.com>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License as
*   published by the Free Software Foundation; either version 2 or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details
*
*   You should have received a copy of the GNU Library General Public
*   License along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "qalculate_history.h"


QalculateHistory::QalculateHistory(QObject* parent): QObject(parent)
{
    m_currentItem = 0;
}

void QalculateHistory::addItem(const QString& expression)
{
    m_history << expression;
    m_backup = "";
    m_currentItem = m_history.size() - 1;
}

QString QalculateHistory::currentItem()
{
    if (m_history.isEmpty())
        return QString();

    if (m_currentItem < 0) {
        m_currentItem = 0;
    }

    if (m_currentItem >= m_history.size()) {
        m_currentItem = m_history.size();
        return m_backup;
    }

    return m_history.at(m_currentItem);
}

QString QalculateHistory::nextItem()
{
    m_currentItem++;
    return currentItem();
}

QString QalculateHistory::previousItem()
{
    m_currentItem--;
    return currentItem();
}

void QalculateHistory::setBackup(const QString& backup)
{
    m_backup = backup;
    m_currentItem++;
}

QString QalculateHistory::backup() const
{
    return m_backup;
}
