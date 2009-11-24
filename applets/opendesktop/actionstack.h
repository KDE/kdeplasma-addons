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

#ifndef ACTIONSTACK_H
#define ACTIONSTACK_H

#include <QtGui/QGraphicsWidget>


class SendMessageWidget;
class RequestFriendshipWidget;
class UserWidget;

namespace Plasma {
    class DataEngine;
    class TabBar;
}

class ActionStack : public QGraphicsWidget
{
    Q_OBJECT

public:
    explicit ActionStack(Plasma::DataEngine* engine, QGraphicsWidget* mainWidget, QGraphicsWidget* parent = 0);

public Q_SLOTS:
    void addFriend(const QString& id);
    void setOwnId(const QString& ownId);
    void setProvider(const QString& provider);
    void sendMessage(const QString& id);
    void showDetails(const QString& id);
    void showMainWidget();

Q_SIGNALS:
    void endWork();
    void ownIdChanged(const QString& ownId);
    void providerChanged(const QString& provider);
    void sourceChanged(const QString& source);
    void startWork();

private:
    UserWidget* m_details;
    RequestFriendshipWidget* m_requestFriendship;
    SendMessageWidget* m_sendMessage;
    Plasma::TabBar* m_tabs;
};


#endif
