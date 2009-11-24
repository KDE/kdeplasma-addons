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

#include "friendlist.h"

#include "contactcontainer.h"
#include "friendmanagementcontainer.h"
#include "utils.h"


FriendList::FriendList(Plasma::DataEngine* engine, QGraphicsWidget* parent)
    : ScrollWidget(parent)
{
    m_invitations = new FriendManagementContainer(engine);
    m_widget = new ContactContainer(engine);

    QGraphicsLinearLayout* layout = new QGraphicsLinearLayout(Qt::Vertical);
    layout->addItem(m_invitations);
    layout->addItem(m_widget);

    QGraphicsWidget* widget = new QGraphicsWidget;
    // FIXME: The following line is a hack to prevent a segfault, report it!
    widget->setMinimumSize(100, 100);
    widget->setLayout(layout);

    setWidget(widget);

    connect(m_widget, SIGNAL(addFriend(QString)), SIGNAL(addFriend(QString)));
    connect(m_widget, SIGNAL(sendMessage(QString)), SIGNAL(sendMessage(QString)));
    connect(m_widget, SIGNAL(showDetails(QString)), SIGNAL(showDetails(QString)));
}


void FriendList::setProvider(const QString& provider)
{
    m_provider = provider;
    m_invitations->setProvider(provider);
    m_widget->setProvider(provider);
    m_widget->setSource(friendsQuery(m_provider, m_ownId));
}


void FriendList::setOwnId(const QString& id)
{
    m_ownId = id;
    m_widget->setOwnId(id);
    m_widget->setSource(friendsQuery(m_provider, m_ownId));
}


#include "friendlist.moc"
