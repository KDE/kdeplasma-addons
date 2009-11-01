#ifndef COMMITCOLLECTOR_HEADER
#define COMMITCOLLECTOR_HEADER

#include "icollector.h"

class QStandardItemModel;

class CommitCollector : public ICollector
{
    Q_OBJECT
public:
    CommitCollector(QStandardItemModel *projects, QObject *parent = 0);
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
    QString m_summary;
    QStandardItemModel *m_projects;

    QMap<QString, int> m_resultMap;
};

#endif
