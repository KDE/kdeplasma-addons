/*
 *   Copyright (C) 2010 Alexey Noskov <alexey.noskov@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2 of
 *   the License or (at your option) version 3 or any later version
 *   accepted by the membership of KDE e.V. (or its successor approved
 *   by the membership of KDE e.V.), which shall act as a proxy
 *   defined in Section 14 of version 3 of the license.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EVENTS_H
#define EVENTS_H

#include <Plasma/AbstractRunner>

#include <Akonadi/Collection>

#include <KIcon>

#include <QMap>

#include "datetime_parser.h"

/**
*/
class EventsRunner : public Plasma::AbstractRunner {
    Q_OBJECT

public:
    // Basic Create/Destroy
    EventsRunner( QObject *parent, const QVariantList& args );
    ~EventsRunner();

    void match(Plasma::RunnerContext &context);
    void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match);
    
    void reloadConfiguration();
    
private slots:
    
    /**
      Called when Akonadi collections loaded
    */
    void collectionsReceived( const Akonadi::Collection::List & list );
    
private:

    enum IncidentType {
        Event,
        Todo
    };

private:

    Plasma::QueryMatch createQueryMatch( const QString & definition, IncidentType type );

    void describeSyntaxes();

private:

    DateTimeParser dateTimeParser;
    
    Akonadi::Collection eventsCollection;
    Akonadi::Collection todoCollection;
    
    KIcon icon;
};

#endif
