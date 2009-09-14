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

#include "activitylist.h"

#include <QtCore/QtAlgorithms>

#include "activitywidget.h"


ActivityList::ActivityList(Plasma::DataEngine* engine, QGraphicsWidget* parent)
    : ScrollWidget(parent),
      m_engine(engine),
      m_limit(20),
      m_updateInterval(10 * 60)
{
    m_container = new QGraphicsWidget(this);
    m_layout = new QGraphicsLinearLayout(Qt::Vertical, m_container);
    setWidget(m_container);

    m_engine->connectSource("activity", this, m_updateInterval * 1000);
    dataUpdated("activity", m_engine->query("activity"));
}


int ActivityList::limit() const
{
    return m_limit;
}


int ActivityList::updateInterval() const
{
    return m_updateInterval;
}


void ActivityList::setLimit(int limit)
{
    if (limit != m_limit) {
        m_limit = limit;
        dataUpdated("activity", m_engine->query("activity"));
    }
}


void ActivityList::setUpdateInterval(int interval)
{
    m_updateInterval = interval;
    m_engine->connectSource("activity", this, m_updateInterval * 1000);
}


QStringList ActivityList::getDisplayedActivities(const Plasma::DataEngine::Data& data) {
    QStringList result = data.keys();
    qSort(result.begin(), result.end(), qGreater<QString>());
    while (result.size() > m_limit) {
        result.pop_back();
    }
    return result;
}


void ActivityList::dataUpdated(const QString& source, const Plasma::DataEngine::Data& data)
{
    Q_UNUSED(source)
    
    QStringList displayedActivities = getDisplayedActivities(data);
    
    // FIXME: This is still highly inefficient
    while (m_layout->count()) {
        ActivityWidget* widget = static_cast<ActivityWidget*>(m_layout->itemAt(0));
        m_layout->removeAt(0);
        widget->deleteLater();
    }

    QStringList::iterator j = displayedActivities.begin();
    for (int i = 0; i < displayedActivities.size(); ++i, ++j) {
        ActivityWidget* widget = new ActivityWidget(m_container);
        widget->setActivityData(data[*j].value<Plasma::DataEngine::Data>());
        m_layout->addItem(widget);
    }
    
    // Go to the top of the list
    this->setPos(0, 0);
}


#include "activitylist.moc"
