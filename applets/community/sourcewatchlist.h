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

#ifndef SOURCEWATCHLIST_H
#define SOURCEWATCHLIST_H

#include <QtCore/QString>
#include <QtCore/QSet>

#include <Plasma/DataEngine>


class SourceWatchList : public QObject
{
    Q_OBJECT

    public:
        explicit SourceWatchList(Plasma::DataEngine* engine, QObject* parent = 0);
        bool contains(const QString& key) const;
        QString query() const;
        void setQuery(const QString& query);
        void setUpdateInterval(uint updateInterval);
        QVariant value(const QString& id) const;

    Q_SIGNALS:
        void keysAdded(const QSet<QString>& keys);
        void keysRemoved(const QSet<QString>& keys);

    private Q_SLOTS:
        void dataUpdated(const QString& source, const Plasma::DataEngine::Data& data);

    private:
        Plasma::DataEngine::Data m_data;
        Plasma::DataEngine* m_engine;
        QString m_query;
        uint m_updateInterval;
};


#endif
