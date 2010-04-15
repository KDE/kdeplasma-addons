/*
 *   Copyright (C) 2007, 2008, 2009, 2010 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser/Library General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser/Library General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser/Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "MessagesKmail.h"
#include <KIcon>

#include <KRun>
#include <KStandardDirs>

#include "Logger.h"

#include "config-lancelot-datamodels.h"

#ifndef LANCELOT_DATAMODELS_HAS_PIMLIBS

    #define DummyModelClassName MessagesKmail
    #define DummyModelInit \
        setSelfTitle(i18n("Unread messages"));  \
        setSelfIcon(KIcon("kmail"));            \
        if (!addService("kontact") && !addService("kmail")) {   \
            add(i18n("Unable to find Kontact"), "",             \
                    KIcon("application-x-executable"),          \
                    QVariant("http://kontact.kde.org"));        \
        }

    #include "DummyModel_p.cpp"

    #undef DummyModelClassName
    #undef DummyModelInit

#else

// We have kdepimlibs

#include <KJob>
#include <Akonadi/Collection>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionStatistics>
#include <Akonadi/CollectionStatisticsJob>
#include <Akonadi/EntityDisplayAttribute>
#include "MessagesKmail_p.h"

namespace Lancelot {
namespace Models {

MessagesKmail::Private::Private(MessagesKmail * parent)
  : q(parent)
{
}

void MessagesKmail::Private::fetchEmailCollectionsDone(KJob * job)
{
    if ( job->error() ) {
        kDebug() << "Job Error:" << job->errorString();
        return;
    }

    Akonadi::CollectionFetchJob * cjob =
        static_cast < Akonadi::CollectionFetchJob * > ( job );

    foreach (const Akonadi::Collection & collection, cjob->collections()) {
        if (collection.contentMimeTypes().contains("message/rfc822")) {
            Akonadi::CollectionStatisticsJob * job =
                new Akonadi::CollectionStatisticsJob(collection);
            connect(job, SIGNAL(result(KJob*)),
                    this, SLOT(fetchCollectionStatisticsDone(KJob*)));

            collectionJobs[job] = collection;

        }
    }
}

QString MessagesKmail::Private::entityName(const Akonadi::Collection & collection) const
{
    Akonadi::EntityDisplayAttribute * displayAttribute =
        static_cast < Akonadi::EntityDisplayAttribute * > (
            collection.attribute < Akonadi::EntityDisplayAttribute > ()
        );

    if (displayAttribute) {
        return displayAttribute->displayName();
    }

    return collection.name();
}

KIcon MessagesKmail::Private::entityIcon(const Akonadi::Collection & collection) const
{
    Akonadi::EntityDisplayAttribute * displayAttribute =
        static_cast < Akonadi::EntityDisplayAttribute * > (
            collection.attribute < Akonadi::EntityDisplayAttribute > ()
        );

    if (displayAttribute) {
        return displayAttribute->icon();
    }

    return KIcon("mail-folder-inbox");
}

QString MessagesKmail::Private::entityPath(const Akonadi::Entity & entity) const
{
    QString path;

    Akonadi::Collection collection = entity.parentCollection();
        kDebug() << path << collection.remoteId() << collection.url() << collection.parent();

    while (collection.isValid()) {
        path = entityName(collection) + "/" + path;
        kDebug() << path << collection.remoteId() << collection.url();
        collection = collection.parentCollection();
    }

    return path;
}

void MessagesKmail::Private::fetchCollectionStatisticsDone(KJob * job)
{
    if ( job->error() ) {
        kDebug() << "Job Error:" << job->errorString();
        return;
    }

    Akonadi::CollectionStatisticsJob * statisticsJob = qobject_cast < Akonadi::CollectionStatisticsJob * > (job);

    const Akonadi::CollectionStatistics statistics = statisticsJob->statistics();
    Akonadi::Collection & collection = collectionJobs[job];

    if (statistics.unreadCount()) {
        q->add(
            i18nc("Directory name (number of unread messages)", "%1 (%2)")
                .arg(entityName(collection))
                .arg(QString::number(statistics.unreadCount())),
            QString::null,
            entityIcon(collection),
            collection.url()
            );
    }
}

MessagesKmail::MessagesKmail()
    : d(new Private(this))
{
    setSelfTitle(i18n("Unread messages"));
    setSelfIcon(KIcon("kmail"));
    load();
}

MessagesKmail::~MessagesKmail()
{
}

void MessagesKmail::activate(int index)
{
    Q_UNUSED(index);

    clear();
    load();
}

void MessagesKmail::load()
{
    kDebug();

    Akonadi::Collection emailCollection(Akonadi::Collection::root());
    emailCollection.setContentMimeTypes(QStringList() << "message/rfc822");

    Akonadi::CollectionFetchJob * fetch = new Akonadi::CollectionFetchJob(
            emailCollection, Akonadi::CollectionFetchJob::Recursive);

    connect(fetch, SIGNAL(result(KJob*)),
            d, SLOT(fetchEmailCollectionsDone(KJob*)));
}

} // namespace Models
} // namespace Lancelot

#endif

