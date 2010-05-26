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

#include "contactcontainer.h"

#include <QtGui/QGraphicsLinearLayout>

#include <Plasma/ScrollWidget>

#include "contactwidget.h"
#include "utils.h"


using namespace Plasma;

ContactContainer::ContactContainer(DataEngine* engine, Plasma::ScrollWidget* parent)
    : QGraphicsWidget(parent),
      m_scrollWidget(parent),
      m_engine(engine),
      m_friendWatcher(engine),
      m_layout(new QGraphicsLinearLayout(Qt::Vertical)),
      m_personWatcher(engine)
{
    setLayout(m_layout);
    connect(&m_addFriendMapper, SIGNAL(mapped(QString)), SIGNAL(addFriend(QString)));
    connect(&m_friendWatcher, SIGNAL(personAdded(QString)), SLOT(friendAdded(QString)));
    connect(&m_friendWatcher, SIGNAL(personRemoved(QString)), SLOT(friendRemoved(QString)));
    connect(&m_personWatcher, SIGNAL(personAdded(QString)), SLOT(personAdded(QString)));
    connect(&m_personWatcher, SIGNAL(personRemoved(QString)), SLOT(personRemoved(QString)));
    connect(&m_sendMessageMapper, SIGNAL(mapped(QString)), SIGNAL(sendMessage(QString)));
    connect(&m_showDetailsMapper, SIGNAL(mapped(QString)), SIGNAL(showDetails(QString)));
}


void ContactContainer::friendAdded(const QString& person)
{
    if (m_idToWidget.contains(person)) {
        m_idToWidget.value(person)->setIsFriend(true);
    }
}


void ContactContainer::friendRemoved(const QString& person) {
    if (m_idToWidget.contains(person)) {
        m_idToWidget.value(person)->setIsFriend(false);
    }
}


void ContactContainer::personAdded(const QString& person)
{
    ContactWidget* widget = new ContactWidget(m_engine, this);

    widget->setProvider(m_provider);
    widget->setId(person);
    widget->setIsFriend(m_friendWatcher.contains(person));
    m_layout->addItem(widget);
    m_idToWidget.insert(person, widget);
    m_addFriendMapper.setMapping(widget, person);
    m_sendMessageMapper.setMapping(widget, person);
    m_showDetailsMapper.setMapping(widget, person);
    connect(widget, SIGNAL(addFriend()), &m_addFriendMapper, SLOT(map()));
    connect(widget, SIGNAL(sendMessage()), &m_sendMessageMapper, SLOT(map()));
    connect(widget, SIGNAL(showDetails()), &m_showDetailsMapper, SLOT(map()));
}


void ContactContainer::personRemoved(const QString& person)
{
    ContactWidget* widget = m_idToWidget.take(person);
    if (widget) {
        m_layout->removeItem(widget);
        widget->deleteLater();
    }
}


void ContactContainer::setOwnId(const QString& ownId)
{
    m_ownId = ownId;
    m_friendWatcher.setSource(friendsQuery(m_provider, m_ownId));
}


void ContactContainer::setProvider(const QString& provider)
{
    m_provider = provider;
    m_friendWatcher.setSource(friendsQuery(m_provider, m_ownId));
    foreach (ContactWidget* widget, m_idToWidget) {
        widget->setProvider(m_provider);
    }
}


void ContactContainer::setSource(const QString& source)
{
    m_source = source;
    m_personWatcher.setSource(m_source);
}


#include "contactcontainer.moc"
