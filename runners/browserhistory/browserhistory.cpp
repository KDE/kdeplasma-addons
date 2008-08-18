/*
 *   Copyright 2008 Sebastian Kügler
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "browserhistory.h"

#include <QDBusInterface>
#include <QDBusReply>

#include <KDebug>
#include <KStandardDirs>
#include <KToolInvocation>
#include <KIcon>
#include <KConfig>
#include <KConfigGroup>

// Update history at most every 30 seconds
#define UPDATE_INTERVAL 30000

BrowserHistoryRunner::BrowserHistoryRunner(QObject *parent, const QVariantList& args)
    : Plasma::AbstractRunner(parent, args)
{
    Q_UNUSED(args);
    KGlobal::locale()->insertCatalog("krunner_browserhistoryrunner");
    setObjectName(i18n("Browser History"));
    m_icon = KIcon("view-history");
    // What is this about? The API doc isn't exactly clear ...
    //setIgnoredTypes(Plasma::RunnerContext::FileSystem);
    m_history = loadHistory();
}

BrowserHistoryRunner::~BrowserHistoryRunner()
{
}

QStringList BrowserHistoryRunner::loadHistory()
{
    m_time.restart();
    KConfig *konq_config = new KConfig("konq_history", KConfig::NoGlobals);
    KConfigGroup locationBarGroup( konq_config, "Location Bar" );
    QStringList lstHistory = locationBarGroup.readPathEntry( "ComboContents", QStringList() );
    delete konq_config;
    return lstHistory;
}

void BrowserHistoryRunner::match(Plasma::RunnerContext &context)
{
    const QString term = context.query();
    if (term.length() < 3) {
        return;
    }
    if (m_time.elapsed() > UPDATE_INTERVAL) {
        loadHistory();
    }
    foreach (const QString &historyitem, m_history) {
        // Filter out error pages, and match ...
        if (!historyitem.startsWith("error:/") && historyitem.contains(term, Qt::CaseInsensitive)) {
            Plasma::QueryMatch match(this);
            match.setType(Plasma::QueryMatch::PossibleMatch);
            match.setRelevance(0.5);
            match.setIcon(m_icon);
            match.setData(historyitem);
            QString text = historyitem;
            match.setText(text.replace("http://", ""));
            context.addMatch(term, match);
        }
    }
}

void BrowserHistoryRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context)
    QString location = match.data().toString();
    kDebug() << "Browse to " << location;
    if (!location.isEmpty()) {
        KToolInvocation::invokeBrowser(location);
    }
}

#include "browserhistory.moc"
