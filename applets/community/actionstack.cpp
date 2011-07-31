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

#include "actionstack.h"

#include <Plasma/TabBar>
#include <QStyleOptionGraphicsItem>
#include "contactlist.h"
#include "requestfriendshipwidget.h"
#include "sendmessagewidget.h"
#include "userwidget.h"


using namespace Plasma;


ActionStack::ActionStack(DataEngine* engine, QGraphicsWidget* mainWidget, QGraphicsWidget* parent)
    : QGraphicsWidget(parent)
{
    m_details = new UserWidget(engine);
    m_requestFriendship = new RequestFriendshipWidget(engine);
    m_sendMessage = new SendMessageWidget(engine);

    m_tabs = new TabBar;
    m_tabs->setTabBarShown(false);
    m_tabs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_tabs->addTab(QString(), mainWidget);
    m_tabs->addTab(QString(), m_details);
    m_tabs->addTab(QString(), m_sendMessage);
    m_tabs->addTab(QString(), m_requestFriendship);

    QGraphicsLinearLayout* layout = new QGraphicsLinearLayout(Qt::Horizontal);
    layout->setContentsMargins(0, 0, 0, 0);
    setContentsMargins(0, 0, 0, 0);
    mainWidget->setContentsMargins(0, 0, 0, 0);
    m_tabs->setContentsMargins(0, 0, 0, 0);
    layout->addItem(m_tabs);

    setLayout(layout);

    connect(this, SIGNAL(providerChanged(QString)), m_details, SLOT(setProvider(QString)));
    connect(this, SIGNAL(providerChanged(QString)), m_requestFriendship, SLOT(setProvider(QString)));
    connect(this, SIGNAL(providerChanged(QString)), m_sendMessage, SLOT(setProvider(QString)));

    connect(m_details, SIGNAL(sendMessage(QString)), this, SLOT(sendMessage(QString)));

    connect(m_details, SIGNAL(done()), SLOT(showMainWidget()));

    connect(m_requestFriendship, SIGNAL(done()), SLOT(showMainWidget()));

    connect(m_sendMessage, SIGNAL(endWork()), SIGNAL(endWork()));
    connect(m_sendMessage, SIGNAL(done()), SLOT(showMainWidget()));
    connect(m_sendMessage, SIGNAL(startWork()), SIGNAL(startWork()));
}


void ActionStack::addFriend(const QString& id)
{
    m_requestFriendship->setId(id);
    m_tabs->setCurrentIndex(3);
}


void ActionStack::sendMessage(const QString& id)
{
    m_sendMessage->setId(id);
    m_tabs->setCurrentIndex(2);
}


void ActionStack::setOwnId(const QString& ownId)
{
    m_details->setOwnId(ownId);
    emit ownIdChanged(ownId);
}


void ActionStack::setProvider(const QString& provider)
{
    emit providerChanged(provider);
}


void ActionStack::showDetails(const QString& id)
{
    m_details->setId(id);
    m_tabs->setCurrentIndex(1);
}


void ActionStack::showMainWidget()
{
    m_tabs->setCurrentIndex(0);
}

#include "actionstack.moc"
