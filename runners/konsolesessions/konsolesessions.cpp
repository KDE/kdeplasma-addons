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

// KF
#include <KDirWatch>
#include <KToolInvocation>
#include <KLocalizedString>
#include <KConfig>
// Qt
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>


KonsoleSessions::KonsoleSessions(QObject *parent, const QVariantList& args)
    : Plasma::AbstractRunner(parent, args)
{
    setObjectName(QStringLiteral("Konsole Sessions"));

    setIgnoredTypes(Plasma::RunnerContext::File | Plasma::RunnerContext::Directory | Plasma::RunnerContext::NetworkLocation);

    Plasma::RunnerSyntax s(QStringLiteral( ":q:" ), i18n("Finds Konsole sessions matching :q:."));
    s.addExampleQuery(QStringLiteral( "konsole :q:" ));
    addSyntax(s);

    setDefaultSyntax(Plasma::RunnerSyntax(QStringLiteral( "konsole" ), i18n("Lists all the Konsole sessions in your account.")));

    connect(this, &Plasma::AbstractRunner::prepare, this, &KonsoleSessions::slotPrepare);
    connect(this, &Plasma::AbstractRunner::teardown, this, &KonsoleSessions::slotTeardown);
}

KonsoleSessions::~KonsoleSessions()
{
}

void KonsoleSessions::slotPrepare()
{
    loadSessions();

    if (!m_sessionWatch) {
        m_sessionWatch = new KDirWatch(this);
        const QStringList sessiondirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("konsole"), QStandardPaths::LocateDirectory);
        foreach (const QString &dir, sessiondirs) {
            m_sessionWatch->addDir(dir);
        }

        connect(m_sessionWatch, &KDirWatch::dirty, this, &KonsoleSessions::loadSessions);
        connect(m_sessionWatch, &KDirWatch::created, this, &KonsoleSessions::loadSessions);
        connect(m_sessionWatch, &KDirWatch::deleted, this, &KonsoleSessions::loadSessions);
    }
}

void KonsoleSessions::slotTeardown()
{
    delete m_sessionWatch;
    m_sessionWatch = nullptr;
    m_sessions.clear();
}

void KonsoleSessions::loadSessions()
{
    m_sessions.clear();

    QStringList profilesPaths;
    const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("konsole"), QStandardPaths::LocateDirectory);

    for (const auto& dir : dirs) {
        const QStringList fileNames = QDir(dir).entryList({QStringLiteral("*.profile")});
        for (const QString& fileName : fileNames) {
            profilesPaths.append(dir + QLatin1Char('/') + fileName);
        }
    }

    for (const auto& profilePath : qAsConst(profilesPaths)) {
        QFileInfo info(profilePath);
        const QString profileName = info.baseName();

        QString niceName=profileName;
        KConfig _config(profilePath, KConfig::SimpleConfig);
        if (_config.hasGroup("General"))
        {
            KConfigGroup cfg(&_config, "General");
            if (cfg.hasKey("Name")) {
                niceName = cfg.readEntry("Name");
            }

            m_sessions.insert(profileName, niceName);
        }
    }
}

void KonsoleSessions::match(Plasma::RunnerContext &context)
{
    if (m_sessions.isEmpty()) {
        return;
    }

    QString term = context.query();
    if (term.length() < 3) {
        return;
    }

    if (term.compare(QLatin1String( "konsole" ), Qt::CaseInsensitive) == 0) {
        QHashIterator<QString, QString> i(m_sessions);
        while (i.hasNext()) {
            i.next();
            Plasma::QueryMatch match(this);
            match.setType(Plasma::QueryMatch::PossibleMatch);
            match.setRelevance(1.0);
            match.setIconName(QStringLiteral("utilities-terminal"));
            match.setData(i.key());
            match.setText(QLatin1String( "Konsole: " ) + i.value());
            context.addMatch(match);
        }
    } else {
        if (term.startsWith(QLatin1String("konsole "), Qt::CaseInsensitive)) {
            term.remove(0, 8);
        }
        QHashIterator<QString, QString> i(m_sessions);
        while (i.hasNext()) {
            if (!context.isValid()) {
                return;
            }

            i.next();

            if (i.value().contains(term, Qt::CaseInsensitive)) {
                Plasma::QueryMatch match(this);
                match.setType(Plasma::QueryMatch::PossibleMatch);
                match.setIconName(QStringLiteral("utilities-terminal"));
                match.setData(i.key());
                match.setText(QLatin1String( "Konsole: " ) + i.value());

                if (i.value().compare(term, Qt::CaseInsensitive) == 0) {
                    match.setRelevance(1.0);
                } else {
                    match.setRelevance(0.6);
                }

                context.addMatch(match);
            }
        }
    }
}

void KonsoleSessions::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context)
    const QString session = match.data().toString();

    if (!session.isEmpty()) {
        return;
    }

    const QStringList args {
        QStringLiteral("--profile"),
        session
    };
    KToolInvocation::kdeinitExec(QStringLiteral("konsole"), args);
}


K_EXPORT_PLASMA_RUNNER(konsolesessions, KonsoleSessions)

#include "konsolesessions.moc"
