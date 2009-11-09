#include "kdeobservatorydatabase.h"

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
    m_query.prepare("delete from commits where commit_date <= :date");
    m_query.bindValue(":commit_date", date);
    if (!m_query.exec())
        kDebug() << "Error when deleting old commits -" << m_db.lastError();
}

int KdeObservatoryDatabase::commitsByProject(const QString &prefix)
{
    m_query.clear();
    m_query.prepare("select count(*) from commits where subject like '" + prefix + "%'");
    if (!m_query.exec())
    {
        qDebug() << "Error when executing commits by project -" << m_db.lastError();
        return 0;
    }
    m_query.next();
    return m_query.value(0).toInt();
}

QMultiMap<int, QString> KdeObservatoryDatabase::developersByProject(const QString &prefix)
{
    m_query.clear();
    m_query.prepare("select count(*), developer from commits where subject like '" + prefix + "%' group by developer order by count(*) desc;");
    QMultiMap<int, QString> result;
    if (!m_query.exec())
    {
        qDebug() << "Error when executing commits by project -" << m_db.lastError();
        return result;
    }
    while(m_query.next())
        result.insert(m_query.value(0).toInt(), m_query.value(1).toString());
    return result;
}

KdeObservatoryDatabase::KdeObservatoryDatabase()
: m_db(QSqlDatabase::addDatabase("QSQLITE"))
{
    qDebug() << KStandardDirs::locate("data", "kdeobservatory/data/kdeobservatory.db");
    m_db.setDatabaseName(KStandardDirs::locate("data", "kdeobservatory/data/kdeobservatory.db"));
    if (!m_db.open())
    {
        qDebug() << "Erro ao abrir o banco de dados";
    }
}
