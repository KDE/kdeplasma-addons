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

#ifndef ACTIVITYLIST_H
#define ACTIVITYLIST_H

#include <QtCore/QString>
#include <QtGui/QGraphicsLinearLayout>

#include <Plasma/DataEngine>
#include <Plasma/ScrollWidget>



class ActivityList : public Plasma::ScrollWidget
{
    Q_OBJECT
    
    public:
        /**
         * Creates a new ActivityList widget
         * @param engine the Plasma data engine to use
         * @param parent the parent of this widget
         */
        explicit ActivityList(Plasma::DataEngine* engine, QGraphicsWidget* parent = 0);

        /**
         * Returns the maximum number of items shown
         * @return the current limit
         */
        int limit() const;

        /**
         * Returns the interval for refreshing the activities list
         * @return the current interval in seconds
         */
        int updateInterval() const;

        /**
         * Adjusts the maximum number of items shown
         * @param limit the new limit
         */
        void setLimit(int limit);

    public Q_SLOTS:
        void dataUpdated(const QString& source, const Plasma::DataEngine::Data& data);

    private:
        QStringList getDisplayedActivities(const Plasma::DataEngine::Data& data);
        QSet<QString> m_knownEvents;
        QGraphicsWidget* m_container;
        Plasma::DataEngine* m_engine;
        QGraphicsLinearLayout* m_layout;
        int m_limit;
        QString m_provider;
        int m_updateInterval;
        bool m_firstUpdateDone;
};


#endif
