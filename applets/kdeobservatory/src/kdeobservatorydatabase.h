#ifndef KDEOBSERVATORYDATABASE_HEADER
#define KDEOBSERVATORYDATABASE_HEADER

#include <QSqlQuery>
#include <QMultiMap>
#include <QSqlDatabase>

class KdeObservatoryDatabase
{
public:
    virtual ~KdeObservatoryDatabase();
    static KdeObservatoryDatabase *self();
    void addCommit(const QString &date, const QString &subject, const QString &developer);
    void truncateCommits();
    void deleteOldCommits(const QString &date);
    int commitsByProject(const QString &prefix);
    QMultiMap<int, QString> developersByProject(const QString &prefix);

private:
    KdeObservatoryDatabase();

    static KdeObservatoryDatabase *m_kdeObservatoryDatabase;
    QSqlDatabase m_db;
    QSqlQuery m_query;
};

#endif
