#include "kdeobservatorydatabase.h"

#include <QVariant>
#include <QSqlError>

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
