#ifndef KDEOBSERVATORYDATABASE_HEADER
#define KDEOBSERVATORYDATABASE_HEADER

#include <QPair>
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
    QList< QPair<QString, int> > commitHistory(const QString &prefix);

    void addKrazyError(const QString &project, const QString &fileType, const QString &testName, const QString &fileName, const QString &error);
    QMap<QString, QMultiMap<int, QString> > krazyErrorsByProject(const QString &project);
    QStringList krazyFilesByProjectTypeAndTest(const QString &project, const QString &fileType, const QString &testName);
    void truncateKrazyErrors();

private:
    KdeObservatoryDatabase();
    void init();

    static KdeObservatoryDatabase *m_kdeObservatoryDatabase;
    QSqlDatabase m_db;
    QSqlQuery m_query;
};

#endif
