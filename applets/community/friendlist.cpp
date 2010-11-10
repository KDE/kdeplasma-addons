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

#include <KDebug>

#include "contactcontainer.h"
#include "friendmanagementcontainer.h"
#include "utils.h"


FriendList::FriendList(Plasma::DataEngine* engine, QGraphicsWidget* parent)
: QGraphicsWidget(parent)
{
    m_invitations = new FriendManagementContainer(engine);

    setContentsMargins(0,0,0,0);
    QGraphicsLinearLayout* layout = new QGraphicsLinearLayout(Qt::Vertical);
    layout->addItem(m_invitations);
    layout->setContentsMargins(0,0,0,0);
    
    Plasma::ScrollWidget* friendScroll = new Plasma::ScrollWidget(this);
    m_friendListWidget = new ContactContainer(engine, friendScroll);
    friendScroll->setWidget(m_friendListWidget);
    
    layout->addItem(friendScroll);
    
    setLayout(layout);
  
    connect(m_friendListWidget, SIGNAL(addFriend(QString)), SIGNAL(addFriend(QString)));
    connect(m_friendListWidget, SIGNAL(sendMessage(QString)), SIGNAL(sendMessage(QString)));
    connect(m_friendListWidget, SIGNAL(showDetails(QString)), SIGNAL(showDetails(QString)));
}


void FriendList::setProvider(const QString& provider)
{
    kDebug() << "provider" << provider;
    m_provider = provider;
    m_invitations->setProvider(provider);
    m_friendListWidget->setProvider(provider);
    m_friendListWidget->setSource(friendsQuery(m_provider, m_ownId));
}


void FriendList::setOwnId(const QString& id)
{
    kDebug() << id;
    m_ownId = id;
    m_friendListWidget->setOwnId(id);
    m_friendListWidget->setSource(friendsQuery(m_provider, m_ownId));
}


#include "friendlist.moc"
