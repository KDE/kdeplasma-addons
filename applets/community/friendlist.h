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

#ifndef FRIENDLIST_H
#define FRIENDLIST_H

#include <QtCore/QString>
#include <QtGui/QGraphicsLinearLayout>

#include <Plasma/ScrollWidget>


class ContactContainer;
class FriendManagementContainer;

namespace Plasma {
    class DataEngine;
}

class FriendList : public QGraphicsWidget
{
    Q_OBJECT
    
    public:
        explicit FriendList(Plasma::DataEngine* engine, QGraphicsWidget* parent = 0);

    public Q_SLOTS:
        void setOwnId(const QString& id);
        void setProvider(const QString& provider);

    Q_SIGNALS:
        void addFriend(const QString& id);
        void sendMessage(const QString& id);
        void showDetails(const QString& id);

    private:
        FriendManagementContainer* m_invitations;
        QString m_ownId;
        QString m_provider;
        ContactContainer* m_friendListWidget;
};


#endif
