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

#include "utils.h"


QString escape(const QString& value) {
    return QString(value).replace('\\', "\\\\");
}


QString friendsQuery(const QString& provider, const QString& id) {
    if (!provider.isEmpty() && ! id.isEmpty()) {
        return QString("Friends\\provider:%1\\id:%2").arg(escape(provider)).arg(escape(id));
    } else {
        return QString();
    }
}


QString messageAddPrefix(const QString& message)
{
    return QString("Message-%1").arg(message);
}


QString messageListQuery(const QString& provider, const QString& folder)
{
    if (!provider.isEmpty() && ! folder.isEmpty()) {
        return QString("Messages\\provider:%1\\folder:%2").arg(escape(provider)).arg(escape(folder));
    } else {
        return QString();
    }
}


QString messageListUnreadQuery(const QString& provider, const QString& folder)
{
    if (!provider.isEmpty() && ! folder.isEmpty()) {
        return QString("Messages\\provider:%1\\folder:%2\\status:unread").arg(escape(provider)).arg(escape(folder));
    } else {
        return QString();
    }
}


QString messageRemovePrefix(const QString& id) {
    if (id.startsWith(QLatin1String("Message-"))) {
        QString message = QString(id).remove(0, 8);
        return message;
    } else {
        return QString();
    }
}


QString messageQuery(const QString& provider, const QString& folder, const QString& message)
{
    if (!provider.isEmpty() && ! folder.isEmpty() && !message.isEmpty()) {
        return QString("Message\\provider:%1\\folder:%2\\id:%3").arg(escape(provider)).arg(escape(folder)).arg(escape(message));
    } else {
        return QString();
    }
}


QString messageSummaryQuery(const QString& provider, const QString& folder, const QString& message)
{
    if (!provider.isEmpty() && ! folder.isEmpty() && !message.isEmpty()) {
        return QString("MessageSummary\\provider:%1\\folder:%2\\id:%3").arg(escape(provider)).arg(escape(folder)).arg(escape(message));
    } else {
        return QString();
    }
}


QString personAddPrefix(const QString& id) {
    return QString("Person-%1").arg(id);
}


QString personQuery(const QString& provider, const QString& id) {
    if (!provider.isEmpty() && ! id.isEmpty()) {
        return QString("Person\\provider:%1\\id:%2").arg(escape(provider)).arg(escape(id));
    } else {
        return QString();
    }
}


QString personRemovePrefix(const QString& id) {
    if (id.startsWith(QLatin1String("Person-"))) {
        QString person = QString(id).remove(0, 7);
        return person;
    } else {
        return QString();
    }
}


QString personSelfQuery(const QString& provider)
{
    if (!provider.isEmpty()) {
        return QString("PersonCheck\\provider:%1").arg(escape(provider));
    } else {
        return QString();
    }
}


QString personSummaryQuery(const QString& provider, const QString& id) {
    if (!provider.isEmpty() && ! id.isEmpty()) {
        return QString("PersonSummary\\provider:%1\\id:%2").arg(escape(provider)).arg(escape(id));
    } else {
        return QString();
    }
}


QString receivedInvitationsQuery(const QString& provider) {
    if (!provider.isEmpty()) {
        return QString("ReceivedInvitations\\provider:%1").arg(escape(provider));
    } else {
        return QString();
    }
}


QString settingsQuery(const QString& provider, const QString& id)
{
    if (!provider.isEmpty()) {
        return QString("Settings\\provider:%1\\id:%2").arg(escape(provider)).arg(escape(id));
    } else {
        return QString();
    }
}
