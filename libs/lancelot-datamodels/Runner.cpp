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

#include "Runner.h"

#include <KDebug>
#include <KRun>
#include <KIcon>
#include <KLocalizedString>
#include <KApplication>
#include <KStandardDirs>

#include <Plasma/AbstractRunner>

#include "Logger.h"
#include "FavoriteApplications.h"

#define SLEEP 200

namespace Lancelot {
namespace Models {

class Runner::Private {
public:
    QString searchString;
    QString runnerName;
    QBasicTimer timer;
    Plasma::RunnerManager * runnerManager;
    bool valid : 1;
};

Runner::Runner(bool limitRunners, QString search)
    : d(new Private())
{
    d->searchString = search;
    d->valid = false;
    d->runnerManager = new Plasma::RunnerManager(this);

    if (limitRunners) {
        QStringList allowed;
        allowed
            << "places"
            << "shell"
            << "services"
            << "bookmarks"
            << "recentdocuments"
            << "locations";
        d->runnerManager->setAllowedRunners(allowed);
    }

    connect(
        d->runnerManager, SIGNAL(matchesChanged(QList<Plasma::QueryMatch>)),
        this, SLOT(setQueryMatches(QList<Plasma::QueryMatch>))
    );
    setSearchString(search);
}

Runner::Runner(QStringList allowedRunners, QString search)
    : d(new Private())
{
    d->searchString = search;
    kDebug() << "init.";
    d->valid = false;
    d->runnerManager = new Plasma::RunnerManager(this);
    d->runnerManager->setAllowedRunners(allowedRunners);

    connect(
        d->runnerManager, SIGNAL(matchesChanged(QList<Plasma::QueryMatch>)),
        this, SLOT(setQueryMatches(QList<Plasma::QueryMatch>))
    );
    setSearchString(search);
}

Runner::~Runner()
{
    delete d;
}

void Runner::reloadConfiguration()
{
    d->runnerManager->reloadConfiguration();
}

QString Runner::searchString() const
{
    return d->searchString;
}

QString Runner::runnerName() const
{
    return d->runnerName;
}

void Runner::setRunnerName(const QString & name)
{
    d->runnerName = name;
}

void Runner::setSearchString(const QString & search)
{
    d->searchString = search.trimmed();
    clear();

    if (d->searchString.isEmpty()) {
        add(
            i18n("Search string is empty"),
            i18n("Enter something to search for"),
            KIcon("help-hint"),
            QVariant()
        );
        d->valid = false;
    } else {
        add(
            i18n("Searching..."),
            i18n("Some searches can take longer to complete"),
            KIcon("help-hint"),
            QVariant()
        );
        d->valid = false;
    }

    d->timer.start(SLEEP, this);
}

void Runner::timerEvent(QTimerEvent * event)
{
    BaseModel::timerEvent(event);
    if (event->timerId() != d->timer.timerId()) {
        return;
    }

    d->timer.stop();

    if (!d->searchString.isEmpty()) {
        d->runnerManager->reset();

        if (d->runnerName.isEmpty()) {
            d->runnerManager->launchQuery(d->searchString);
        } else {
            d->runnerManager->launchQuery(d->searchString, d->runnerName);
        }
    }
}

// Code taken from KRunner Runner::setQueryMatches
void Runner::setQueryMatches(const QList< Plasma::QueryMatch > & m)
{
    // kDebug() << m.size();
    setEmitInhibited(true);
    clear();

    if (m.count() == 0) {
        add(
            i18n("No matches found"),
            i18n("No matches found for current search"),
            KIcon("help-hint"),
            QVariant()
        );

        d->valid = false;
    } else {
        QList < Plasma::QueryMatch > matches = m;
        qSort(matches.begin(), matches.end());

        while (matches.size()) {
            Plasma::QueryMatch match = matches.takeLast();
            QStringList data;
            data << match.id();
            data << match.runner()->id();
            data << match.data().toString();

            add(
                match.text(),
                match.subtext(),
                match.icon(),
                data
            );
        }
        d->valid = true;
    }
    setEmitInhibited(false);
    emit updated();
}

void Runner::load()
{
}

void Runner::activate(int index)
{
    if (!d->valid) return;
    QString data = itemAt(index).data.value< QStringList >().at(0);
    Logger::self()->log("run-model", data);
    d->runnerManager->run(data);
    // d->runnerManager->reset();
    // changeApplicationSearchString(QString());
    hideApplicationWindow();
}

bool Runner::hasContextActions(int index) const
{
    if (!d->valid) return false;

    if (itemAt(index).data.value< QStringList >().at(1) == "services") {
        return true;
    }

    QString id = itemAt(index).data.value< QStringList >().at(0);
    foreach (const Plasma::QueryMatch &match, d->runnerManager->matches()) {
        if (match.id() == id) {
            if (d->runnerManager->actionsForMatch(match).size() > 0) {
                return true;
            }
        }
    }

    return false;
}

void Runner::setContextActions(int index, Lancelot::PopupMenu * menu)
{
    if (!d->valid) return;

    if (itemAt(index).data.value< QStringList >().at(1) == "services") {
        menu->addAction(KIcon("list-add"), i18n("Add to Favorites"))
            ->setData(QVariant(0));
    }

    QString id = itemAt(index).data.value< QStringList >().at(0);
    foreach (const Plasma::QueryMatch &match, d->runnerManager->matches()) {
        if (match.id() == id) {
            foreach (QAction * action, d->runnerManager->actionsForMatch(match)) {
                menu->addAction(action->icon(), action->text());
            }
        }
    }
}

void Runner::contextActivate(int index, QAction * context)
{
    if (!d->valid || !context) return;

    if (context->data().toInt() == 0) {
        KService::Ptr service = KService::serviceByStorageId(
                itemAt(index).data.value< QStringList >().at(2));
        if (service) {
            FavoriteApplications::self()
                ->addFavorite(service->entryPath());
        }
    }
}

QMimeData * Runner::mimeData(int index) const
{
    if (!d->valid) return NULL;

    // if (itemAt(index).data.value< QStringList >().at(1) == "services") {
    //     KService::Ptr service = KService::serviceByStorageId(
    //             itemAt(index).data.value< QStringList >().at(2));
    //     return BaseModel::mimeForService(service);
    // } else {
        QString data = itemAt(index).data.value< QStringList >().at(0);
        kDebug() << data;

        QMimeData * result = d->runnerManager->mimeDataForMatch(data);
        kDebug() << result;
        return result;
    // }

    return NULL;
}

void Runner::setDropActions(int index,
            Qt::DropActions & actions, Qt::DropAction & defaultAction)
{
    Q_UNUSED(index);
    actions = Qt::CopyAction;
    defaultAction = Qt::CopyAction;
}

} // namespace Models
} // namespace Lancelot
