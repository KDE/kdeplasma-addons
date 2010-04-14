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

#include "friendmanagementcontainer.h"

#include <QtGui/QGraphicsLinearLayout>

#include "friendmanagementwidget.h"
#include "utils.h"


using namespace Plasma;


FriendManagementContainer::FriendManagementContainer(DataEngine* engine, QGraphicsWidget* parent)
    : QGraphicsWidget(parent), m_engine(engine), m_layout(new QGraphicsLinearLayout(Qt::Vertical)), m_personWatcher(engine)
{
    setLayout(m_layout);
    connect(&m_personWatcher, SIGNAL(personAdded(QString)), SLOT(personAdded(QString)));
    connect(&m_personWatcher, SIGNAL(personRemoved(QString)), SLOT(personRemoved(QString)));
}


void FriendManagementContainer::personAdded(const QString& person)
{
    FriendManagementWidget* widget = new FriendManagementWidget(m_engine);
    widget->setProvider(m_provider);
    widget->setId(person);
    m_layout->addItem(widget);
    m_idToWidget.insert(person, widget);
}


void FriendManagementContainer::personRemoved(const QString& person)
{
    FriendManagementWidget* widget = m_idToWidget.take(person);
    if (widget) {
        m_layout->removeItem(widget);
        widget->deleteLater();
    }
}


void FriendManagementContainer::setProvider(const QString& provider)
{
    m_provider = provider;
    m_personWatcher.setSource(receivedInvitationsQuery(m_provider));
    foreach (FriendManagementWidget* widget, m_idToWidget) {
        widget->setProvider(m_provider);
    }
}


#include "friendmanagementcontainer.moc"
