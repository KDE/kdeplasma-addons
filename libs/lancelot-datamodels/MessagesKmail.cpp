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
#include <Akonadi/CollectionStatistics>
#include <Akonadi/CollectionFetchJob>
#include "MessagesKmail_p.h"

namespace Lancelot {
namespace Models {

void MessagesKmail::Private::fetchEmailCollectionsDone(KJob * job)
{
    if ( job->error() ) {
        kDebug() << "Job Error:" << job->errorString();

    } else {
        Akonadi::CollectionFetchJob * cjob =
            static_cast < Akonadi::CollectionFetchJob * > ( job );
        int i = 0;
        foreach (const Akonadi::Collection & collection, cjob->collections()) {
            if (collection.contentMimeTypes().contains("message/rfc822")) {
                kDebug() << collection.name() << collection.statistics().unreadCount();
            }
        }
    }
}


MessagesKmail::MessagesKmail()
    : d(new Private())
{
    load();
}

MessagesKmail::~MessagesKmail()
{
}

void MessagesKmail::activate(int index)
{
    Q_UNUSED(index)
}

void MessagesKmail::load()
{
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

