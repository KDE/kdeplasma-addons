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

//Project-Includes
#include "events_config.h"
#include "collection_selector.h"

//KDE-Includes
#include <Plasma/AbstractRunner>

K_EXPORT_RUNNER_CONFIG(events, EventsRunnerConfig)

using Akonadi::Collection;

EventsRunnerConfigForm::EventsRunnerConfigForm(QWidget* parent) : QWidget(parent) {
    setupUi( this );
}

EventsRunnerConfig::EventsRunnerConfig(QWidget* parent, const QVariantList& args): KCModule(ConfigFactory::componentData(), parent, args) {
    ui = new EventsRunnerConfigForm(this);

    QGridLayout* layout = new QGridLayout(this);
    layout->addWidget(ui, 0, 0);

    connect( ui->eventCollectionCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(changed()) );
    connect( ui->todoCollectionCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(changed()) );
}

void EventsRunnerConfig::defaults() {
    KCModule::defaults();
}

void EventsRunnerConfig::load() {
    KCModule::load();

    CollectionSelector * selector = new CollectionSelector( this );
    connect( selector, SIGNAL(collectionsReceived(CollectionSelector&)), this, SLOT(collectionsReceived(CollectionSelector&)) );
    selector->receiveCollections();
}

void EventsRunnerConfig::collectionsReceived( CollectionSelector& selector ) {
    KConfigGroup cfg = config();

    Collection::Id eventCollectionId = cfg.readEntry( CONFIG_EVENT_COLLECTION, (Collection::Id)0 );
    Collection::Id todoCollectionId = cfg.readEntry( CONFIG_TODO_COLLECTION, (Collection::Id)0 );

    ui->eventCollectionCombo->clear();
    ui->todoCollectionCombo->clear();

    foreach ( const Collection & collection, selector.eventCollections ) {
        ui->eventCollectionCombo->addItem( collection.name(), collection.id() );

        if ( collection.id() == eventCollectionId )
            ui->eventCollectionCombo->setCurrentIndex( ui->eventCollectionCombo->count() - 1 );
    }

    foreach ( const Collection & collection, selector.todoCollections ) {
        ui->todoCollectionCombo->addItem( collection.name(), collection.id() );

        if ( collection.id() == todoCollectionId )
            ui->todoCollectionCombo->setCurrentIndex( ui->todoCollectionCombo->count() - 1 );
    }

    selector.deleteLater();

    emit changed(false);
}

void EventsRunnerConfig::save() {
    KCModule::save();
    KConfigGroup cfg = config();

    cfg.writeEntry( CONFIG_EVENT_COLLECTION, ui->eventCollectionCombo->itemData( ui->eventCollectionCombo->currentIndex() ).toLongLong() );
    cfg.writeEntry( CONFIG_TODO_COLLECTION, ui->todoCollectionCombo->itemData( ui->todoCollectionCombo->currentIndex() ).toLongLong() );

    emit changed(true);
}

KConfigGroup EventsRunnerConfig::config() {
    KSharedConfig::Ptr cfg = KSharedConfig::openConfig("krunnerrc");
    KConfigGroup grp = cfg->group("Runners");
    return KConfigGroup(&grp, RUNNER_NAME);
}
