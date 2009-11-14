#ifndef KRAZYCOLLECTOR_HEADER
#define KRAZYCOLLECTOR_HEADER

#include "icollector.h"
#include "kdeobservatory.h"

class KrazyCollector : public ICollector
{
    Q_OBJECT
public:
    KrazyCollector(const QMap<QString, KdeObservatory::Project> &projects, QObject *parent = 0);
    virtual ~KrazyCollector();

    void setExtent (int extent);
    int extent() const;

    virtual void run();

protected Q_SLOTS:
    virtual void requestFinished (int id, bool error);

private:
    void collectProject(const QString &project);

    int m_connectId;
    int m_projectsCollected;
    const QMap<QString, KdeObservatory::Project> &m_projects;
    QMap<int, QString> m_idProjectMap;
};

#endif
