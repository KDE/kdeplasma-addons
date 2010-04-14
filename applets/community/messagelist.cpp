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

#include "messagelist.h"

#include "messagewidget.h"
#include "utils.h"


MessageList::MessageList(Plasma::DataEngine* engine, QGraphicsWidget* parent)
    : ScrollWidget(parent),
      m_engine(engine),
      m_messageWatcher(engine)
{
    m_container = new QGraphicsWidget(this);
    m_layout = new QGraphicsLinearLayout(Qt::Vertical, m_container);
    setWidget(m_container);
    connect(&m_messageWatcher, SIGNAL(messageAdded(QString)), SLOT(messageAdded(QString)));
    connect(&m_messageWatcher, SIGNAL(messageRemoved(QString)), SLOT(messageRemoved(QString)));
}


void MessageList::setFolder(const QString& folder)
{
    m_folder = folder;
    m_messageWatcher.setSource(messageListQuery(m_provider, m_folder));
    foreach (MessageWidget* widget, m_idToWidget) {
        widget->setFolder(m_folder);
    }
}


void MessageList::setProvider(const QString& provider)
{
    m_provider = provider;
    m_messageWatcher.setSource(messageListQuery(m_provider, m_folder));
    foreach (MessageWidget* widget, m_idToWidget) {
        widget->setProvider(m_provider);
    }
}


void MessageList::messageAdded(const QString& id)
{
    MessageWidget* widget = new MessageWidget(m_engine);
    widget->setProvider(m_provider);
    widget->setFolder(m_folder);
    widget->setMessage(id);
    m_layout->addItem(widget);
    m_idToWidget.insert(id, widget);
}


void MessageList::messageRemoved(const QString& id)
{
    MessageWidget* widget = m_idToWidget.take(id);
    if (widget) {
        m_layout->removeItem(widget);
        widget->deleteLater();
    }
}


#include "messagelist.moc"
