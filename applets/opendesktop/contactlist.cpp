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


ContactList::ContactList(Plasma::DataEngine* engine, QGraphicsWidget* parent)
    : ScrollWidget(parent),
      m_engine(engine),
      m_limit(20),
      m_friends(engine)
{
    m_container = new QGraphicsWidget(this);
    m_layout = new QGraphicsLinearLayout(Qt::Vertical, m_container);
    setWidget(m_container);
    connect(&m_friends, SIGNAL(friendAdded(QString)), SLOT(friendAdded(QString)));
    connect(&m_friends, SIGNAL(friendRemoved(QString)), SLOT(friendRemoved(QString)));
}


QString ContactList::query() const
{
    return m_query;
}


void ContactList::setQuery(const QString& query)
{
    if (query != m_query) {
        if (!m_query.isEmpty()) {
            m_engine->disconnectSource(m_query, this);
        }
        m_query = query;
        if (!m_query.isEmpty()) {
            m_engine->connectSource(m_query, this);
            dataUpdated(m_query, m_engine->query(m_query));
        }
    }
}


void ContactList::setOwnId(const QString& id)
{
    m_friends.setRelativeTo(id);
}


QStringList ContactList::getDisplayedContacts(const Plasma::DataEngine::Data& data) {
    // FIXME: This should take into order filtering, sorting etc.
    QStringList result;
    foreach (const QString& key, data.keys()) {
        if (result.size() >= m_limit) {
            break;
        }
        if (key.startsWith("Person-")) {
            result.append(QString(key).remove(0, 7));
        }
    }
    return result;
}


void ContactList::dataUpdated(const QString& source, const Plasma::DataEngine::Data& data)
{
    Q_UNUSED(source)
    
    QStringList displayedContacts = getDisplayedContacts(data);
    
    // FIXME: This is still highly inefficient
    while (m_layout->count()) {
        ContactWidget* widget = static_cast<ContactWidget*>(m_layout->itemAt(0));
        m_layout->removeAt(0);
        widget->deleteLater();
    }
    m_mapping.clear();
    m_idToWidget.clear();

    QStringList::iterator j = displayedContacts.begin();
    for (int i = 0; i < displayedContacts.size(); ++i, ++j) {
        ContactWidget* widget;
        widget = new ContactWidget(m_engine, m_container);
        widget->setId(*j);
        widget->setIsFriend(m_friends.contains(*j));
        m_layout->addItem(widget);
        m_mapping.insert(widget, *j);
        m_idToWidget.insert(*j, widget);
        connect(widget, SIGNAL(addFriend()), SLOT(addFriend()));
        connect(widget, SIGNAL(sendMessage()), SLOT(sendMessage()));
        connect(widget, SIGNAL(showDetails()), SLOT(showDetails()));
    }
    this->setPos(0, 0);
}


int ContactList::limit() const
{
    return m_limit;
}


void ContactList::setLimit(int limit)
{
    if (limit != m_limit) {
        m_limit = limit;
        dataUpdated(m_query, m_engine->query(m_query));
    }
}


void ContactList::addFriend()
{
    ContactWidget* widget = static_cast<ContactWidget*>(sender());
    emit addFriend(m_mapping[widget]);
}


void ContactList::sendMessage()
{
    ContactWidget* widget = static_cast<ContactWidget*>(sender());
    emit sendMessage(m_mapping[widget]);
}


void ContactList::showDetails()
{
    ContactWidget* widget = static_cast<ContactWidget*>(sender());
    emit showDetails(m_mapping[widget]);
}


void ContactList::friendAdded(const QString& id)
{
    if (m_idToWidget.contains(id)) {
        m_idToWidget[id]->setIsFriend(true);
    }
}


void ContactList::friendRemoved(const QString& id)
{
    if (m_idToWidget.contains(id)) {
        m_idToWidget[id]->setIsFriend(false);
    }
}


#include "contactlist.moc"
