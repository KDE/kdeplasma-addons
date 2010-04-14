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

#ifndef CONTACTLIST_H
#define CONTACTLIST_H

#include <QtCore/QString>
#include <QtGui/QGraphicsLinearLayout>

#include <Plasma/ScrollWidget>


class ContactContainer;

namespace Plasma {
    class DataEngine;
}

/**
 * The ContactList class provides a Plasma widget for displaying lists of contactwidget
 */
class ContactList : public Plasma::ScrollWidget
{
    Q_OBJECT
    
    public:
        /**
         * Creates a new ContactList widget without entries
         * @param engine the Plasma data engine to use
         * @param parent the parent of this widget
         */
        explicit ContactList(Plasma::DataEngine* engine, QGraphicsWidget* parent = 0);

    public Q_SLOTS:
        /**
         * Sets the id of the user (i.e. the id that is taken into account when computing friendships)
         * @param id the new id
         */
        void setOwnId(const QString& id);

        void setProvider(const QString& provider);

        /**
         * Sets the query whose results should be displayed
         * @param query a source of the data engine that can be interpreted as a list of contacts (where each person is identified by a Person-[id] key)
         */
        void setQuery(const QString& query);

    Q_SIGNALS:
        /**
         * This signal is emitted when the user requests to add a friend
         * @param id the identification of the new friend
         */
        void addFriend(const QString& id);

        /**
         * This signal is emitted when the user requests the details of a contact
         * @param id the identification of the requested contact details
         */
        void showDetails(const QString& id);

        /**
         * This signal is emitted when the user requests to write a message to a contact
         * @param id the identification of the recipient
         */
        void sendMessage(const QString& id);

    private:
        ContactContainer* m_widget;
};


#endif
