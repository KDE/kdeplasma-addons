#ifndef COMMITCOLLECTOR_HEADER
#define COMMITCOLLECTOR_HEADER

#include "icollector.h"

#include <QStack>

class CommitCollector : public ICollector
{
    Q_OBJECT
public:
    CommitCollector(QObject *parent = 0);
    virtual ~CommitCollector();

    void setExtent (int extent);
    int extent() const;

    void setCommitsRead(int commitsRead);
    int commitsRead() const;

    void setFullUpdate(bool fullUpdate);

    virtual void run();

    typedef struct
    {
        QString date;
        QString subject;
        QString developer;
    } Commit;

protected Q_SLOTS:
    virtual void requestFinished (int id, bool error);

private:
    bool m_fullUpdate;
    int  m_commitsRead;
    QString m_lastArchiveRead;
    int  m_commitsToBeRead;

    int m_extent;
    int m_page;
    int m_connectId;
    long long m_stopCollectingDay;
    long long m_lastDay;
    int m_dayCounter;

    QStack<Commit> m_commits;
    QHttpRequestHeader m_header;
    QString m_archiveName;
    QString m_initialArchiveName;
};

#endif
