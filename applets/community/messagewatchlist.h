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

#ifndef MESSAGEWATCHLIST_H
#define MESSAGEWATCHLIST_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QSet>

#include "sourcewatchlist.h"


class MessageWatchList : public QObject
{
    Q_OBJECT

public:
    explicit MessageWatchList(Plasma::DataEngine* engine, QObject* parent = 0);
    bool contains(const QString& id) const;
    void setSource(const QString& source);

Q_SIGNALS:
    void messageAdded(const QString& id);
    void messageRemoved(const QString& id);

private Q_SLOTS:
    void slotKeysAdded(const QSet<QString>& keys);
    void slotKeysRemoved(const QSet<QString>& keys);

private:
    SourceWatchList m_list;
    QString m_source;
};


#endif
