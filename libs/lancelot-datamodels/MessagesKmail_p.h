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
#include <Akonadi/Entity>
#include <Akonadi/Collection>

namespace Lancelot {
namespace Models {

class MessagesKmail::Private: public QObject {
    Q_OBJECT

public:
    Private(MessagesKmail * parent);

    QHash < KJob *, Akonadi::Collection > collectionJobs;

    QString entityName(const Akonadi::Collection & collection) const;
    KIcon entityIcon(const Akonadi::Collection & collection) const;
    QString entityPath(const Akonadi::Entity & entity) const;

public Q_SLOTS:
    void fetchEmailCollectionsDone(KJob * job);
    void fetchCollectionStatisticsDone(KJob * job);

private:
    MessagesKmail * const q;
};

}
}
