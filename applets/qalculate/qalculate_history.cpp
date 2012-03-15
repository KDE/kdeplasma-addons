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

#include <QDebug>

QalculateHistory::QalculateHistory(QObject* parent): QObject(parent)
{
    m_currentItem = 0;
}

void QalculateHistory::addItem(const QString& expression)
{
    m_history.push_back(expression);
    m_backup = "";
    m_currentItem = m_history.size() - 1;
    if (m_history.size() > 10) {
        m_history.removeFirst();
    }
}

QString QalculateHistory::currentItem()
{
    qDebug() << "Current item: " << m_currentItem;
    qDebug() << "History size: " << m_history.size();

    if (m_history.isEmpty()) {
        return QString();
    }

    if (m_currentItem >= m_history.size()) {
        if (!backup().isEmpty()) {
            m_currentItem = m_history.size();
            return m_backup;
        } else {
            m_currentItem = m_history.size() - 1;
        }
    }

    if (m_currentItem < 0) {
        m_currentItem = 0;
    }

    qDebug() << "Final current item: " << m_currentItem;
    qDebug() << "---";

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
    if (m_history.isEmpty()) {
        return;
    }

    if (backup != m_history.last()) {
        m_backup = backup;
        m_currentItem++;
    } else {
        m_backup = "";
    }
}

QString QalculateHistory::backup() const
{
    return m_backup;
}

bool QalculateHistory::isAtEnd() const
{
    return m_currentItem >= 0 && m_history.size() - 1;
}

QStringList QalculateHistory::historyItems() const
{
    return m_history;
}

void QalculateHistory::setHistoryItems ( QStringList items )
{
    m_history = items;
}


