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
#include <QHash>
#include <KIcon>
#include <KJob>

#include "config-lancelot-datamodels.h"

#ifdef LANCELOT_DATAMODELS_HAS_PIMLIBS
#include <Akonadi/Entity>
#include <Akonadi/Monitor>
#include <Akonadi/Collection>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionFetchScope>
#include <Akonadi/ServerManager>
#endif

namespace Lancelot {
namespace Models {

class MessagesKmail::Private: public QObject {
    Q_OBJECT

public:
    Private(MessagesKmail * parent);

#ifdef LANCELOT_DATAMODELS_HAS_PIMLIBS
    QHash < KJob *, Akonadi::Collection > collectionJobs;
    Akonadi::Monitor * monitor;
    int unread;

    KIcon entityIcon(const Akonadi::Collection & collection) const;

public Q_SLOTS:
    void fetchEmailCollectionsDone(KJob * job);

#endif

private:
    MessagesKmail * const q;
};

}
}
