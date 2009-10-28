#ifndef COMMITCOLLECTOR_HEADER
#define COMMITCOLLECTOR_HEADER

#include "icollector.h"

class CommitCollector : public ICollector
{
    Q_OBJECT
public:
    CommitCollector(QObject *parent = 0);
    virtual ~CommitCollector();

    virtual void run();

    void setExtent (int extent);
    int extent() const;

protected Q_SLOTS:
    virtual void requestFinished (int id, bool error);

private:
    int m_extent;
    int m_page;
    int m_connectId;
    long long m_stopCollectingDay;
    QHttpRequestHeader m_header;
    QString m_archiveName;
};

#endif
