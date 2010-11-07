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

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusConnectionInterface>

#include <KIcon>
#include <KRun>
#include <KToolInvocation>
#include <KStandardDirs>

#include "Logger.h"

#include "config-lancelot-datamodels.h"

#ifndef LANCELOT_DATAMODELS_HAS_PIMLIBS

#warning "Pimlibs are not present"

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
  : monitor(NULL), unread(0), q(parent)
{
}

void MessagesKmail::Private::fetchEmailCollectionsDone(KJob * job)
{
    q->setEmitInhibited(true);

    if ( job->error() ) {
        kDebug() << "Job Error:" << job->errorString();

    } else {
        Akonadi::CollectionFetchJob * cjob =
            static_cast < Akonadi::CollectionFetchJob * > ( job );

        foreach (const Akonadi::Collection & collection, cjob->collections()) {
            if (collection.contentMimeTypes().contains("message/rfc822")) {
                int unreadCount = collection.statistics().unreadCount();

                if (unreadCount) {
                    q->add(
                            i18nc("Directory name (number of unread messages)",
                                "%1 (%2)",
                                collection.name(),
                                unreadCount),
                            QString::null,
                            entityIcon(collection),
                            collection.url()
                        );
                    unread += unreadCount;
                }
            }
        }

    }

    if (q->size() == 0) {
        q->add(i18n("No unread mail"), "", KIcon("mail-folder-inbox"), QVariant());
    }

    q->setEmitInhibited(false);
    q->updated();
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

MessagesKmail::MessagesKmail()
    : d(new Private(this))
{
    setSelfTitle(i18n("Unread messages"));
    setSelfIcon(KIcon("kmail"));

    d->monitor = new Akonadi::Monitor();
    d->monitor->setCollectionMonitored(Akonadi::Collection::root());
    d->monitor->fetchCollection(true);

    connect(d->monitor, SIGNAL(collectionAdded(const Akonadi::Collection &, const Akonadi::Collection &)),
            this, SLOT(update()));
    connect(d->monitor, SIGNAL(collectionRemoved(const Akonadi::Collection &)),
            this, SLOT(update()));
    connect(d->monitor, SIGNAL(collectionChanged(const Akonadi::Collection &)),
            this, SLOT(update()));
    connect(d->monitor, SIGNAL(collectionStatisticsChanged(Akonadi::Collection::Id, const Akonadi::CollectionStatistics &)),
            this, SLOT(update()));

    load();
}

MessagesKmail::~MessagesKmail()
{
    delete d->monitor;
    delete d;
}

void MessagesKmail::update()
{
    clear();
    load();
}

void MessagesKmail::activate(int index)
{
    Q_UNUSED(index);

    if (QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kmail")) {
        QDBusInterface kmailInterface("org.kde.kmail", "/KMail", "org.kde.kmail.kmail");
        kmailInterface.call("openReader");
    } else {
        KToolInvocation::startServiceByDesktopName("kmail");
    }
}

QString MessagesKmail::selfShortTitle() const
{
    if (d->unread) {
        return QString::number(d->unread);
    } else {
        return QString();
    }
}

void MessagesKmail::load()
{
    kDebug();

    d->unread = 0;

    Akonadi::Collection emailCollection(Akonadi::Collection::root());
    emailCollection.setContentMimeTypes(QStringList() << "message/rfc822");

    Akonadi::CollectionFetchScope scope;
    scope.setIncludeStatistics(true);
    scope.setContentMimeTypes(QStringList() << "message/rfc822");
    scope.setAncestorRetrieval(Akonadi::CollectionFetchScope::All);

    Akonadi::CollectionFetchJob * fetch = new Akonadi::CollectionFetchJob(
            emailCollection, Akonadi::CollectionFetchJob::Recursive);
    fetch->setFetchScope(scope);

    connect(fetch, SIGNAL(result(KJob*)),
            d, SLOT(fetchEmailCollectionsDone(KJob*)));
}

} // namespace Models
} // namespace Lancelot

#endif

