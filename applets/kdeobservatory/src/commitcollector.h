#ifndef COMMITCOLLECTOR_HEADER
#define COMMITCOLLECTOR_HEADER

#include "icollector.h"
#include "kdeobservatory.h"

class CommitCollector : public ICollector
{
    Q_OBJECT
public:
    CommitCollector(QObject *parent = 0);
    virtual ~CommitCollector();

    void setExtent (int extent);
    int extent() const;

    virtual void run();

protected Q_SLOTS:
    virtual void requestFinished (int id, bool error);

private:
    bool m_fullUpdate;
    int m_commitsRead;

    int m_extent;
    int m_page;
    int m_connectId;
    long long m_stopCollectingDay;

    QHttpRequestHeader m_header;
    QString m_archiveName;
};

#endif
