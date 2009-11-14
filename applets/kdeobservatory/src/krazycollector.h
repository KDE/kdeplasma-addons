#ifndef KRAZYCOLLECTOR_HEADER
#define KRAZYCOLLECTOR_HEADER

#include "icollector.h"

class KrazyCollector : public ICollector
{
    Q_OBJECT
public:
    KrazyCollector(QObject *parent = 0);
    virtual ~KrazyCollector();

    void setExtent (int extent);
    int extent() const;

    virtual void run();

protected Q_SLOTS:
    virtual void requestFinished (int id, bool error);

private:
    int m_connectId;
};

#endif
