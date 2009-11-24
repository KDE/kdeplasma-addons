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

#ifndef FRIENDMANAGEMENTCONTAINER_H
#define FRIENDMANAGEMENTCONTAINER_H

#include <QtGui/QGraphicsWidget>

#include <Plasma/DataEngine>

#include "personwatchlist.h"


class FriendManagementWidget;
class QGraphicsLinearLayout;

class FriendManagementContainer : public QGraphicsWidget
{
    Q_OBJECT

public:
    explicit FriendManagementContainer(Plasma::DataEngine* engine, QGraphicsWidget* parent = 0);

public Q_SLOTS:
    void setProvider(const QString& provider);

private Q_SLOTS:
    void personAdded(const QString& person);
    void personRemoved(const QString& person);

private:
    Plasma::DataEngine* m_engine;
    QHash<QString, FriendManagementWidget*> m_idToWidget;
    QGraphicsLinearLayout* m_layout;
    PersonWatchList m_personWatcher;
    QString m_provider;
    QHash<FriendManagementWidget*, QString> m_widgetToId;
};


#endif
