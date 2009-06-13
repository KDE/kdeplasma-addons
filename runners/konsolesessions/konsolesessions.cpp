/*
 *   Copyright 2008 Montel Laurent <montel@kde.org>
 *   based on kate session from sebas
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

#include "konsolesessions.h"

#include <QFileInfo>
#include <KDirWatch>
#include <KDebug>
#include <KStandardDirs>
#include <KToolInvocation>
#include <KIcon>
#include <KConfig>
#include <KConfigGroup>
#include <kio/global.h>


KonsoleSessions::KonsoleSessions(QObject *parent, const QVariantList& args)
    : Plasma::AbstractRunner(parent, args)
{
    Q_UNUSED(args);
    setObjectName("Konsole Sessions");
    m_icon = KIcon("utilities-terminal");
    loadSessions();

    KDirWatch *historyWatch = new KDirWatch(this);
    const QStringList sessiondirs = KGlobal::dirs()->findDirs("data", "konsole/");
    foreach (const QString &dir, sessiondirs) {
        historyWatch->addDir(dir);
    }

    connect(historyWatch, SIGNAL(dirty(QString)), this,SLOT(loadSessions()));
    connect(historyWatch, SIGNAL(created(QString)), this,SLOT(loadSessions()));
    connect(historyWatch, SIGNAL(deleted(QString)), this,SLOT(loadSessions()));

    Plasma::RunnerSyntax s(":q:", i18n("Finds Konsole sessions matching :q:."));
    s.addExampleQuery("konsole :q:");
    addSyntax(s);

    addSyntax(Plasma::RunnerSyntax("konsole", i18n("Lists all the Konsole sessions in your account.")));
}

KonsoleSessions::~KonsoleSessions()
{
}

void KonsoleSessions::loadSessions()
{
    const QStringList list = KGlobal::dirs()->findAllResources("data", "konsole/*.profile", KStandardDirs::NoDuplicates);
    QStringList::ConstIterator end = list.constEnd();
    for (QStringList::ConstIterator it = list.constBegin(); it != end; ++it) {
        QFileInfo info(*it);
        const QString profileName = KIO::decodeFileName(info.baseName());

        QString niceName=profileName;
        //kDebug()<<" loadSessions :";
        KConfig _config(*it, KConfig::SimpleConfig);
        if (_config.hasGroup("General"))
        {
            KConfigGroup cfg(&_config, "General");
            if (cfg.hasKey("Name")) {
                niceName = cfg.readEntry("Name");
            }

            m_sessions.insert(profileName, niceName);
            //kDebug()<<" profileName :"<<profileName<<" niceName :"<<niceName;
        }
    }
}

void KonsoleSessions::match(Plasma::RunnerContext &context)
{
    if (m_sessions.isEmpty()) {
        return;
    }

    const QString term = context.query();
    if (term.length() < 3) {
        return;
    }

    if (term.toLower() == "konsole") {
        QHashIterator<QString, QString> i(m_sessions);
        while (i.hasNext()) {
            i.next();
            Plasma::QueryMatch match(this);
            match.setType(Plasma::QueryMatch::PossibleMatch);
            match.setRelevance(1.0);
            match.setIcon(m_icon);
            match.setData(i.key());
            match.setText("Konsole: " + i.value());
            context.addMatch(term, match);
        }
    } else {
        // we could just return here, but the kate hackers might have a session kate,
        // so everybody else will suffer. And rightfully so! ;-)
        QHashIterator<QString, QString> i(m_sessions);
        while (i.hasNext()) {
            if (!context.isValid()) {
                return;
            }

            i.next();
            if (i.value().contains(term, Qt::CaseInsensitive)) {
                Plasma::QueryMatch match(this);
                match.setType(Plasma::QueryMatch::PossibleMatch);
                match.setIcon(m_icon);
                match.setData(i.key());
                match.setText("Konsole: " + i.value());

                if (i.value().toLower() == term) {
                    match.setRelevance(1.0);
                } else {
                    match.setRelevance(0.6);
                }

                context.addMatch(term, match);
            }
        }
    }
}

void KonsoleSessions::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context)
    const QString session = match.data().toString();
    kDebug() << "Open Konsole Session " << session;

    if (!session.isEmpty()) {
        QStringList args;
        args << "--profile";
        args << session;
        kDebug() << "=== START: konsole" << args;
        KToolInvocation::kdeinitExec("konsole", args);
    }
}

#include "konsolesessions.moc"
