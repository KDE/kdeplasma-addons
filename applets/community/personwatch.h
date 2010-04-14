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

#ifndef PERSONWATCH_H
#define PERSONWATCH_H

#include <QtCore/QObject>

#include <Plasma/DataEngine>


class PersonWatch : public QObject
{
    Q_OBJECT

public:
    explicit PersonWatch(Plasma::DataEngine* engine, QObject* parent = 0);
    Plasma::DataEngine::Data data() const;

public Q_SLOTS:
    void setId(const QString& id);
    void setProvider(const QString& provider);

Q_SIGNALS:
    void updated();

private Q_SLOTS:
    void dataUpdated(const QString& source, const Plasma::DataEngine::Data& data);

private:
    void setSourceParameter(QString& variable, const QString& value);

    Plasma::DataEngine::Data m_data;
    Plasma::DataEngine* m_engine;
    QString m_id;
    QString m_provider;
    QString m_source;
};


#endif
