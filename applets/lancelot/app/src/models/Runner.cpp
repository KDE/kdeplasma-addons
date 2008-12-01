/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "Runner.h"
#include <KRun>
#include <KDebug>
#include <KIcon>
#include <KLocalizedString>
#include <KStandardDirs>
#include "FavoriteApplications.h"

#include <plasma/abstractrunner.h>

namespace Models {

Runner::Runner(QString search)
    : m_searchString(search), valid(false)
{
    m_runnerManager = new Plasma::RunnerManager(this);
    connect(
        m_runnerManager, SIGNAL(matchesChanged(const QList<Plasma::QueryMatch>&)),
        this, SLOT(setQueryMatches(const QList<Plasma::QueryMatch>&))
    );
    setSearchString(QString());
}

Runner::~Runner()
{
}

QString Runner::searchString()
{
    return m_searchString;
}

void Runner::setSearchString(const QString & search)
{
    m_searchString = search.trimmed();

    if (m_searchString.isEmpty()) {
        clear();
        add(
            i18n("Search string is empty"),
            i18n("Enter something to search for"),
            KIcon("help-hint"),
            QVariant()
        );
        valid = false;
    } else {
        m_runnerManager->launchQuery(search);
    }
}

// Code taken from KRunner Runner::setQueryMatches
void Runner::setQueryMatches(const QList< Plasma::QueryMatch > & m)
{
    setEmitInhibited(true);
    clear();

    if (m.count() == 0) {
        add(
            i18n("No matches found"),
            i18n("No matches found for current search"),
            KIcon("help-hint"),
            QVariant()
        );
        valid = false;
    } else {
        QList < Plasma::QueryMatch > matches = m;
        QMutableListIterator < Plasma::QueryMatch > newMatchIt(matches);

        while (newMatchIt.hasNext()) {
            Plasma::QueryMatch match = newMatchIt.next();
            kDebug() << match.id() << match.runner()->id() << match.runner()->objectName();
            QStringList data;
            data << match.id();
            data << match.runner()->id();
            data << match.data().toString();
            kDebug() << data;

            add(
                match.text(),
                match.subtext(),
                match.icon(),
                data
            );
        }
        valid = true;
    }
    setEmitInhibited(false);
    emit updated();
}

void Runner::load()
{
}

void Runner::activate(int index)
{
    if (!valid) return;
    m_runnerManager->run(m_items[index].data.value< QStringList >().at(0));
    m_runnerManager->reset();
    changeLancelotSearchString(QString());
    hideLancelotWindow();
}

bool Runner::hasContextActions(int index) const
{
    if (!valid) return false;
    kDebug() << m_items[index].data.value< QString >();
    return (m_items[index].data.value< QStringList >().at(1) == "Application");
}

void Runner::setContextActions(int index, QMenu * menu)
{
    if (!valid) return;

    if (m_items[index].data.value< QStringList >().at(1) == "Application") {
        menu->addAction(KIcon("list-add"), i18n("Add to Favorites"))
            ->setData(QVariant(0));
    }
}

void Runner::contextActivate(int index, QAction * context)
{
    if (!valid || !context) return;

    if (context->data().toInt() == 0) {
        KService::Ptr service = KService::serviceByStorageId(
                m_items[index].data.value< QStringList >().at(2));
        if (service) {
;           FavoriteApplications::instance()
                ->addFavorite(service->entryPath());
        }
    }
}

} // namespace Models
