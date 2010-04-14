/*
    This file is part of KDE.

    Copyright (c) 2009 Eckhart Wörner <ewoerner@kde.org>
    Copyright 2010 Frederik Gladhorn <gladhorn@kde.org>

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

#include "activitylist.h"

#include <KNotification>

#include "activitywidget.h"


ActivityList::ActivityList(Plasma::DataEngine* engine, QGraphicsWidget* parent)
    : ScrollWidget(parent),
      m_engine(engine),
      m_limit(30),
      m_firstUpdateDone(false)
{
    m_container = new QGraphicsWidget(this);
    m_layout = new QGraphicsLinearLayout(Qt::Vertical, m_container);
    setWidget(m_container);
}

int ActivityList::limit() const
{
    return m_limit;
}

void ActivityList::setLimit(int limit)
{
    m_limit = limit;
}

void ActivityList::dataUpdated(const QString& source, const Plasma::DataEngine::Data& data)
{
    Q_UNUSED(source)
    Q_ASSERT(m_engine);
    
    int newItems = 0;

    foreach(const QString& key, data.keys()) {
        if (!data.value(key).value<Plasma::DataEngine::Data>().isEmpty()) {
            
            if (m_knownEvents.contains(source + data[key].value<Plasma::DataEngine::Data>().value("id").toString())) {
                continue;
            }
            
            QString id(source + data[key].value<Plasma::DataEngine::Data>().value("id").toString());
            QDateTime time(data[key].value<Plasma::DataEngine::Data>().value("timestamp").toDateTime());
            m_knownEvents.insert(id);

            ActivityWidget* widget = new ActivityWidget(m_engine, m_container);
            widget->setActivityData(data[key].value<Plasma::DataEngine::Data>());

            int i;
            for(i = 0; i < m_layout->count(); ++i) {
                ActivityWidget* widget = static_cast<ActivityWidget*>(m_layout->itemAt(i));
                if (time > widget->timestamp()) {
                    break;
                }
            }
            m_layout->insertItem(i, widget);
            
            ++newItems;
        }
    }
    
    while (m_layout->count() > m_limit) {
        ActivityWidget* widget = static_cast<ActivityWidget*>(m_layout->itemAt(m_layout->count()-1));
        m_layout->removeAt(m_layout->count()-1);
        widget->deleteLater();
    }
    
    // Don't mass-spam the user with notifications
    if (newItems < 4) {
        for(int i = 0; i<newItems; ++i) {
            ActivityWidget* widget = static_cast<ActivityWidget*>(m_layout->itemAt(i));
            KNotification* notification = new KNotification("activity");
            notification->setTitle("OpenDesktop Activities");
            notification->setText(widget->message());
            notification->setComponentData(KComponentData("plasma-applet-opendesktop-activities", "plasma-applet-opendesktop-activities", KComponentData::SkipMainComponentRegistration));
            notification->sendEvent();
        }
    }

    // Go to the top of the list
    setScrollPosition(QPointF(0, 0));
}


#include "activitylist.moc"
