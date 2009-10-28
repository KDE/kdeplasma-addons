#include "icollector.h"

ICollector::ICollector(QObject *parent)
: QHttp(parent)
{
    connect (this, SIGNAL(requestFinished(int, bool)),
             this, SLOT  (requestFinished(int, bool)));
}

ICollector::~ICollector()
{
}

const QString &ICollector::summary() const
{
    return m_summary;
}

const ICollector::ResultMap &ICollector::resultMap() const
{
    return m_resultMap;
}
