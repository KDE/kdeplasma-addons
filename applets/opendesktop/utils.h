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

#ifndef UTILS_H
#define UTILS_H

#include <QtCore/QString>


QString escape(const QString& value);

QString friendsQuery(const QString& provider, const QString& id);

QString messageAddPrefix(const QString& message);

QString messageListQuery(const QString& provider, const QString& folder);

QString messageListUnreadQuery(const QString& provider, const QString& folder);

QString messageRemovePrefix(const QString& message);

QString messageQuery(const QString& provider, const QString& folder, const QString& message);

QString messageSummaryQuery(const QString& provider, const QString& folder, const QString& message);

QString personAddPrefix(const QString& id);

QString personQuery(const QString& provider, const QString& id);

QString personRemovePrefix(const QString& id);

QString personSelfQuery(const QString& provider);

QString personSummaryQuery(const QString& provider, const QString& id);

QString receivedInvitationsQuery(const QString& provider);

QString settingsQuery(const QString& provider, const QString& id);

#endif
