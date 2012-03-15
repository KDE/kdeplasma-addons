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

#ifndef QALCULATE_HISTORY_H
#define QALCULATE_HISTORY_H

#include <QObject>
#include <QStringList>

class QalculateHistory : public QObject
{
    Q_OBJECT
public:
    QalculateHistory(QObject* parent = 0);

    void addItem(const QString&);

    QString currentItem();
    QString previousItem();
    QString nextItem();

    void setBackup(const QString&);
    QString backup() const;
    void setHistoryItems(QStringList items);
    QStringList historyItems() const;

    bool isAtEnd() const;

private:
    QStringList m_history;
    QString m_backup;

    int m_currentItem;
};

#endif // QALCULATE_HISTORY_H
