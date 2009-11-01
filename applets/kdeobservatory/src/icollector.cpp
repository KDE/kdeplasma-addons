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
