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

#ifndef EVENTS_CONFIG_H
#define EVENTS_CONFIG_H

//Project-Includes
#include "ui_events_config.h"

//KDE-Includes
#include <KCModule>
#include <Akonadi/Collection>

//Qt

static const char RUNNER_NAME[] = "Events Runner";

static const char CONFIG_TODO_COLLECTION[] = "todoCollection";
static const char CONFIG_EVENT_COLLECTION[] = "eventCollection";

class CollectionSelector;

class EventsRunnerConfigForm : public QWidget, public Ui_EventsRunnerConfig
{
    Q_OBJECT

public:
    explicit EventsRunnerConfigForm(QWidget* parent);
};

class EventsRunnerConfig : public KCModule
{
    Q_OBJECT

public:
    explicit EventsRunnerConfig(QWidget* parent = 0, const QVariantList& args = QVariantList());

public slots:
    void save();
    void load();
    void defaults();

private:
    KConfigGroup config();

private slots:

    void collectionsReceived( CollectionSelector & selector );

private:
    EventsRunnerConfigForm* ui;
    Akonadi::Collection::List todoCollections;
    Akonadi::Collection::List eventCollections;
};
#endif
