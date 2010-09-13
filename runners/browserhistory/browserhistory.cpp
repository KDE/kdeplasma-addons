/*
 *   Copyright 2008 Sebastian Kügler <sebas@kde.org>
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


#include <KDebug>
#include <KStandardDirs>
#include <KDirWatch>
#include <KToolInvocation>
#include <KIcon>
#include <KConfig>
#include <KConfigGroup>


BrowserHistoryRunner::BrowserHistoryRunner(QObject *parent, const QVariantList& args)
    : Plasma::AbstractRunner(parent, args)
{
    Q_UNUSED(args);
    setObjectName(QLatin1String( "Browser History" ));
    m_icon = KIcon(QLatin1String( "view-history" ));

    loadHistory();

    // listen for changes to the list of recent documents
    KDirWatch *historyWatch = new KDirWatch(this);
    historyWatch->addFile(KStandardDirs::locate("config", QLatin1String( "konq_history" )));
    connect(historyWatch,SIGNAL(dirty(QString)),this,SLOT(loadHistory()));
    connect(historyWatch,SIGNAL(created(QString)),this,SLOT(loadHistory()));
    connect(historyWatch,SIGNAL(deleted(QString)),this,SLOT(loadHistory()));

    addSyntax(Plasma::RunnerSyntax(QLatin1String( ":q:" ), i18n("Finds web sites you have visited matching :q:.")));
}

BrowserHistoryRunner::~BrowserHistoryRunner()
{
}

void BrowserHistoryRunner::loadHistory()
{
    KConfig *konq_config = new KConfig(QLatin1String( "konq_history" ), KConfig::NoGlobals);
    KConfigGroup locationBarGroup( konq_config, "Location Bar" );
    QStringList lstHistory = locationBarGroup.readPathEntry( "ComboContents", QStringList() );
    delete konq_config;

    QMutableStringListIterator it(lstHistory);
    while (it.hasNext()) {
        if (it.next().startsWith(QLatin1String("error:/"))) {
            it.remove();
        }
    }

    m_history = lstHistory;
}

void BrowserHistoryRunner::match(Plasma::RunnerContext &context)
{
    const QString term = context.query();
    if (term.length() < 3) {
        return;
    }

    foreach (const QString &historyitem, m_history) {
        if (!context.isValid()) {
            return;
        }

        // Filter out error pages, and match ...
        if (historyitem.contains(term, Qt::CaseInsensitive)) {
            Plasma::QueryMatch match(this);
            match.setType(Plasma::QueryMatch::PossibleMatch);
            match.setRelevance(0.5);
            match.setIcon(m_icon);
            match.setData(historyitem);
            QString text = historyitem;
            text.remove(QLatin1String( "http://" ));
            text.remove(QLatin1String( "https://" ));
            match.setSubtext(text);
            text.remove(QRegExp(QLatin1String( "/.*" )));
            match.setText(text);
            context.addMatch(term, match);
        }
    }
}

void BrowserHistoryRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context)
    const QString location = match.data().toString();
    kDebug() << "Browse to " << location;
    if (!location.isEmpty()) {
        KToolInvocation::invokeBrowser(location);
    }
}

#include "browserhistory.moc"
