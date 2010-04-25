/*************************************************************************
 * Copyright 2009 Sandro Andrade sandroandrade@kde.org                   *
 *                                                                       *
 * This program is free software; you can redistribute it and/or         *
 * modify it under the terms of the GNU General Public License as        *
 * published by the Free Software Foundation; either version 2 of        *
 * the License or (at your option) version 3 or any later version        *
 * accepted by the membership of KDE e.V. (or its successor approved     *
 * by the membership of KDE e.V.), which shall act as a proxy            *
 * defined in Section 14 of version 3 of the license.                    *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 * ***********************************************************************/

#include "kdeobservatorydatabase.h"

#include <QFile>
#include <QVariant>
#include <QSqlError>

#include <KDebug>
#include <KGlobal>
#include <KStandardDirs>

KdeObservatoryDatabase *KdeObservatoryDatabase::m_kdeObservatoryDatabase = 0;

KdeObservatoryDatabase::~KdeObservatoryDatabase()
{
    if (m_kdeObservatoryDatabase)
        m_db.close();
}

void KdeObservatoryDatabase::beginTransaction()
{
    m_db.transaction();
}

void KdeObservatoryDatabase::commitTransaction()
{
    m_db.commit();
}

KdeObservatoryDatabase *KdeObservatoryDatabase::self()
{
    if (!m_kdeObservatoryDatabase)
        m_kdeObservatoryDatabase = new KdeObservatoryDatabase();
    return m_kdeObservatoryDatabase;
}

void KdeObservatoryDatabase::addCommit(const QString &date, const QString &subject, const QString &developer)
{
    m_query.clear();
    m_query.prepare("INSERT INTO commits (insert_date, commit_date, subject, developer) VALUES (datetime('now'), :commit_date, :subject, :developer)");
    m_query.bindValue(":commit_date", date);
    m_query.bindValue(":subject", subject);
    m_query.bindValue(":developer", developer);
    if (!m_query.exec())
        kDebug() << "Error when inserting into table commits -" << m_db.lastError();
}

void KdeObservatoryDatabase::truncateCommits()
{
    m_query.clear();
    m_query.prepare("delete from commits");
    if (!m_query.exec())
        kDebug() << "Error when truncating table commits -" << m_db.lastError();
}

void KdeObservatoryDatabase::deleteOldCommits(const QString &date)
{
    m_query.clear();
    m_query.prepare("delete from commits where commit_date <= '" + date + '\'');
    if (!m_query.exec())
        kDebug() << "Error when deleting old commits -" << m_db.lastError();
}

int KdeObservatoryDatabase::commitsByProject(const QString &prefix)
{
    m_query.clear();
    m_query.prepare("select count(*) from commits where subject like '%" + prefix + "%'");
    if (!m_query.exec())
    {
        kDebug() << "Error when executing commits by project -" << m_db.lastError();
        return 0;
    }
    m_query.next();
    return m_query.value(0).toInt();
}

QMultiMap<int, QString> KdeObservatoryDatabase::developersByProject(const QString &prefix)
{
    m_query.clear();
    m_query.prepare("select count(*), developer from commits where subject like '%" + prefix + "%' group by developer order by count(*) desc");
    QMultiMap<int, QString> result;
    if (!m_query.exec())
    {
        kDebug() << "Error when executing commits by project -" << m_db.lastError();
        return result;
    }
    while(m_query.next())
        result.insert(m_query.value(0).toInt(), m_query.value(1).toString());
    return result;
}

QList< QPair<QString, int> > KdeObservatoryDatabase::commitHistory(const QString &prefix)
{
    m_query.clear();
    m_query.prepare("select commit_date, count(*) from commits where subject like '%" + prefix + "%' group by commit_date order by commit_date");
    QList< QPair<QString, int> > result;
    if (!m_query.exec())
    {
        kDebug() << "Error when executing commit history by project -" << m_db.lastError();
        return result;
    }
    while(m_query.next())
        result.append(QPair<QString, int>(m_query.value(0).toString(), m_query.value(1).toInt()));
    return result;
}

void KdeObservatoryDatabase::addKrazyError(const QString &project, const QString &fileType, const QString &testName, const QString &fileName, const QString &error)
{
    m_query.clear();
    m_query.prepare("INSERT INTO krazy_errors (project, file_type, test_name, file_name, error) VALUES (:project, :file_type, :test_name, :file_name, :error)");
    m_query.bindValue(":project", project);
    m_query.bindValue(":file_type", fileType);
    m_query.bindValue(":test_name", testName);
    m_query.bindValue(":file_name", fileName);
    m_query.bindValue(":error", error);
    if (!m_query.exec())
        kDebug() << "Error when inserting into table commits -" << m_db.lastError();
}

void KdeObservatoryDatabase::truncateKrazyErrors()
{
    m_query.clear();
    m_query.prepare("delete from krazy_errors");
    if (!m_query.exec())
        kDebug() << "Error when truncating table commits -" << m_db.lastError();
}

QMap<QString, QMultiMap<int, QString> > KdeObservatoryDatabase::krazyErrorsByProject(const QString &project)
{
    m_query.clear();
    m_query.prepare("select file_type, test_name, count(*) from krazy_errors where project = '" + project + "' group by file_type, test_name");
    QMap<QString, QMultiMap<int, QString> > result;
    if (!m_query.exec())
    {
        kDebug() << "Error when executing krazy errors by project -" << m_db.lastError();
        return result;
    }
    while(m_query.next())
        result[m_query.value(0).toString()].insert(m_query.value(2).toInt(), m_query.value(1).toString());
    return result;
}

QStringList KdeObservatoryDatabase::krazyFilesByProjectTypeAndTest(const QString &project, const QString &fileType, const QString &testName)
{
    m_query.clear();
    m_query.prepare("select file_name, error from krazy_errors where project = '" + project + "' and file_type = '" + fileType + "' and test_name = '" + testName + '\'');
    QStringList result;
    if (!m_query.exec())
    {
        kDebug() << "Error when executing krazy files by project '" + project +"', type '" + fileType + "', and test '" + testName + "' -" << m_db.lastError();
        return result;
    }
    while(m_query.next())
        result.append(m_query.value(0).toString() + ": " + m_query.value(1).toString());
    return result;
}

KdeObservatoryDatabase::KdeObservatoryDatabase()
: m_db(QSqlDatabase::addDatabase("QSQLITE"))
{
    const QString dbPath = KStandardDirs::locateLocal("data", "kdeobservatory/data/kdeobservatory.db");
    bool databaseExists = QFile(dbPath).exists();
    m_db.setDatabaseName(dbPath);
    if (!m_db.open())
    {
        kDebug() << "Error when opening database";
    }
    else
    {
        m_query.setForwardOnly(true);
        if (!databaseExists)
            init();
    }
}

void KdeObservatoryDatabase::init()
{
    m_query.clear();
    m_query.prepare("CREATE TABLE commits (insert_date datetime, commit_date date, subject text, developer text)");
    if (!m_query.exec())
        kDebug() << "Error when creating database -" << m_db.lastError();
    m_query.clear();
    m_query.prepare("CREATE TABLE krazy_errors (project text, file_type text, test_name text, file_name text, error text)");
    if (!m_query.exec())
        kDebug() << "Error when creating database -" << m_db.lastError();
}
