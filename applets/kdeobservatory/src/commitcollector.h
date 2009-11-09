#ifndef COMMITCOLLECTOR_HEADER
#define COMMITCOLLECTOR_HEADER

#include "icollector.h"
#include "kdeobservatory.h"

class CommitCollector : public ICollector
{
    Q_OBJECT
public:
    CommitCollector(const QMap<QString, KdeObservatory::Project> &projects, QObject *parent = 0);
    virtual ~CommitCollector();

    virtual void run();

    void setExtent (int extent);
    int extent() const;

    const QList< QPair <QString, int> > &resultingCommits() const;
    const QHash<QString, QList< QPair<QString, int> > > &resultingCommiters() const;

protected Q_SLOTS:
    virtual void requestFinished (int id, bool error);

private:
    void createSortedLists();

    int m_extent;
    int m_page;
    int m_connectId;
    long long m_stopCollectingDay;
    QHttpRequestHeader m_header;
    QString m_archiveName;
    const QMap<QString, KdeObservatory::Project> &m_projects;

    QHash<QString, int> m_tempCommits;
    QList< QPair <QString, int> > m_resultingCommits;

    QHash<QString, QHash<QString, int> > m_tempCommiters;
    QHash<QString, QList< QPair<QString, int> > > m_resultingCommiters;
};

#endif
