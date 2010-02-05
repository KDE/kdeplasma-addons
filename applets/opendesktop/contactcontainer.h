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

#ifndef CONTACTCONTAINER_H
#define CONTACTCONTAINER_H

#include <QtCore/QSignalMapper>
#include <QtGui/QGraphicsWidget>

#include <Plasma/DataEngine>

#include "personwatchlist.h"


class ContactWidget;
class QGraphicsLinearLayout;

namespace Plasma
{
    class ScrollWidget;
}

class ContactContainer : public QGraphicsWidget
{
    Q_OBJECT

public:
    explicit ContactContainer(Plasma::DataEngine* engine, Plasma::ScrollWidget* parent = 0);

Q_SIGNALS:
    void addFriend(const QString& id);
    void sendMessage(const QString& id);
    void showDetails(const QString& id);

public Q_SLOTS:
    void setOwnId(const QString& ownId);
    void setProvider(const QString& provider);
    void setSource(const QString& source);

private Q_SLOTS:
    void friendAdded(const QString& person);
    void friendRemoved(const QString& person);
    void personAdded(const QString& person);
    void personRemoved(const QString& person);

private:
    Plasma::ScrollWidget *m_scrollWidget;
    QSignalMapper m_addFriendMapper;
    Plasma::DataEngine* m_engine;
    PersonWatchList m_friendWatcher;
    QHash<QString, ContactWidget*> m_idToWidget;
    QGraphicsLinearLayout* m_layout;
    QString m_ownId;
    PersonWatchList m_personWatcher;
    QString m_provider;
    QSignalMapper m_sendMessageMapper;
    QSignalMapper m_showDetailsMapper;
    QString m_source;
};


#endif
