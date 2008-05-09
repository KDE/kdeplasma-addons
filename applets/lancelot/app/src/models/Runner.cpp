/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
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
#include <KRun>
#include <KDebug>
#include <KLocalizedString>

#include <plasma/abstractrunner.h>

namespace Lancelot {
namespace Models {

Runner::Runner(QString search)
    : m_searchString(search)
{
    // m_runners = Plasma::AbstractRunner::load(this);

    load();
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
    load();
}

void Runner::load()
{/*
    m_items.clear();

    int matchCount = 0;

    if (m_searchString.isEmpty()) {
        add(
            "Search string is empty",
            "Enter something to search for",
            NULL,
            QVariant()
        );
        return;
    }

    m_context.resetSearchTerm(m_searchString);
    //m_context.addStringCompletions(m_executions);

    foreach (Plasma::AbstractRunner* runner, m_runners) {
        runner->match(&m_context);
    }

    QList < QList < Plasma::SearchMatch * > > matchLists;
    matchLists << m_context.matches();
    //           << m_context.exactMatches()
    //           << m_context.possibleMatches();

    foreach (QList < Plasma::SearchMatch * > matchList, matchLists) {
        foreach (Plasma::SearchMatch * action, matchList) {
            add(
                action->text(),
                action->runner()->objectName(),
                new KIcon(action->icon()),
                qVariantFromValue((void *)action)
            );

            ++matchCount;
        }
    }
*/}

void Runner::activate(int index)
{/*
    Plasma::SearchMatch * action = (Plasma::SearchMatch *)
        m_items[index].data.value< void * >();

    Q_ASSERT(action);

    if (!action->isEnabled()) {
        return;
    }

    if (action->type() == Plasma::SearchMatch::InformationalMatch) {
        changeLancelotSearchString(action->data().toString());
    } else {
        action->exec(&m_context);
        hideLancelotWindow();
    }
*/
}

} // namespace Models
} // namespace Lancelot
