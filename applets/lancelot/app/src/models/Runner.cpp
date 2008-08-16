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
        m_items.clear();
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
    m_items.clear();

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

        // first pass: we try and match up items with existing ids (match persisitence)
        while (newMatchIt.hasNext()) {
            Plasma::QueryMatch match = newMatchIt.next();
            add(
                match.text(),
                match.subtext(),
                match.icon(),
                match.id()
            );
        }
        valid = true;
    }
}

void Runner::load()
{
}

void Runner::activate(int index)
{
    if (!valid) return;
    m_runnerManager->run(m_items[index].data.value< QString >());
    changeLancelotSearchString(QString());
    hideLancelotWindow();
}

} // namespace Models
