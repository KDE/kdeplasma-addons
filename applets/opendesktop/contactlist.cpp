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

#include "contactlist.h"

#include "contactcontainer.h"


ContactList::ContactList(Plasma::DataEngine* engine, QGraphicsWidget* parent)
    : ScrollWidget(parent)
{
    m_widget = new ContactContainer(engine, this);
    setWidget(m_widget);

    connect(m_widget, SIGNAL(addFriend(QString)), SIGNAL(addFriend(QString)));
    connect(m_widget, SIGNAL(sendMessage(QString)), SIGNAL(sendMessage(QString)));
    connect(m_widget, SIGNAL(showDetails(QString)), SIGNAL(showDetails(QString)));
}


void ContactList::setProvider(const QString& provider)
{
    m_widget->setProvider(provider);
}


void ContactList::setQuery(const QString& query)
{
    m_widget->setSource(query);
}


void ContactList::setOwnId(const QString& id)
{
    m_widget->setOwnId(id);
}


#include "contactlist.moc"
