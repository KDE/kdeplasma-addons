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

#ifndef ATTICA_EVENT_H
#define ATTICA_EVENT_H

#include "atticaclient_export.h"

#include <QtCore/QDate>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QSharedDataPointer>


namespace Attica {
    
class EventParser;

class ATTICA_EXPORT Event
{
    public:
        typedef QList<Event> List;
        typedef EventParser Parser;

        Event();
        Event(const Event& other);
        Event& operator=(const Event& other);
        ~Event();

        void setId(const QString& id);
        QString id() const;

        void setName(const QString& name);
        QString name() const;

        void setDescription(const QString& text);
        QString description() const;

        void setUser(const QString& id);
        QString user() const;

        void setStartDate(const QDate& date);
        QDate startDate() const;

        void setEndDate(const QDate& date);
        QDate endDate() const;

        void setLatitude(qreal lat);
        qreal latitude() const;

        void setLongitude(qreal lon);
        qreal longitude() const;

        void setHomepage(const QString& url);
        QString homepage() const;

        void setCountry(const QString& country);
        QString country() const;

        void setCity(const QString& city);
        QString city() const;

        void addExtendedAttribute(const QString& key, const QString& value);
        QString extendedAttribute(const QString& key) const;
        QMap<QString,QString> extendedAttributes() const;

    private:
        class Private;
        QSharedDataPointer<Private> d;
};

}


#endif
