/***************************************************************************
 *   Copyright 2009 by Eckhart Wörner <ewoerner@kde.org>                   *
 *   Copyright 2010 Frederik Gladhorn <gladhorn@kde.org>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef ACTIVITIES_H
#define ACTIVITIES_H

#include <Plasma/PopupApplet>
#include <Plasma/DataEngine>

class ActivityList;

class OpenDesktopActivities : public Plasma::PopupApplet
{
    Q_OBJECT

    public:
        OpenDesktopActivities(QObject* parent, const QVariantList& args);
        void init();
        QGraphicsWidget* graphicsWidget();

    private Q_SLOTS:
        void initAsync();
        
        void dataUpdated(const QString &source, const Plasma::DataEngine::Data &data);

    private:
        void initEngine();
        ActivityList* m_activityList;
        Plasma::DataEngine* m_engine;
        int m_updateInterval;
        bool m_firstUpdateDone;
};

#endif
